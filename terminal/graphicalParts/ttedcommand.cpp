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
}

ttedCommand::~ttedCommand()
{
    delete ui;
}

void ttedCommand::loadFile(QString file) {
    QString statusBar = file;

    loadedFile = file;
    QFile fileObj(file);
    if (fileObj.exists()) {
        fileObj.open(QFile::ReadOnly);
        ui->editor->setPlainText(fileObj.readAll());
        fileObj.close();
    } else {
        statusBar.append(" [NEW FILE]");
    }
    ui->filename->setText(statusBar);
}

void ttedCommand::on_saveButton_triggered(QAction *arg1)
{
    QFile fileObj(loadedFile);
    fileObj.open(QFile::WriteOnly);
    fileObj.write(ui->editor->toPlainText().toUtf8());
    fileObj.close();

    ui->saveWarningLabel->setVisible(false);
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
