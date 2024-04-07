#include "qmlterminalscreencontroller.h"

#include <Emulation/vt100emulation.h>
#include <QCache>
#include <QClipboard>
#include <QCoreApplication>
#include <QTimer>
#include <ipty.h>
#include <terminalscreen.h>
#include <tlogger.h>

struct QmlTerminalScreenControllerPrivate {
        IPty* pty = nullptr;
        TerminalScreen* terminalScreen = nullptr;

        VT100Emulation* emulation = nullptr;

        QTimer* rowUpdateTimer;
        QSet<int> rowsToUpdate;

        QPoint selectionStart;
        QPoint selectionEnd;

        ScreenColorManager screenColorManager;
};

QmlTerminalScreenController::QmlTerminalScreenController(QObject* parent) :
    QObject{parent}, d{new QmlTerminalScreenControllerPrivate()} {
    d->terminalScreen = new TerminalScreen(this);
    connect(d->terminalScreen, &TerminalScreen::colsChanged, this, &QmlTerminalScreenController::colsChanged);
    connect(d->terminalScreen, &TerminalScreen::rowsChanged, this, &QmlTerminalScreenController::rowsChanged);
    connect(d->terminalScreen, &TerminalScreen::colsChanged, this, [this] {
        if (!d->pty) return;
        d->pty->setWindowSize(d->terminalScreen->cols(), d->terminalScreen->rows());
    });
    connect(d->terminalScreen, &TerminalScreen::rowsChanged, this, [this] {
        if (!d->pty) return;
        d->pty->setWindowSize(d->terminalScreen->cols(), d->terminalScreen->rows());
    });
    connect(d->terminalScreen, &TerminalScreen::caretColChanged, this, &QmlTerminalScreenController::caretColChanged);
    connect(d->terminalScreen, &TerminalScreen::caretRowChanged, this, &QmlTerminalScreenController::caretRowChanged);
    connect(d->terminalScreen, &TerminalScreen::rowContentChanged, this, [this](int row) {
        // d->cachedRuns.remove(row);
        queueRowUpdate(row);
    });
    connect(d->terminalScreen, &TerminalScreen::historyRolled, this, [this] {
        for (auto i = 0; i < d->terminalScreen->rows(); i++) {
            // if (d->cachedRuns.contains(i + 1)) {
            // d->cachedRuns.insert(i, d->cachedRuns.value(i + 1));
            // }
            queueRowUpdate(i);
        }
        // d->cachedRuns.remove(d->terminalScreen->rows() - 1);
        queueRowUpdate(d->terminalScreen->rows() - 1);
    });
    connect(d->terminalScreen, &TerminalScreen::invertScreenChanged, this, [this] {
        // d->cachedRuns.clear();
        for (auto i = 0; i < d->terminalScreen->rows(); i++) {
            queueRowUpdate(i);
        }
        emit invertScreenChanged();
    });
    connect(d->terminalScreen, &TerminalScreen::scrollbackLinesChanged, this, &QmlTerminalScreenController::scrollbackLinesChanged);
    connect(d->terminalScreen, &TerminalScreen::caretVisibleChanged, this, &QmlTerminalScreenController::caretVisibleChanged);

    d->rowUpdateTimer = new QTimer(this);
    d->rowUpdateTimer->setInterval(0);
    d->rowUpdateTimer->setSingleShot(true);
    connect(d->rowUpdateTimer, &QTimer::timeout, this, [this] {
        for (auto row : std::as_const(d->rowsToUpdate)) {
            emit rowContentChanged(row);
        }
        d->rowsToUpdate.clear();
    });
}

QmlTerminalScreenController::~QmlTerminalScreenController() {
    delete d;
}

int QmlTerminalScreenController::cols() {
    return d->terminalScreen->cols();
}

void QmlTerminalScreenController::setCols(int cols) {
    d->terminalScreen->setCols(cols);
}

int QmlTerminalScreenController::rows() {
    return d->terminalScreen->rows();
}

void QmlTerminalScreenController::setRows(int rows) {
    d->terminalScreen->setRows(rows);
}

