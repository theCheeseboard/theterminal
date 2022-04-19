#include "history.h"

History::History(QObject* parent) :
    QObject(parent) {
    watcher = new QFileSystemWatcher();
    connect(watcher, &QFileSystemWatcher::fileChanged, this, [=] {
        if (justSaved) {
            justSaved = false;
        } else {
            loadHistory();
        }
    });

    loadHistory();
}

void History::loadHistory() {
    QFile historyFile(QDir::homePath() + "/.theterminal_history");
    historyFile.open(QFile::ReadOnly);
    items = QString(historyFile.readAll()).split("\n", Qt::SkipEmptyParts);
    historyFile.close();

    if (watcher->files().isEmpty()) {
        watcher->addPath(QDir::homePath() + "/.theterminal_history");
    }
}

void History::appendToHistory(QString item) {
    clearState();
    items.prepend(item);
    while (items.count() > 500) {
        items.takeLast();
    }
    saveHistory();
}

void History::saveHistory() {
    justSaved = true;
    QFile historyFile(QDir::homePath() + "/.theterminal_history");
    historyFile.open(QFile::WriteOnly);
    historyFile.write(items.join("\n").toUtf8());
    historyFile.close();

    if (watcher->files().isEmpty()) {
        watcher->addPath(QDir::homePath() + "/.theterminal_history");
    }
}

void History::clearState() {
    currentIndex = -1;
    temporaryItems.clear();
}

QString History::historyDown(QString currentText) {
    temporaryItems.insert(currentIndex, currentText);
    currentIndex--;
    if (currentIndex == -2) currentIndex++;
    return historyItem(currentIndex);
}

QString History::historyUp(QString currentText) {
    temporaryItems.insert(currentIndex, currentText);
    currentIndex++;
    if (currentIndex == items.count()) currentIndex--;
    return historyItem(currentIndex);
}

QString History::historyItem(int index) {
    if (temporaryItems.contains(index)) return temporaryItems.value(index);
    return items.at(index);
}
