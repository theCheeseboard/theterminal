#include "terminalscreen.h"

#include <tlogger.h>

using CharacterLine = QSharedPointer<QList<TerminalScreen::CharacterSpace>>;

struct TerminalScreenPrivate {
        int cols = 80;
        int rows = 24;

        int caretCol = 0;
        int caretRow = 0;

        bool invertScreen = false;

        TerminalScreen::CharacterSpace::CharacterFormat currentFormat;

        QList<CharacterLine> characters;
        QList<TerminalScreen::RowScaleMode> rowScaleModes;
};

TerminalScreen::TerminalScreen(QObject* parent) :
    QObject{parent}, d{new TerminalScreenPrivate()} {
    setCols(80);
    setRows(24);
}

TerminalScreen::~TerminalScreen() {
    delete d;
}

QChar TerminalScreen::emptyChar() {
    return ' ';
}

int TerminalScreen::cols() {
    return d->cols;
}

void TerminalScreen::setCols(int cols) {
    if (cols < 1) return;

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
    if (rows < 1) return;

    if (rows < d->rows) {
        // TODO: Move everything
        for (auto i = rows; i < d->rows; i++) {
            pushToHistory();
        }
    }
    d->characters.resize(rows);
    d->rowScaleModes.resize(rows);
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
    if (col > d->cols) col = d->cols - 1;

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

void TerminalScreen::setCurrentCharacterFormat(CharacterSpace::CharacterFormat format) {
    d->currentFormat = format;
}

TerminalScreen::CharacterSpace::CharacterFormat TerminalScreen::currentCharacterFormat() {
    return d->currentFormat;
}

bool TerminalScreen::invertScreen() {
    return d->invertScreen;
}

void TerminalScreen::setInvertScreen(bool invertScreen) {
    d->invertScreen = invertScreen;
    emit invertScreenChanged();
}

void TerminalScreen::setCharacter(int col, int row, CharacterSpace character) {
    if (d->cols <= col) return;
    if (d->rows <= row) return;

    d->characters.at(row)->replace(col, character);
    emit rowContentChanged(row);
}

void TerminalScreen::setCharacter(int col, int row, QChar character) {
    setCharacter(col, row, {character, d->currentFormat});
}

TerminalScreen::CharacterSpace TerminalScreen::character(int col, int row) {
    return d->characters.at(row)->at(col);
}

void TerminalScreen::pushToHistory() {
    // TODO: Push the top row to history
    d->characters.removeFirst();
    d->characters.append(CharacterLine(new QList<TerminalScreen::CharacterSpace>(d->cols)));
    d->rowScaleModes.removeFirst();
    d->rowScaleModes.append(RowScaleMode::Normal);
    emit historyRolled();
}

void TerminalScreen::setRowScaleMode(int row, RowScaleMode mode) {
    d->rowScaleModes.insert(row, mode);
    emit rowContentChanged(row);
}

TerminalScreen::RowScaleMode TerminalScreen::rowScaleMode(int row) {
    return d->rowScaleModes.at(row);
}
