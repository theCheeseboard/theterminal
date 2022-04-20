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
#include "remoteconnectionpopover.h"
#include "ui_remoteconnectionpopover.h"

#include "terminalwidget.h"
#include <tcontentsizer.h>

RemoteConnectionPopover::RemoteConnectionPopover(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::RemoteConnectionPopover) {
    ui->setupUi(this);

    ui->titleLabel->setBackButtonShown(true);
    new tContentSizer(ui->connectionSettingsPane);
    new tContentSizer(ui->connectButton);
}

RemoteConnectionPopover::~RemoteConnectionPopover() {
    delete ui;
}

void RemoteConnectionPopover::on_titleLabel_backButtonClicked() {
    emit done();
}

void RemoteConnectionPopover::on_connectButton_clicked() {
    QString command;
    QStringList args;

    if (ui->sshButton->isChecked()) {
        command = "ssh";
        args = {
            ui->hostBox->text()};
    } else if (ui->telnetButton->isChecked()) {
        command = "telnet";
        args = {
            ui->hostBox->text()};
    }

    TerminalWidget* term = new TerminalWidget("", command, args);
    emit openTerminal(term);
    emit done();
}
