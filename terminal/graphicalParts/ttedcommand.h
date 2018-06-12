#ifndef TTEDCOMMAND_H
#define TTEDCOMMAND_H

#include <QWidget>
#include <QFile>
#include <QPlainTextEdit>
#include <QLabel>
#include <tvariantanimation.h>

namespace Ui {
    class ttedCommand;
}

class ttedCommand : public QWidget
{
        Q_OBJECT

    public:
        explicit ttedCommand(QWidget *parent = 0);
        ~ttedCommand();

    public slots:
        void loadFile(QString file);

    private slots:
        void on_saveButton_triggered(QAction *arg1);

        void on_editor_textChanged();

    private:
        Ui::ttedCommand *ui;

        QString loadedFile;
};

#endif // TTEDCOMMAND_H
