#include "terminalscreen.h"

#include <tlogger.h>

using CharacterLine = QSharedPointer<QList<TerminalScreen::CharacterSpace>>;

struct TerminalScreenPrivate {
        int cols = 80;
        int rows = 24;

        int caretCol = 0;
        int caretRow = 0;

        bool invertScreen = false;
        bool caretVisible = true;

        TerminalScreen::CharacterSpace::CharacterFormat currentFormat;
        QList<CharacterLine> scrollback;

        struct Screen {
                QList<CharacterLine> characters;
                QList<TerminalScreen::RowScaleMode> rowScaleModes;

                int marginBottom = -1;
                int marginTop = -1;
                bool marginsBound = false;
        };

        Screen* screen = &screens[0];
        Screen screens[2]{
            {}, {}};
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
    for (auto& screen : d->screens) {
        for (const auto& row : screen.characters) {
            // Don't delete anything off the end
            if (row->length() < cols) {
                row->resize(cols);
            }
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

    for (auto& screen : d->screens) {
        screen.characters.resize(rows);
        screen.rowScaleModes.resize(rows);
        for (auto i = 0; i < rows; i++) {
            if (screen.characters.value(i) == nullptr) {
                screen.characters.replace(i, CharacterLine(new QList<TerminalScreen::CharacterSpace>(d->cols)));
            }
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
    return d->caretRow + this->firstRow();
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

bool TerminalScreen::caretVisible() {
    return d->caretVisible;
}

void TerminalScreen::setCaretVisible(bool visible) {
    d->caretVisible = visible;
    emit caretVisibleChanged();
}

TerminalScreen::ScreenBuffer TerminalScreen::screenBuffer() {
    return static_cast<ScreenBuffer>(d->screen - &d->screens[0]);
}

void TerminalScreen::setScreenBuffer(ScreenBuffer screenBuffer) {
    d->screen = &d->screens[static_cast<int>(screenBuffer)];
    emit screenBufferChanged();
    emit caretRowChanged();
    emit marginsBoundChanged();
    emit scrollbackLinesChanged();
    for (auto i = 0; i < d->rows; i++) {
        emit rowContentChanged(i);
    }
}

void TerminalScreen::setVerticalMargins(int top, int bottom) {
    d->screen->marginTop = top;
    d->screen->marginBottom = bottom;
    emit caretRowChanged();
}

void TerminalScreen::setMarginsBound(bool marginsBound) {
    d->screen->marginsBound = marginsBound;
    emit marginsBoundChanged();
    emit caretRowChanged();
}

bool TerminalScreen::marginsBound() {
    return d->screen->marginsBound;
}

int TerminalScreen::firstRow() {
    if (!d->screen->marginsBound) return 0;
    return d->screen->marginTop == -1 ? 0 : d->screen->marginTop;
}

void TerminalScreen::setCharacter(int col, int row, CharacterSpace character) {
    if (d->cols <= col) return;
    if (d->rows <= row) return;

    d->screen->characters.at(row)->replace(col, character);
    emit rowContentChanged(row);
}

void TerminalScreen::setCharacter(int col, int row, QChar character) {
    setCharacter(col, row, {character, d->currentFormat});
}

TerminalScreen::CharacterSpace TerminalScreen::character(int col, int row) {
    return d->screen->characters.at(row)->at(col);
}

void TerminalScreen::pushToHistory() {
    int topRow = d->screen->marginTop;
    if (topRow == -1) topRow = 0;

    int bottomRow = d->screen->marginBottom;
    if (bottomRow == -1) bottomRow = d->rows - 1;

    // TODO: truncate scrollback
    if (this->screenBuffer() == ScreenBuffer::StandardScreen) {
        d->scrollback.append(d->screen->characters.takeAt(topRow));
    } else {
        d->screen->characters.removeAt(topRow);
    }

    d->screen->characters.insert(bottomRow, CharacterLine(new QList<TerminalScreen::CharacterSpace>(d->cols)));
    d->screen->rowScaleModes.removeAt(topRow);
    d->screen->rowScaleModes.insert(bottomRow, RowScaleMode::Normal);
    emit historyRolled();
    emit scrollbackLinesChanged();
}

void TerminalScreen::clearScrollback() {
    d->scrollback.clear();
    emit scrollbackLinesChanged();
}

void TerminalScreen::setRowScaleMode(int row, RowScaleMode mode) {
    d->screen->rowScaleModes.insert(row, mode);
    emit rowContentChanged(row);
}

TerminalScreen::RowScaleMode TerminalScreen::rowScaleMode(int row) {
    return d->screen->rowScaleModes.at(row);
}

quint64 TerminalScreen::scrollbackLines() {
    return this->screenBuffer() == ScreenBuffer::StandardScreen ? d->scrollback.count() : 0;
}

QSharedPointer<QList<TerminalScreen::CharacterSpace>> TerminalScreen::scrollbackLine(quint64 line) {
    return d->scrollback.at(line);
}
