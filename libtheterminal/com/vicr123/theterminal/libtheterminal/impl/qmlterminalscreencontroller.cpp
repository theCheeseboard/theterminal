#include "qmlterminalscreencontroller.h"

#include <Emulation/vt100emulation.h>
#include <QCache>
#include <QCoreApplication>
#include <QTimer>
#include <ipty.h>
#include <terminalscreen.h>
#include <tlogger.h>

struct QmlTerminalScreenControllerPrivate {
        IPty* pty = nullptr;
        TerminalScreen* terminalScreen = nullptr;
        QHash<int, QVariantList> cachedRuns;

        VT100Emulation* emulation = nullptr;

        QTimer* rowUpdateTimer;
        QSet<int> rowsToUpdate;

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
        d->cachedRuns.remove(row);
        queueRowUpdate(row);
    });
    connect(d->terminalScreen, &TerminalScreen::historyRolled, this, [this] {
        for (auto i = 0; i < d->terminalScreen->rows(); i++) {
            if (d->cachedRuns.contains(i + 1)) {
                d->cachedRuns.insert(i, d->cachedRuns.value(i + 1));
            }
            queueRowUpdate(i);
        }
        d->cachedRuns.remove(d->terminalScreen->rows() - 1);
        queueRowUpdate(d->terminalScreen->rows() - 1);
    });

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

quint64 QmlTerminalScreenController::scrollbackLines() {
    return 0;
}

void QmlTerminalScreenController::start(QString process) {
    d->pty = IPty::createPty(this);
    d->pty->start("sh", QProcessEnvironment::systemEnvironment(), QCoreApplication::applicationDirPath(), d->terminalScreen->cols(), d->terminalScreen->rows());

    d->emulation = new VT100Emulation(d->pty->device(), d->terminalScreen, this);
}

void QmlTerminalScreenController::pressKey(Qt::KeyboardModifiers modifiers, Qt::Key key, QString keyChar) {
    d->emulation->pressKey(modifiers, key, keyChar);
}

QVariantList QmlTerminalScreenController::runs(int row) {
    if (row < 0) {
        tWarn("QmlTerminalScreenController") << "Attempted to calculate runs for row " << row << " which is less than 0";
        return {};
    }

    if (row >= d->terminalScreen->rows()) {
        tWarn("QmlTerminalScreenController") << "Attempted to calculate runs for row " << row << " which is more than the number of rows, " << d->terminalScreen->rows();
        return {};
    }

    if (d->cachedRuns.contains(row)) return d->cachedRuns.value(row);

    QVariantList runs;
    QVariantMap currentMap = initFormat({});
    QString currentText;
    TerminalScreen::CharacterSpace::CharacterFormat previousFormat;
    for (auto i = 0; i < d->terminalScreen->cols(); i++) {
        auto character = d->terminalScreen->character(i, row);

        if (previousFormat != character.format) {
            if (!currentText.isEmpty()) {
                currentMap.insert("text", currentText);
                runs.append(currentMap);
                currentText.clear();
            }

            currentMap = initFormat(character.format);
        }

        currentText.append(character.character);

        previousFormat = character.format;
    }

    currentMap.insert("text", currentText);
    runs.append(currentMap);

    d->cachedRuns.insert(row, runs);
    return runs;
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

QVariantMap QmlTerminalScreenController::initFormat(TerminalScreen::CharacterSpace::CharacterFormat format) {
    auto color = d->screenColorManager.decodeColor(format.color);
    auto backgroundColor = d->screenColorManager.decodeColor(format.backgroundColor);

    QVariantMap map;
    if (format.invert) {
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
    d->rowUpdateTimer->start();
}
