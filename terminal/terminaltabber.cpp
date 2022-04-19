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
#include "terminaltabber.h"
#include "ui_terminaltabber.h"

#include "terminalwidget.h"
#include <QStack>
#include <tcsdtools.h>

#ifdef T_OS_UNIX_NOT_MAC
    #include <X11/Xlib.h>

    #undef FocusIn
#endif

struct TerminalTabberPrivate {
        tCsdTools csd;
        QList<TerminalWidget*> allTerminals;
};

TerminalTabber::TerminalTabber(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::TerminalTabber) {
    ui->setupUi(this);
    d = new TerminalTabberPrivate();

    d->csd.installMoveAction(ui->topFrame);
    if (tCsdGlobal::windowControlsEdge() == tCsdGlobal::Left) {
        static_cast<QBoxLayout*>(ui->csdWidget->layout())->setDirection(QBoxLayout::RightToLeft);
    }

    ui->termStack->setCurrentAnimation(tStackedWidget::SlideHorizontal);
    qApp->installEventFilter(this);

    connect(ui->termStack, &tStackedWidget::switchingFrame, this, [=](int switchingTo) {
        QWidget* widget = ui->termStack->widget(switchingTo);
        widget->setFocus();
        this->setFocusProxy(widget);
    });

    ui->tabber->setShowNewTabButton(true);
    connect(ui->tabber, &tWindowTabber::newTabRequested, this, [=] {
        this->addTab(new TerminalWidget());
    });
}

TerminalTabber::~TerminalTabber() {
    delete d;
    delete ui;
}

void TerminalTabber::addTab(TerminalWidget* tab) {
    d->allTerminals.append(tab);

    // Install an event filter on all the tab's childrem
    QStack<QWidget*> filterInstalls;
    filterInstalls.push(tab);
    while (!filterInstalls.isEmpty()) {
        QWidget* w = filterInstalls.pop();
        for (QObject* child : w->children()) {
            QWidget* childWidget = qobject_cast<QWidget*>(child);
            if (childWidget != nullptr) filterInstalls.push(childWidget);
        }

        w->installEventFilter(this);
    }

    tWindowTabberButton* button = new tWindowTabberButton();
    connect(tab, &TerminalWidget::finished, [=] {
        ui->tabber->removeButton(button);

        int index = d->allTerminals.indexOf(tab);
        d->allTerminals.removeOne(tab);
        ui->termStack->removeWidget(tab);
        tab->deleteLater();

        if (d->allTerminals.count() == 0) {
            emit done();
        } else {
            if (index == 0) {
                ui->termStack->setCurrentIndex(0);
            } else {
                ui->termStack->setCurrentIndex(index - 1);
            }
        }
    });
    connect(tab, &TerminalWidget::switchToThis, [=] {
        ui->termStack->setCurrentWidget(tab);
    });
    connect(tab, &TerminalWidget::openNewTerminal, this, QOverload<TerminalWidget*>::of(&TerminalTabber::addTab));

    ui->termStack->addWidget(tab);

    button->setText(tr("Terminal %1").arg(d->allTerminals.indexOf(tab) + 1));
    button->syncWithStackedWidget(ui->termStack, tab);
    ui->tabber->addButton(button);

    ui->termStack->setCurrentWidget(tab);
}

TerminalWidget* TerminalTabber::currentTerminal() {
    return qobject_cast<TerminalWidget*>(ui->termStack->currentWidget());
}

void TerminalTabber::setMenuButton(QWidget* menuButton) {
    if (menuButton->parentWidget() != nullptr) {
        menuButton->parentWidget()->layout()->removeWidget(menuButton);
    }

    static_cast<QBoxLayout*>(ui->topFrame->layout())->insertWidget(0, menuButton);
}

void TerminalTabber::setCsdButtons(QWidget* csdButtons) {
    if (csdButtons->parentWidget() != nullptr) {
        csdButtons->parentWidget()->layout()->removeWidget(csdButtons);
    }

    static_cast<QBoxLayout*>(ui->csdWidget->layout())->addWidget(csdButtons);
}

void TerminalTabber::closeAllTabs() {
    for (TerminalWidget* widget : d->allTerminals) {
        QTimer::singleShot(0, [=] {
            widget->close();
        });
    }
}

void TerminalTabber::focusInEvent(QFocusEvent* event) {
    // Pass on focus to the current terminal
    currentTerminal()->setFocus();
}

bool TerminalTabber::eventFilter(QObject* watched, QEvent* event) {
    if (event->type() == QEvent::FocusIn) {
        emit gotFocus();
    }

    if (event->type() == QEvent::MouseMove && watched == qApp) {
    }

    return false;
}
