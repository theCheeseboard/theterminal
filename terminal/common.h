/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2022 Victor Tran
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
#ifndef COMMON_H
#define COMMON_H

#include <QKeySequence>

class QWidget;
namespace Common {
    enum Action {
        NewWindow,
        NewTab,
        Copy,
        Paste,
        Find,
        Print,
        CloseTab,
        Exit
    };

    QKeySequence shortcutForPlatform(Action action);
    QKeyCombination translateMacKey(QKeyCombination keys);
} // namespace Common

#endif // COMMON_H
