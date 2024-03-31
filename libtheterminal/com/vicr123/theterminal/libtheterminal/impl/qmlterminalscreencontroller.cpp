#include "qmlterminalscreencontroller.h"

#include <QCache>
#include <QCoreApplication>
#include <QTimer>
#include <ipty.h>
#include <terminalscreen.h>
#include <tlogger.h>

struct QmlTerminalScreenControllerPrivate {
        IPty* pty = nullptr;
        TerminalScreen* terminalScreen = nullptr;
        QMap<int, QVariantList> cachedRuns;
        // QTimer* contentChangeDebounce;
};

QmlTerminalScreenController::QmlTerminalScreenController(QObject* parent) :
    QObject{parent}, d{new QmlTerminalScreenControllerPrivate()} {
    // d->contentChangeDebounce = new QTimer(this);
    // d->contentChangeDebounce->setInterval(50);

    d->terminalScreen = new TerminalScreen(this);
    connect(d->terminalScreen, &TerminalScreen::colsChanged, this, &QmlTerminalScreenController::colsChanged);
    connect(d->terminalScreen, &TerminalScreen::rowsChanged, this, &QmlTerminalScreenController::rowsChanged);
    connect(d->terminalScreen, &TerminalScreen::caretColChanged, this, &QmlTerminalScreenController::caretColChanged);
    connect(d->terminalScreen, &TerminalScreen::caretRowChanged, this, &QmlTerminalScreenController::caretRowChanged);
    connect(d->terminalScreen, &TerminalScreen::rowContentChanged, this, [this](int row) {
        d->cachedRuns.remove(row);
        emit rowContentChanged(row);
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

#include <QTimer>
void QmlTerminalScreenController::start(QString process) {
    d->pty = IPty::createPty(this);
    d->pty->start("fish", QProcessEnvironment::systemEnvironment(), QCoreApplication::applicationDirPath(), 80, 24);
    connect(d->pty->device(), &QIODevice::readyRead, this, [this] {
        for (auto character : d->pty->device()->readAll()) {
            if (character == '\n') {
                d->terminalScreen->setCaretCol(0);
                d->terminalScreen->setCaretRow(d->terminalScreen->caretRow() + 1);
            } else {
                d->terminalScreen->setCharacter(d->terminalScreen->caretCol(), d->terminalScreen->caretRow(), {character});
                d->terminalScreen->setCaretCol(d->terminalScreen->caretCol() + 1);
            }
        }
    });

    QTimer::singleShot(1000, this, [this] {
        d->pty->device()->write("diskutil list\n");
    });
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
    QVariantMap currentMap;
    QString currentText;

    currentMap.insert("color", QColor(Qt::white));
    currentMap.insert("backgroundColor", QColor(Qt::black));

    TerminalScreen::CharacterSpace previous;
    for (auto i = 0; i < d->terminalScreen->cols(); i++) {
        auto character = d->terminalScreen->character(i, row);
        // TODO: Compare attributes with previous

        currentText.append(character.character);

        previous = character;
    }

    currentMap.insert("text", currentText);
    runs.append(currentMap);

    d->cachedRuns.insert(row, runs);
    return runs;
}
