#include "terminalstatus.h"
#include "ui_terminalstatus.h"

TerminalStatus::TerminalStatus(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TerminalStatus)
{
    ui->setupUi(this);

    ui->gitIcon->setPixmap(QIcon::fromTheme("branch").pixmap(16, 16));
    ui->gitCommitIcon->setPixmap(QIcon::fromTheme("commit").pixmap(16, 16));
}

TerminalStatus::~TerminalStatus()
{
    delete ui;
}

void TerminalStatus::setDir(QDir dir) {
    this->dir = dir;

    ui->workingDir->setText(dir.absolutePath().replace(QDir::homePath(), "~"));

    ui->gitWidget->setVisible(false);
    ui->gitCommitWidget->setVisible(false);
    QProcess* gitDetect = new QProcess();
    connect(gitDetect, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=](int exitCode, QProcess::ExitStatus exitStatus) {
        gitDetect->deleteLater();

        if (exitCode == 0) {
            QStringList branches = QString(gitDetect->readAll()).split("\n", QString::SkipEmptyParts);
            for (QString branch : branches) {
                if (branch.startsWith("*")) {
                    ui->gitLabel->setText(branch.mid(2).trimmed());
                    ui->gitWidget->setVisible(true);
                }
            }
        }
    });
    gitDetect->start("git -C \"" + dir.path() + "\" branch --color=never");

    QProcess* gitCommit = new QProcess();
    connect(gitCommit, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=](int exitCode, QProcess::ExitStatus exitStatus) {
        gitCommit->deleteLater();

        if (exitCode == 0) {
            ui->gitCommit->setText(gitCommit->readAll().trimmed());
            ui->gitCommitWidget->setVisible(true);
        }
    });
    gitCommit->start("git -C \"" + dir.path() + "\" rev-parse --short HEAD");
}
