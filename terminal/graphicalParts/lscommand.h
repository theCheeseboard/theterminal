#ifndef LSCOMMAND_H
#define LSCOMMAND_H

#include <QWidget>
#include <QDir>
#include <QFileSystemModel>
#include <QListView>
#include <QTimer>
#include <tvariantanimation.h>

namespace Ui {
    class lsCommand;
}

class lsCommand : public QWidget
{
        Q_OBJECT

    public:
        explicit lsCommand(QWidget *parent = 0);
        ~lsCommand();

    public slots:
        void setDir(QDir dir, QStringList args);

    signals:
        void executeCommands(QStringList commands);

    private slots:
        void on_filesListView_activated(const QModelIndex &index);

    private:
        Ui::lsCommand *ui;
        QFileSystemModel* model;
        QStringList args;
};

#endif // LSCOMMAND_H
