#ifndef TTEDCOMMAND_H
#define TTEDCOMMAND_H

#include <QWidget>
#include <QFile>
#include <QPlainTextEdit>
#include <QLabel>
#include <tvariantanimation.h>
#include <QToolButton>
#include <QFileInfo>
#include <QFileSystemWatcher>

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
        void on_editor_textChanged();

        void on_editor_undoAvailable(bool b);

        void on_editor_redoAvailable(bool b);

        void on_saveButton_clicked();

    private:
        Ui::ttedCommand *ui;

        QString loadedFile;
        QFileSystemWatcher* watcher;
        bool justSaved = false;
};

#endif // TTEDCOMMAND_H
