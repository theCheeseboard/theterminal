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
#include "printcontroller.h"

#include "terminalwidget.h"
#include <QPainter>
#include <tpopover.h>
#include <tprintpopover.h>

struct PrintControllerPrivate {
        TerminalWidget* parentWidget;
        QPrinter* printer;
};

PrintController::PrintController(TerminalWidget* parent) :
    QObject{parent} {
    d = new PrintControllerPrivate();
    d->parentWidget = parent;

    d->printer = new QPrinter(QPrinter::HighResolution);
    d->printer->setPageMargins(QMarginsF(1, 1, 1, 1), QPageLayout::Inch);
    d->printer->setDocName(parent->title());
}

PrintController::~PrintController() {
    delete d;
}

void PrintController::confirmAndPerformPrint() {
    tPrintPopover* jp = new tPrintPopover(d->printer);
    connect(jp, &tPrintPopover::paintRequested, this, &PrintController::paintPrinter);

    tPopover* popover = new tPopover(jp);
    popover->setPopoverWidth(SC_DPI_W(-200, d->parentWidget));
    popover->setPopoverSide(tPopover::Bottom);
    connect(jp, &tPrintPopover::done, popover, &tPopover::dismiss);
    connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
    connect(popover, &tPopover::dismissed, jp, &tPrintPopover::deleteLater);
    connect(popover, &tPopover::dismissed, this, &PrintController::deleteLater);
    popover->show(d->parentWidget->window());
}

void PrintController::paintPrinter(QPrinter* printer) {
    QPainter painter;
    painter.begin(printer);

    QRectF dest = d->parentWidget->rect();
    QRectF fitter = printer->pageRect(QPrinter::DevicePixel);
    dest.setSize(dest.size().scaled(fitter.size(), Qt::KeepAspectRatio));
    dest.moveTopLeft(QPointF(0, 0));

    QPixmap screenPm(d->parentWidget->size());
    QPainter screenPmPainter(&screenPm);
    d->parentWidget->render(&screenPmPainter);
    screenPmPainter.end();

    painter.drawPixmap(dest, screenPm, QRectF());
}
