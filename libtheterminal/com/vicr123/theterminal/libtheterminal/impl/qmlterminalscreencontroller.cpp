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
        QMap<int, QVariantList> cachedRuns;

        VT100Emulation* emulation = nullptr;
};

QmlTerminalScreenController::QmlTerminalScreenController(QObject* parent) :
    QObject{parent}, d{new QmlTerminalScreenControllerPrivate()} {
    d->terminalScreen = new TerminalScreen(this);
    connect(d->terminalScreen, &TerminalScreen::colsChanged, this, &QmlTerminalScreenController::colsChanged);
    connect(d->terminalScreen, &TerminalScreen::rowsChanged, this, &QmlTerminalScreenController::rowsChanged);
    connect(d->terminalScreen, &TerminalScreen::caretColChanged, this, &QmlTerminalScreenController::caretColChanged);
    connect(d->terminalScreen, &TerminalScreen::caretRowChanged, this, &QmlTerminalScreenController::caretRowChanged);
    connect(d->terminalScreen, &TerminalScreen::rowContentChanged, this, [this](int row) {
        d->cachedRuns.remove(row);
        emit rowContentChanged(row);
    });
    connect(d->terminalScreen, &TerminalScreen::historyRolled, this, [this] {
        for (auto i = 0; i < d->terminalScreen->rows(); i++) {
            if (d->cachedRuns.contains(i + 1)) {
                d->cachedRuns.insert(i, d->cachedRuns.value(i + 1));
            }
            emit rowContentChanged(i);
        }
        d->cachedRuns.remove(d->terminalScreen->rows() - 1);
        emit rowContentChanged(d->terminalScreen->rows() - 1);
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
    d->pty->start("sh", QProcessEnvironment::systemEnvironment(), QCoreApplication::applicationDirPath(), 80, 24);

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
