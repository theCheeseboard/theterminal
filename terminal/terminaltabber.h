/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2019 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/
#ifndef TERMINALTABBER_H
#define TERMINALTABBER_H

#include <QWidget>

namespace Ui {
    class TerminalTabber;
}

class TerminalWidget;
struct TerminalTabberPrivate;
class TerminalTabber : public QWidget {
        Q_OBJECT

    public:
        explicit TerminalTabber(QWidget* parent = nullptr);
        ~TerminalTabber();

        TerminalWidget* currentTerminal();

    public slots:
        void setMenuButton(QWidget* menuButton);
        void setCsdButtons(QWidget* csdButtons);
        void addTab(TerminalWidget* tab);

        void closeAllTabs();

    signals:
        void done();
        void gotFocus();

    private:
        Ui::TerminalTabber* ui;
        TerminalTabberPrivate* d;

        void focusInEvent(QFocusEvent* event) override;
        bool eventFilter(QObject* watched, QEvent* event) override;
};

#endif // TERMINALTABBER_H
