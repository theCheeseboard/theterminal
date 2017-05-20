#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QString workDir, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->addTerminal(workDir);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_terminal_finished()
{
    this->close();
}

void MainWindow::on_actionNew_Window_triggered()
{
    MainWindow* win = new MainWindow();
    win->show();
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::exit();
}

void MainWindow::on_actionCopy_triggered()
{
    currentTerminal->copyClipboard();
}

void MainWindow::on_actionPaste_triggered()
{
    currentTerminal->pasteClipboard();

}

void MainWindow::on_terminal_customContextMenuRequested(const QPoint &pos)
{
    QMenu* menu = new QMenu();
    menu->addAction(ui->actionCopy);
    menu->addAction(ui->actionPaste);

    menu->exec(currentTerminal->mapToGlobal(pos));
}

void MainWindow::on_terminal_copyAvailable(bool canCopy)
{
    ui->actionCopy->setEnabled(canCopy);

}

void MainWindow::addTerminal(QString workDir) {
    terminalWidget* widget = new terminalWidget(workDir);
    QPushButton* button = new QPushButton();
    widget->setContextMenuPolicy(Qt::CustomContextMenu);
    button->setCheckable(true);
    allTerminals.append(widget);
    terminalButtons.insert(widget, button);

    connect(widget, SIGNAL(copyAvailable(bool)), this, SLOT(on_terminal_copyAvailable(bool)));
    connect(widget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(on_terminal_customContextMenuRequested(QPoint)));
    connect(widget, &QTermWidget::finished, [=]() {
        closeTerminal(widget);
    });

    button->setText("Terminal " + QString::number(allTerminals.indexOf(widget) + 1));
    ui->tabFrame->layout()->addWidget(button);
    ui->tabFrame->layout()->removeItem(ui->horizontalSpacer);
    ui->tabFrame->layout()->addItem(ui->horizontalSpacer);
    connect(button, &QPushButton::clicked, [=]() {
        changeToTerminal(widget);
    });

    ui->centralWidget->layout()->addWidget(widget);

    changeToTerminal(widget);
}

void MainWindow::changeToTerminal(terminalWidget *widget) {
    for (terminalWidget* terminal : allTerminals) {
        terminal->setVisible(false);
    }
    for (QPushButton* button : terminalButtons.values()) {
        button->setChecked(false);
    }

    widget->setVisible(true);
    terminalButtons.value(widget)->setChecked(true);
    widget->setFocus();
    currentTerminal = widget;
}

void MainWindow::changeToTerminal(int index) {
    changeToTerminal(allTerminals.at(index));
}

void MainWindow::on_actionNew_Tab_triggered()
{
    addTerminal();
}

void MainWindow::closeTerminal(terminalWidget *widget) {
    if (allTerminals.count() == 1) {
        this->close();
    } else {
        if (allTerminals.indexOf(widget) == 0) {
            changeToTerminal(1);
        } else {
            changeToTerminal(allTerminals.indexOf(widget) - 1);
        }
        delete terminalButtons.value(widget);
        terminalButtons.remove(widget);
        allTerminals.removeOne(widget);
        delete widget;
    }
}

void MainWindow::on_actionClose_Tab_triggered()
{
    closeTerminal(currentTerminal);
}

void MainWindow::on_actionFind_triggered()
{
    currentTerminal->toggleShowSearchBar();
}

void MainWindow::on_actionGo_Full_Screen_triggered()
{
    if (this->isFullScreen()) {
        this->showNormal();
        ui->tabFrame->setVisible(true);
    } else {
        this->showFullScreen();
        ui->tabFrame->setVisible(false);
    }
}

void MainWindow::on_actionAbout_triggered()
{
    About* window = new About(this);
    window->exec();
}

void MainWindow::on_actionSettings_triggered()
{
    SettingsWindow* settings = new SettingsWindow();
    settings->exec();
    settings->deleteLater();
}
