#include "mainwindow.h"
#include "testbackgroundthread.h"

#include <QApplication>

#include <QTest>

#define TEST false

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    if (TEST)
    {
        QTest::qExec(new TestBackgroundThread, argc, argv);
    }
    return a.exec();
}
