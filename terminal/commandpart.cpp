#include "commandpart.h"
#include "ui_commandpart.h"

extern QStringList splitSpaces(QString str);

CommandPart::CommandPart(TerminalWidget* parentTerminal, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CommandPart)
{
    ui->setupUi(this);

    ui->commandLabel->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    ui->returnValue->setVisible(false);
    ui->expandButton->setVisible(false);
    ui->dismissButton->setVisible(false);
    this->parentTerminal = parentTerminal;

    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect();
    this->setGraphicsEffect(effect);
    tPropertyAnimation* anim = new tPropertyAnimation(effect, "opacity");
    anim->setStartValue((float) 0);
    anim->setEndValue((float) 1);
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &tPropertyAnimation::finished, [=] {
        this->setGraphicsEffect(nullptr);
    });
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    connect(this, SIGNAL(destroyed(QObject*)), anim, SLOT(stop()));
    anim->start();
}

CommandPart::~CommandPart()
{
    delete ui;
}

void CommandPart::setCommandText(QString text) {
    ui->commandLabel->setText(text);
}

void CommandPart::setReturnValue(int retval) {
    if (retval == 0) {
        ui->returnValue->setPixmap(QIcon::fromTheme("dialog-ok").pixmap(16, 16));
    } else {
        ui->returnValue->setText(QString::number(retval));
    }

    switch (retval) {
        case 128 + SIGABRT:
            this->appendOutput(tr("Aborted"));
            break;
        case 128 + SIGFPE:
            this->appendOutput(tr("Floating Point Exception"));
            break;
        case 128 + SIGSEGV:
            this->appendOutput(tr("Segmentation Fault"));
            break;
        case 128 + SIGKILL:
            this->appendOutput(tr("Killed"));
            break;
    }

    ui->returnValue->setVisible(true);
    ui->spinnerContainer->setVisible(false);
    ui->dismissButton->setVisible(true);

    if (retval != 0) {
        QPalette pal = this->palette();
        pal.setColor(QPalette::Window, QColor(200, 0, 0));
        pal.setColor(QPalette::WindowText, Qt::white);
        this->setPalette(pal);
    }
}

void CommandPart::executeCommand(int height, QProcess* pipe, QString command) {
    QStringList args = splitSpaces(command);
    QString executable = args.takeFirst();

    //bool connectPty = (pipe == nullptr);
    currentTerminal = new TerminalPart(true, 0);
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

    if (pipe != nullptr) {
        connect(pipe, &QProcess::readyReadStandardOutput, [=] {
            currentTerminal->getProcess()->write(pipe->readAllStandardOutput());
        });
        connect(pipe, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=](int exitCode, QProcess::ExitStatus exitStatus){
            currentTerminal->getProcess()->close();
        });
    }
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


void CommandPart::on_dismissButton_clicked()
{
    //emit dismiss();
    close();
}

void CommandPart::close() {
    QVariantAnimation* h = new QVariantAnimation();
    h->setStartValue(this->height());
    h->setEndValue(0);
    h->setDuration(250);
    h->setEasingCurve(QEasingCurve::OutCubic);
    connect(h, &QVariantAnimation::valueChanged, [=](QVariant value) {
        this->setFixedHeight(value.toInt());
    });
    connect(h, SIGNAL(finished()), this, SIGNAL(dismiss()));
    h->start();
}
