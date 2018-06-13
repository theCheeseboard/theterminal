#ifndef HISTORY_H
#define HISTORY_H

#include <QObject>
#include <QFile>
#include <QDir>
#include <QFileSystemWatcher>
#include <QMap>

class History : public QObject
{
        Q_OBJECT
    public:
        explicit History(QObject *parent = nullptr);
        History* instance();

    signals:

    public slots:
        void appendToHistory(QString item);
        void clearState();
        QString historyUp(QString currentText);
        QString historyDown(QString currentText);
        QString historyItem(int index);

    private slots:
        void loadHistory();
        void saveHistory();

    private:
        QStringList items;
        QMap<int, QString> temporaryItems;
        QFileSystemWatcher* watcher;
        bool justSaved = false;
        int currentIndex = -1;
};

#endif // HISTORY_H
