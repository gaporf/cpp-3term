#ifndef BACKGROUNDTHREAD_H
#define BACKGROUNDTHREAD_H

#include <QObject>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <QFile>
#include <QDir>
#include <QTimer>

#include <vector>
#include <queue>

struct BackgroundThread : QObject
{
public:

    BackgroundThread();

    ~BackgroundThread();

    QString getResult();

    void setFindString(QString newPath, QString newPattern);

    void cancelThread();

private:

    void grep();

    void dfs1(QString path, qint64 &all);

    void dfs2(QString path);

    void grepFile(QString path, QString pattern, std::vector<int> &p);

    void init(QString const &str, std::vector<int> &p);

    void handle(std::vector<int> &p, QChar cur, int &lastP, QString const &str);

    bool isBinary(QString const &str);

private:

    static const size_t K = 6;

    static const size_t MAX_SIZE = 10000;

    bool quit;

    std::atomic<bool> finish, cancel;

    std::mutex m;

    std::condition_variable hasWork, hasFilesToParse, isFull, isFullToParse, isFinished;

    QString path;

    QString pattern;

    std::queue<QString> pathsToParse;

    std::queue<QString> ans;

    std::thread deepSearch;

    std::vector<std::thread> parseFile;

};

#endif // BACKGROUNDTHREAD_H
