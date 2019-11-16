#ifndef TESTBACKGROUNDTHREAD_H
#define TESTBACKGROUNDTHREAD_H

#include <QObject>

struct TestBackgroundThread : public QObject
{
    Q_OBJECT
public:
    explicit TestBackgroundThread(QObject *parent = nullptr);

private slots:

    void emptyDir();

    void cancelThread();

    void cancelAllThreads();

    void findInOneFile();

    void findInManyFiles();

    void missBinaryFiles();

    void openFileIsToHard();

};

#endif // TESTBACKGROUNDTHREAD_H