int QmlTerminalScreenController::caretCol() {
    return d->terminalScreen->caretCol();
}

int QmlTerminalScreenController::caretRow() {
    return d->terminalScreen->caretRow();
}

QPoint QmlTerminalScreenController::selectionStart() {
    return d->selectionStart;
}

void QmlTerminalScreenController::setSelectionStart(QPoint selectionStart) {
    d->selectionStart = selectionStart;
    emit selectionStartChanged();
    emit normalisedSelectionChanged();
    emit haveSelectionChanged();
}

QPoint QmlTerminalScreenController::selectionEnd() {
    return d->selectionEnd;
}

void QmlTerminalScreenController::setSelectionEnd(QPoint selectionEnd) {
    d->selectionEnd = selectionEnd;
    emit selectionEndChanged();
    emit normalisedSelectionChanged();
    emit haveSelectionChanged();
}

bool QmlTerminalScreenController::invertScreen() {
    return d->terminalScreen->invertScreen();
}

bool QmlTerminalScreenController::caretVisible() {
    return d->terminalScreen->caretVisible();
}

bool QmlTerminalScreenController::reportMouseEvents() {
    return false;
}

quint64 QmlTerminalScreenController::scrollbackLines() {
    return d->terminalScreen->scrollbackLines();
}

void QmlTerminalScreenController::start(QString process) {
    d->pty = IPty::createPty(this);
    d->pty->start(process, QProcessEnvironment::systemEnvironment(), QCoreApplication::applicationDirPath(), d->terminalScreen->cols(), d->terminalScreen->rows());

    d->emulation = new VT100Emulation(d->pty->device(), d->terminalScreen, this);
}

void QmlTerminalScreenController::pressKey(Qt::KeyboardModifiers modifiers, Qt::Key key, QString keyChar) {
    d->emulation->pressKey(modifiers, key, keyChar);
}

QVariantList QmlTerminalScreenController::runs(int row, int start) {
    if (row < 0) {
        tWarn("QmlTerminalScreenController") << "Attempted to calculate runs for row " << row << " which is less than 0";
        return {};
    }

    if (row >= d->terminalScreen->rows()) {
        tWarn("QmlTerminalScreenController") << "Attempted to calculate runs for row " << row << " which is more than the number of rows, " << d->terminalScreen->rows();
        return {};
    }

    return this->calculateRuns(start, d->terminalScreen->cols(), [&row, this](int i) {
        return d->terminalScreen->character(i, row);
    });
}

QVariantList QmlTerminalScreenController::scrollbackRuns(quint64 line, int start) {
    if (line >= d->terminalScreen->scrollbackLines()) {
        tWarn("QmlTerminalScreenController") << "Attempted to calculate runs for scrollback line " << line << " which is more than the number of scrollback lines, " << d->terminalScreen->scrollbackLines();
        return {};
    }

    auto scrollbackLine = d->terminalScreen->scrollbackLine(line);
    return this->calculateRuns(start, scrollbackLine->length(), [this, &scrollbackLine](int i) {
        return scrollbackLine->at(i);
    });
}

TerminalScreen::RowScaleMode QmlTerminalScreenController::rowScaleMode(int row) {
    if (row < 0) {
        return {};
    }

    if (row >= d->terminalScreen->rows()) {
        return {};
    }

    return d->terminalScreen->rowScaleMode(row);
}

void QmlTerminalScreenController::copy() {
    if (this->selectionStart() == this->selectionEnd()) return;
    qApp->clipboard()->setText(this->selectedText());
}

void QmlTerminalScreenController::paste() {
    auto clipboardContents = qApp->clipboard()->text();
    // TODO: Check if clipboard contents are dangerous
    d->emulation->paste(clipboardContents);
}

