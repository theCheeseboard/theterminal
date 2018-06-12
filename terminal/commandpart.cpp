#include "commandpart.h"
#include "ui_commandpart.h"

CommandPart::CommandPart(TerminalWidget* parentTerminal, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CommandPart)
{
    ui->setupUi(this);

    ui->commandLabel->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    ui->returnValue->setVisible(false);
    ui->expandButton->setVisible(false);
    this->parentTerminal = parentTerminal;
}

CommandPart::~CommandPart()
{
    delete ui;
}

void CommandPart::setCommandText(QString text) {
    ui->commandLabel->setText(text);
}

void CommandPart::setReturnValue(int retval) {
    ui->returnValue->setText(QString::number(retval));
    ui->returnValue->setVisible(true);
    ui->workingSpinner->setVisible(false);

    if (retval != 0) {
        QPalette pal = this->palette();
        pal.setColor(QPalette::Window, QColor(200, 0, 0));
        pal.setColor(QPalette::WindowText, Qt::white);
        this->setPalette(pal);
    }
}

void CommandPart::executeCommand(int height, QString command) {
    QStringList args = command.split(" ");
    QString executable = args.takeFirst();

    currentTerminal = new TerminalPart(0);
    ((QBoxLayout*) ui->frame->layout())->addWidget(currentTerminal);
    currentTerminal->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    currentTerminal->setWorkingDirectory(parentTerminal->getWorkingDir().path());
    currentTerminal->setAutoClose(false);
    currentTerminal->setShellProgram(executable);
    currentTerminal->setArgs(args);
    currentTerminal->setEnvironment(env.toStringList());
    currentTerminal->setFocus();

    //Run the animation to hide the artifacts of resizing an app such as nano
    tVariantAnimation* anim = new tVariantAnimation();
    anim->setStartValue(0);
    anim->setEndValue(height);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->setDuration(500);
    connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
        this->setFixedHeight(value.toInt());
    });
    anim->start();

    connect(currentTerminal, &TerminalPart::shellProgramFinished, [=](int exitCode) {
        if (anim->state() == tVariantAnimation::Running) {
            anim->stop();
        }
        anim->deleteLater();

        this->setReturnValue(exitCode);
        emit finished(exitCode);

        ui->expandButton->setVisible(true);
        tVariantAnimation* anim = new tVariantAnimation();
        anim->setStartValue(currentTerminal->height());
        anim->setEndValue(0);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        anim->setDuration(250);
        connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
            currentTerminal->setFixedHeight(value.toInt());
        });
        connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
        anim->start();
        expanded = false;

        this->setFixedHeight(QWIDGETSIZE_MAX);
        this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    });
    currentTerminal->startShellProgram();
}

void CommandPart::appendOutput(QString text) {
    if (inbuiltOutputLabel == nullptr) {
        inbuiltOutputLabel = new QLabel();
        inbuiltOutputLabel->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
        inbuiltOutputLabel->setWordWrap(true);
        inbuiltOutputLabel->setMargin(9);

        ((QBoxLayout*) ui->frame->layout())->addWidget(inbuiltOutputLabel);
    }

    outputLines.append(text);
    inbuiltOutputLabel->setText(outputLines.join(" "));
}

void CommandPart::on_expandButton_clicked()
{
    tVariantAnimation* anim = new tVariantAnimation();
    anim->setStartValue(currentTerminal->height());

    if (expanded) {
        anim->setEndValue(0);
        expanded = false;
        ui->expandButton->setIcon(QIcon::fromTheme("go-up"));
    } else {
        anim->setEndValue(20 * currentTerminal->fontHeight());
        expanded = true;
        ui->expandButton->setIcon(QIcon::fromTheme("go-down"));
    }

    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->setDuration(250);
    connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
        currentTerminal->setFixedHeight(value.toInt());
    });
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    anim->start();
}

void CommandPart::executeWidget(QWidget *widget) {
    ((QBoxLayout*) ui->frame->layout())->addWidget(widget);
}

void CommandPart::setEnvironment(QProcessEnvironment env) {
    this->env = env;
}
