#include "backgroundthread.h"

#include <QTextStream>

BackgroundThread::BackgroundThread()
    : quit(false)
    , finish(false)
    , cancel(false)
    , path("")
    , pattern("")
    , deepSearch([this]
        {
            for(;;)
            {
                std::unique_lock<std::mutex> lg(m);
                hasWork.wait(lg, [this]
                {
                    return (path != "" && pattern != "") || quit;
                });
                if (quit)
                    break;
                std::queue<QString>().swap(pathsToParse);
                std::queue<QString>().swap(ans);
                lg.unlock();
                grep();
                lg.lock();
                if (!cancel.load())
                {
                    finish.store(true);
                    if (!pathsToParse.empty())
                    {
                        isFinished.wait(lg, [this]
                        {
                            return pathsToParse.empty() || quit;
                        });
                    }
                }
                if (quit)
                    break;
                if (!cancel.load())
                {
                    path = "";
                    pattern = "";
                }
                cancel.store(false);
            }
        })
    {
        for (size_t i = 0; i < K; i++) {
            parseFile.push_back(std::thread([this]
            {
                for(;;)
                {
                    std::unique_lock<std::mutex> lg(m);
                    if (pathsToParse.empty())
                    {
                        hasFilesToParse.wait(lg, [this]
                        {
                            return !pathsToParse.empty() || quit;
                        });
                    }
                    if (quit)
                        break;
                    auto curPath = pathsToParse.front();
                    pathsToParse.pop();
                    QString curPattern = pattern;
                    std::vector<int> p(static_cast<size_t>(curPattern.size()));
                    init(curPattern, p);
                    lg.unlock();
                    grepFile(curPath, curPattern, p);
                    lg.lock();
                    if (quit)
                        break;
                    if (finish.load() && pathsToParse.empty())
                    {
                        isFinished.notify_one();
                    }
                }
            }));
        }
    }

BackgroundThread::~BackgroundThread()
{
    cancel.store(true);
    {
        std::unique_lock<std::mutex> lg(m);
        quit = true;
        hasWork.notify_all();
        hasFilesToParse.notify_all();
        isFull.notify_all();
        isFullToParse.notify_all();
        isFinished.notify_all();
    }
    deepSearch.join();
    std::for_each(parseFile.begin(), parseFile.end(), [](std::thread &thread) {
        thread.join();
    });
}

QString BackgroundThread::getResult()
{
    std::unique_lock<std::mutex> lg(m);
    if (ans.empty() || cancel.load() || quit)
    {
        return "";
    } else
    {
        auto res = ans.front();
        ans.pop();
        isFull.notify_one();
        return res;
    }
}

void BackgroundThread::setFindString(QString newPath, QString newPattern)
{
    std::unique_lock<std::mutex> lg(m);
    if (path != "")
        cancel.store(true);
    std::queue<QString>().swap(ans);
    std::queue<QString>().swap(pathsToParse);
    path = newPath;
    pattern = newPattern;
    hasWork.notify_all();
    isFull.notify_all();
    isFullToParse.notify_all();
    isFinished.notify_all();
}

void BackgroundThread::cancelThread()
{
    setFindString("", "");
}

void BackgroundThread::grep()
{
    qint64 all = 0;
    try
    {
        dfs1(path, all);
    } catch (std::exception)
    {
        return;
    }
    {
        std::unique_lock<std::mutex> lg(m);
        ans.push(QString::number(all));
    }
    try
    {
        dfs2(path);
    } catch (std::exception)
    {
        return;
    }
}

void BackgroundThread::dfs1(QString path, qint64 &all)
{
    if (cancel.load())
        throw std::runtime_error("The thread is canceled");
    QFileInfo info(path);
    if (info.isFile())
    {
        if (!info.isExecutable())
            all += info.size();
    } else
    {
        QDir dir(path);
        dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable | QDir::AllEntries | QDir::NoSymLinks);
        for (auto file : dir.entryInfoList())
        {
            dfs1(file.absoluteFilePath(), all);
        }
    }
}

void BackgroundThread::dfs2(QString path)
{
    if (cancel.load())
        throw std::runtime_error("The thread is canceled");
    QFileInfo info(path);
    if (info.isFile() && !info.isExecutable())
    {
        if (pathsToParse.size() > MAX_SIZE)
        {
            std::unique_lock<std::mutex> lg(m);
            isFullToParse.wait(lg, [this]
            {
                return pathsToParse.size() < MAX_SIZE || quit || cancel.load();
            });
        }
        if (quit)
            throw std::runtime_error("The thread is finished");
        if (cancel.load())
            throw std::runtime_error("The thread is canceled");
        pathsToParse.push(path);
        hasFilesToParse.notify_one();
    } else if (info.isDir())
    {
        QDir dir(path);
        dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable | QDir::AllEntries | QDir::NoSymLinks);
        for (auto file : dir.entryInfoList())
        {
            dfs2(file.absoluteFilePath());
        }
    }
}

void BackgroundThread::grepFile(QString path, QString curPattern, std::vector<int> &p)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        std::unique_lock<std::mutex> lg(m);
        if (ans.size() > MAX_SIZE)
        {
            isFull.wait(lg, [this]
            {
                return ans.size() < MAX_SIZE || quit || cancel.load();
            });
        }
        if (quit || cancel.load())
            return;
        ans.push("Can't open file " + path);
    } else
    {
        std::unique_lock<std::mutex> lg(m);
        lg.unlock();
        int m = curPattern.size();

        QTextStream stream(&file);
        size_t numOfLine = 1;
        while (!stream.atEnd())
        {
            if (cancel.load())
                break;
            QString cur = stream.readLine();
            if (isBinary(cur))
                break;
            int lastP = 0;
            for (auto it = cur.begin(); it != cur.end(); it++)
            {
                handle(p, *it, lastP, curPattern);
                if (lastP == m)
                {
                    lg.lock();
                    if (ans.size() > MAX_SIZE)
                    {
                        isFull.wait(lg, [this]
                        {
                            return ans.size() < MAX_SIZE || quit || cancel.load();
                        });
                    }
                    if (quit || cancel.load())
                        return;
                    ans.push(path + ":" + QString::number(numOfLine) + ":" + cur);
                    lg.unlock();
                    break;
                }
            }
            numOfLine++;
        }
        lg.lock();
        if (!cancel.load())
            ans.push(QString::number(file.size()) + " " + path);
    }
}

void BackgroundThread::init(QString const &str, std::vector<int> &p)
{
    int m = str.size();
    p[0] = 0;
    for (int i = 1; i < m; i++)
    {
        int j = p[static_cast<size_t>(i - 1)];
        while (j > 0 && str[i] != str[j])
        {
            j = p[static_cast<size_t>(j - 1)];
        }
        if (str[i] == str[j])
            j++;
        p[static_cast<size_t>(i)] = j;
    }
}

void BackgroundThread::handle(std::vector<int> &p, QChar cur, int &lastP, QString const &str)
{
    int j = lastP;
    while (j > 0 && (j == str.size() || cur != str[j]))
        j = p[static_cast<size_t>(j - 1)];
    if (cur == str[j])
            j++;
    lastP = j;
}

bool BackgroundThread::isBinary(const QString &str)
{
    for (auto it = str.begin(); it != str.end(); it++) {
        if (!it->isPrint())
            return true;
    }
    return false;
}
