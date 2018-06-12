#include "ttedcommand.h"
#include "ui_ttedcommand.h"

ttedCommand::ttedCommand(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ttedCommand)
{
    ui->setupUi(this);

    ui->editor->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    QPalette pal = ui->filename->palette();
    QColor temp = pal.color(QPalette::WindowText);
    pal.setColor(QPalette::WindowText, pal.color(QPalette::Window));
    pal.setColor(QPalette::Window, temp);
    ui->filename->setPalette(pal);

    ui->saveWarningLabel->setVisible(false);
    ui->permissionWarningLabel->setVisible(false);
}

ttedCommand::~ttedCommand()
{
    watcher->deleteLater();
    delete ui;
}

void ttedCommand::loadFile(QString file) {
    QString statusBar = file;
    watcher = new QFileSystemWatcher();
    watcher->addPath(file);
    connect(watcher, &QFileSystemWatcher::fileChanged, [=] {
        if (!justSaved) {
            ui->saveWarningLabel->setVisible(true);
        } else {
            justSaved = false;
        }
    });

    loadedFile = file;
    QFile fileObj(file);
    QFileInfo fileInfo(file);
    if (fileObj.exists()) {
        fileObj.open(QFile::ReadOnly);
        ui->editor->setPlainText(fileObj.readAll());
        fileObj.close();

        if (!fileInfo.isWritable()) {
            ui->permissionWarningLabel->setVisible(true);
            ui->saveButton->setVisible(false);
        }
    } else {
        statusBar.append(" [NEW FILE]");
    }
    ui->filename->setText(statusBar);
}

void ttedCommand::on_editor_textChanged()
{
    tVariantAnimation* anim = new tVariantAnimation();
    anim->setStartValue(ui->editor->height());
    anim->setEndValue((ui->editor->blockCount() + 2) * ui->editor->fontMetrics().height());
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->setDuration(250);
    connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
        ui->editor->setFixedHeight(value.toInt());
    });
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    anim->start();
}

void ttedCommand::on_editor_undoAvailable(bool b)
{
    ui->undoButton->setEnabled(b);
}

void ttedCommand::on_editor_redoAvailable(bool b)
{
    ui->redoButton->setEnabled(b);
}

void ttedCommand::on_saveButton_clicked()
{
    justSaved = true;
    QFile fileObj(loadedFile);
    fileObj.open(QFile::WriteOnly);
    fileObj.write(ui->editor->toPlainText().toUtf8());
    fileObj.close();

    ui->saveWarningLabel->setVisible(false);
}