QVariantList QmlTerminalScreenController::calculateRuns(int start, int cols, std::function<TerminalScreen::CharacterSpace(int)> getCharacter) {
    QVariantList runs;
    QVariantMap currentMap = initFormat({});
    QByteArray currentText;
    TerminalScreen::CharacterSpace::CharacterFormat previousFormat;
    for (auto i = start; i < cols; i++) {
        auto character = getCharacter(i);

        if (previousFormat != character.format) {
            if (!currentText.isEmpty()) {
                currentMap.insert("text", QString(currentText));
                runs.append(currentMap);
                currentText.clear();
            }

            currentMap = initFormat(character.format);
        }

        currentText.append(character.character.unicode());

        previousFormat = character.format;
    }

    currentMap.insert("text", QString(currentText));
    runs.append(currentMap);
    return runs;
}

QVariantMap QmlTerminalScreenController::initFormat(TerminalScreen::CharacterSpace::CharacterFormat format) {
    auto color = d->screenColorManager.decodeColor(format.color);
    auto backgroundColor = d->screenColorManager.decodeColor(format.backgroundColor);

    QVariantMap map;
    if (format.invert ^ d->terminalScreen->invertScreen()) {
        map.insert("color", backgroundColor);
        map.insert("backgroundColor", color);
    } else {
        map.insert("color", color);
        map.insert("backgroundColor", backgroundColor);
    }
    map.insert("underline", format.underline);
    map.insert("blink", format.blink);
    map.insert("bold", format.bold);
    return map;
}

void QmlTerminalScreenController::queueRowUpdate(int row) {
    d->rowsToUpdate.insert(row);
    if (!d->rowUpdateTimer->isActive()) {
        d->rowUpdateTimer->start();
    }
}

QString QmlTerminalScreenController::selectedText() {
    if (this->normalisedSelectionStart() == this->normalisedSelectionEnd()) return {};

    QStringList text;
    for (auto i = this->normalisedSelectionStart().y(); i <= this->normalisedSelectionEnd().y(); i++) {
        auto runStart = i == this->normalisedSelectionStart().y() ? this->normalisedSelectionStart().x() : 0;
        auto runs = i < this->scrollbackLines() ? this->scrollbackRuns(i, runStart) : this->runs(i - this->scrollbackLines(), runStart);

        QString line;
        for (const auto& run : runs) {
            line.append(run.toMap().value("text").toString());
        }
        if (i == this->normalisedSelectionEnd().y()) {
            auto length = this->normalisedSelectionEnd().x() - (i == normalisedSelectionStart().y() ? normalisedSelectionStart().x() : 0) + 1;
            line = line.left(length);
        }

        // Trim the end of the line
        for (auto n = line.size() - 1; n >= 0; --n) {
            if (!line.at(n).isSpace()) {
                line = line.left(n + 1);
                break;
            }
        }
        text.append(line);
    }
    return text.join("\n");
}

QPoint QmlTerminalScreenController::normalisedSelectionStart() const {
    auto yDiff = d->selectionStart.y() <=> d->selectionEnd.y();
    auto xDiff = d->selectionStart.x() <=> d->selectionEnd.x();

    // Compare Y first
    // If Y is equal, comapre X
    // Pick the earliest
    if (yDiff < 0) {
        return d->selectionStart;
    } else if (yDiff > 0) {
        return d->selectionEnd;
    } else if (xDiff < 0) {
        return d->selectionStart;
    } else if (xDiff > 0) {
        return d->selectionEnd;
    } else {
        return d->selectionStart;
    }
}

QPoint QmlTerminalScreenController::normalisedSelectionEnd() const {
    auto yDiff = d->selectionEnd.y() <=> d->selectionStart.y();
    auto xDiff = d->selectionEnd.x() <=> d->selectionStart.x();

    // Compare Y first
    // If Y is equal, comapre X
    // Pick the earliest
    if (yDiff < 0) {
        return d->selectionStart;
    } else if (yDiff > 0) {
        return d->selectionEnd;
    } else if (xDiff < 0) {
        return d->selectionStart;
    } else if (xDiff > 0) {
        return d->selectionEnd;
    } else {
        return d->selectionStart;
    }
}

bool QmlTerminalScreenController::haveSelection() const {
    return d->selectionStart != d->selectionEnd;
}
