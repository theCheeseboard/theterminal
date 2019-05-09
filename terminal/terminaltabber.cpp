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

#include <QStack>
#include "terminalwidget.h"

struct TerminalTabberPrivate {
    QList<TerminalWidget*> allTerminals;
    TerminalWidget* currentTerminal;

#ifdef Q_OS_MAC
    QTabBar* tabBar;
#else
    QMap<TerminalWidget*, QPushButton*> terminalButtons;
#endif
};

TerminalTabber::TerminalTabber(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TerminalTabber)
{
    ui->setupUi(this);
    d = new TerminalTabberPrivate();

#ifdef Q_OS_MAC
    ui->tabFrame->setVisible(false);
    d->tabBar = new QTabBar();
    d->tabBar->setDocumentMode(true);
    d->tabBar->setTabsClosable(true);
    this->layout()->addWidget(d->allTerminalstabBar);

    connect(d->tabBar, &QTabBar::currentChanged, [=](int index) {
        if (index != -1) {
            changeToTerminal(d->allTerminals.value(index));
        }
    });
    connect(d->tabBar, &QTabBar::tabCloseRequested, [=](int index) {
        d->allTerminals.value(index)->close();
    });
#endif

    ui->termStack->setCurrentAnimation(tStackedWidget::SlideHorizontal);
}

TerminalTabber::~TerminalTabber()
{
    delete d;
    delete ui;
}

void TerminalTabber::addTab(TerminalWidget *tab) {
    d->allTerminals.append(tab);

    //Install an event filter on all the tab's childrem
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

#ifndef Q_OS_MAC
    QPushButton* button = new QPushButton();
    button->setCheckable(true);
    button->setAutoExclusive(true);
    button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    d->terminalButtons.insert(tab, button);
#endif

    connect(tab, &TerminalWidget::finished, [=]() {
        int index = d->allTerminals.indexOf(tab);
#ifdef Q_OS_MAC
        d->tabBar->removeTab(index);
#else
        delete d->terminalButtons.value(tab);
        d->terminalButtons.remove(tab);
#endif
        d->allTerminals.removeOne(tab);
        ui->termStack->removeWidget(tab);
        delete tab;

        if (d->allTerminals.count() == 0) {
            emit done();
        } else {
            if (index == 0) {
                changeToTerminal(0);
            } else {
                changeToTerminal(index - 1);
            }
        }
    });
    connect(tab, &TerminalWidget::switchToThis, [=] {
        changeToTerminal(tab);
    });
    connect(tab, &TerminalWidget::openNewTerminal, this, QOverload<TerminalWidget*>::of(&TerminalTabber::addTab));
    /*connect(widget, &TerminalWidget::copyAvailable, [=](bool canCopy) {
        if (currentTerminal == widget) {
            ui->actionCopy->setEnabled(canCopy);
        }
    });*/

#ifdef Q_OS_MAC
    d->tabBar->addTab(tr("Terminal %1").arg(d->allTerminals.indexOf(tab) + 1));
#else
    button->setText(tr("Terminal %1").arg(d->allTerminals.indexOf(tab) + 1));
    ui->tabFrame->layout()->addWidget(button);
    ui->tabFrame->layout()->removeItem(ui->horizontalSpacer);
    ui->tabFrame->layout()->addItem(ui->horizontalSpacer);
    connect(button, &QPushButton::clicked, [=]() {
        changeToTerminal(tab);
    });
#endif

    ui->termStack->addWidget(tab);

    changeToTerminal(tab);
}

TerminalWidget* TerminalTabber::currentTerminal() {
    return d->currentTerminal;
}

void TerminalTabber::changeToTerminal(TerminalWidget *widget) {
    ui->termStack->setCurrentWidget(widget);
#ifdef Q_OS_MAC
    d->tabBar->setCurrentIndex(d->allTerminals.indexOf(widget));
#else
    d->terminalButtons.value(widget)->setChecked(true);
#endif

    widget->setFocus();
    d->currentTerminal = widget;

    //ui->actionCopy->setEnabled(widget->canCopy());
}

void TerminalTabber::changeToTerminal(int index) {
    changeToTerminal(d->allTerminals.at(index));
}

void TerminalTabber::setMenuButton(QWidget *menuButton) {
    if (menuButton->parentWidget() != nullptr) {
        menuButton->parentWidget()->layout()->removeWidget(menuButton);
    }

    static_cast<QBoxLayout*>(ui->topFrame->layout())->insertWidget(0, menuButton);
}

void TerminalTabber::closeAllTabs() {
    if (d->allTerminals.count() != 0) {
        for (TerminalWidget* widget : d->allTerminals) {
            widget->close();
        }
    }
}

void TerminalTabber::focusInEvent(QFocusEvent *event) {
    emit gotFocus();
}

bool TerminalTabber::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::FocusIn) {
        emit gotFocus();
    }
    return false;
}
