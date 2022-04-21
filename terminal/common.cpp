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
#include "common.h"

QKeySequence Common::shortcutForPlatform(Action action) {
    switch (action) {
        case Common::NewWindow:
            return QKeySequence(translateMacKey(Qt::ControlModifier | Qt::Key_N));
        case Common::Find:
            return QKeySequence(translateMacKey(Qt::ControlModifier | Qt::Key_F));
        case Common::Print:
            return QKeySequence(translateMacKey(Qt::ControlModifier | Qt::Key_P));
        case Common::NewTab:
            return QKeySequence(translateMacKey(Qt::ControlModifier | Qt::Key_T));
        case Common::Copy:
            return QKeySequence(translateMacKey(Qt::ControlModifier | Qt::Key_C));
        case Common::Paste:
            return QKeySequence(translateMacKey(Qt::ControlModifier | Qt::Key_V));
        case Common::CloseTab:
            return QKeySequence(translateMacKey(Qt::ControlModifier | Qt::Key_W));
        case Common::Exit:
            return QKeySequence(translateMacKey(Qt::ControlModifier | Qt::Key_Q));
    }
}

QKeyCombination Common::translateMacKey(QKeyCombination keys) {
#ifdef Q_OS_MAC
    return keys;
#else
    return QKeyCombination::fromCombined(keys.toCombined() | Qt::ShiftModifier);
#endif
}
