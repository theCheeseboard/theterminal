#include "terminalscreen.h"

#include <tlogger.h>

using CharacterLine = QSharedPointer<QList<TerminalScreen::CharacterSpace>>;

struct TerminalScreenPrivate {
        int cols = 80;
        int rows = 24;

        int caretCol = 0;
        int caretRow = 0;

        QList<CharacterLine> characters;
};

TerminalScreen::TerminalScreen(QObject* parent) :
    QObject{parent}, d{new TerminalScreenPrivate()} {
    setCols(80);
    setRows(24);

    setCharacter(3, 0, {'x'});
}

TerminalScreen::~TerminalScreen() {
    delete d;
}

int TerminalScreen::cols() {
    return d->cols;
}

void TerminalScreen::setCols(int cols) {
    if (cols < 0) cols = 0;

    d->cols = cols;
    for (const auto& row : d->characters) {
        // Don't delete anything off the end
        if (row->length() < cols) {
            row->resize(cols);
        }
    }
    emit colsChanged();
    tDebug("TerminalScreen") << "Cols: " << cols;
}

int TerminalScreen::rows() {
    return d->rows;
}

void TerminalScreen::setRows(int rows) {
    if (rows < 1) rows = 1;

    if (rows < d->rows) {
        // TODO: Move everything
    }
    d->characters.resize(rows);
    for (auto i = 0; i < rows; i++) {
        if (d->characters.value(i) == nullptr) {
            d->characters.replace(i, CharacterLine(new QList<TerminalScreen::CharacterSpace>(d->cols)));
        }
    }

    d->rows = rows;
    emit rowsChanged();
    for (auto i = 0; i < rows; i++) {
        emit rowContentChanged(i);
    }
    tDebug("TerminalScreen") << "Rows: " << rows;
}

int TerminalScreen::caretCol() {
    return d->caretCol;
}

void TerminalScreen::setCaretCol(int col) {
    if (col <= 0) col = 0;
    if (col >= d->cols) col = d->cols - 1;

    d->caretCol = col;
    emit caretColChanged();
}

int TerminalScreen::caretRow() {
    return d->caretRow;
}

void TerminalScreen::setCaretRow(int row) {
    if (row <= 0) row = 0;
    if (row >= d->rows) row = d->rows - 1;

    d->caretRow = row;
    emit caretRowChanged();
}

void TerminalScreen::setCharacter(int col, int row, CharacterSpace character) {
    if (d->cols <= col) return;
    if (d->rows <= row) return;

    d->characters.at(row)->replace(col, character);
    emit rowContentChanged(row);
}

TerminalScreen::CharacterSpace TerminalScreen::character(int col, int row) {
    return d->characters.at(row)->at(col);
}

void TerminalScreen::pushToHistory() {
    // TODO: Push the top row to history
    d->characters.removeFirst();
    d->characters.append(CharacterLine(new QList<TerminalScreen::CharacterSpace>(d->cols)));
    emit historyRolled();
}
