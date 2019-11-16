#include "testbackgroundthread.h"
#include "backgroundthread.h"

#include <QDir>
#include <QTest>
#include <iostream>
#include <unistd.h>

TestBackgroundThread::TestBackgroundThread(QObject *parent) : QObject(parent)
{

}

void TestBackgroundThread::emptyDir()
{
    BackgroundThread bg;

    QCOMPARE(bg.getResult(), "");

    bg.setFindString(QDir::currentPath() + "/empty/", "nothing");
    sleep(1);

    QCOMPARE(bg.getResult(), "0");
    QCOMPARE(bg.getResult(), "");
}

void TestBackgroundThread::cancelThread()
{
    BackgroundThread bg;

    bg.setFindString(QDir::currentPath() + "/big/", "1");
    sleep(1);

    QCOMPARE(bg.getResult(), "7570026");
    QCOMPARE(bg.getResult(), QDir::currentPath() + "/big/file.txt:1:1");
    QCOMPARE(bg.getResult(), QDir::currentPath() + "/big/file.txt:2:1");
    bg.cancelThread();

    QCOMPARE(bg.getResult(), "");
}

void TestBackgroundThread::cancelAllThreads()
{
    BackgroundThread bg;

    bg.setFindString(QDir::currentPath() + "/poems/", "o");
    sleep(1);
    bg.getResult();
    bg.getResult();
    bg.cancelThread();

    QCOMPARE(bg.getResult(), "");
}

void TestBackgroundThread::findInOneFile()
{
    BackgroundThread bg;

    bg.setFindString(QDir::currentPath() + "/heap/", "int");
    sleep(1);

    QCOMPARE(bg.getResult(), "9365");
    QCOMPARE(bg.getResult(), "/home/gaporf/ITMO/C++/3 семестр/build-my_grep-Desktop_Qt_5_13_0_GCC_64bit-Debug/heap/main.cpp:5:const int MAX_VALUE = 2147483647;");
    QCOMPARE(bg.getResult(), "/home/gaporf/ITMO/C++/3 семестр/build-my_grep-Desktop_Qt_5_13_0_GCC_64bit-Debug/heap/main.cpp:6:const int MIN_VALUE = -2147483648;");
    QCOMPARE(bg.getResult(), "/home/gaporf/ITMO/C++/3 семестр/build-my_grep-Desktop_Qt_5_13_0_GCC_64bit-Debug/heap/main.cpp:11:        int key, index, degree, numHeap;");
    QCOMPARE(bg.getResult(), "/home/gaporf/ITMO/C++/3 семестр/build-my_grep-Desktop_Qt_5_13_0_GCC_64bit-Debug/heap/main.cpp:22:int n, curNumber = 0;");
    QCOMPARE(bg.getResult(), "/home/gaporf/ITMO/C++/3 семестр/build-my_grep-Desktop_Qt_5_13_0_GCC_64bit-Debug/heap/main.cpp:27:int getMinimum(ptrHeap heap) {");
    QCOMPARE(bg.getResult(), "/home/gaporf/ITMO/C++/3 семестр/build-my_grep-Desktop_Qt_5_13_0_GCC_64bit-Debug/heap/main.cpp:31:    int ans = MAX_VALUE;");
    QCOMPARE(bg.getResult(), "/home/gaporf/ITMO/C++/3 семестр/build-my_grep-Desktop_Qt_5_13_0_GCC_64bit-Debug/heap/main.cpp:40:ptrHeap merge(ptrHeap a, ptrHeap b, int NUM) {");
    QCOMPARE(bg.getResult(), "/home/gaporf/ITMO/C++/3 семестр/build-my_grep-Desktop_Qt_5_13_0_GCC_64bit-Debug/heap/main.cpp:163:void add(int v, int a) {");
    QCOMPARE(bg.getResult(), "/home/gaporf/ITMO/C++/3 семестр/build-my_grep-Desktop_Qt_5_13_0_GCC_64bit-Debug/heap/main.cpp:177:void extractMin(int a) {");
    QCOMPARE(bg.getResult(), "/home/gaporf/ITMO/C++/3 семестр/build-my_grep-Desktop_Qt_5_13_0_GCC_64bit-Debug/heap/main.cpp:213:void changeValue(ptrElement element, int value) {");
    QCOMPARE(bg.getResult(), "/home/gaporf/ITMO/C++/3 семестр/build-my_grep-Desktop_Qt_5_13_0_GCC_64bit-Debug/heap/main.cpp:264:int main() {");
    QCOMPARE(bg.getResult(), "/home/gaporf/ITMO/C++/3 семестр/build-my_grep-Desktop_Qt_5_13_0_GCC_64bit-Debug/heap/main.cpp:266:    int m;");
    QCOMPARE(bg.getResult(), "/home/gaporf/ITMO/C++/3 семестр/build-my_grep-Desktop_Qt_5_13_0_GCC_64bit-Debug/heap/main.cpp:269:    for (int i = 0; i < n; i++) {");
    QCOMPARE(bg.getResult(), "/home/gaporf/ITMO/C++/3 семестр/build-my_grep-Desktop_Qt_5_13_0_GCC_64bit-Debug/heap/main.cpp:273:        int type;");
    QCOMPARE(bg.getResult(), "/home/gaporf/ITMO/C++/3 семестр/build-my_grep-Desktop_Qt_5_13_0_GCC_64bit-Debug/heap/main.cpp:276:            int v, a;");
    QCOMPARE(bg.getResult(), "/home/gaporf/ITMO/C++/3 семестр/build-my_grep-Desktop_Qt_5_13_0_GCC_64bit-Debug/heap/main.cpp:281:            int a, b;");
    QCOMPARE(bg.getResult(), "/home/gaporf/ITMO/C++/3 семестр/build-my_grep-Desktop_Qt_5_13_0_GCC_64bit-Debug/heap/main.cpp:288:            int i;");
    QCOMPARE(bg.getResult(), "/home/gaporf/ITMO/C++/3 семестр/build-my_grep-Desktop_Qt_5_13_0_GCC_64bit-Debug/heap/main.cpp:294:            int i, v;");
    QCOMPARE(bg.getResult(), "/home/gaporf/ITMO/C++/3 семестр/build-my_grep-Desktop_Qt_5_13_0_GCC_64bit-Debug/heap/main.cpp:299:            int a;");
    QCOMPARE(bg.getResult(), "/home/gaporf/ITMO/C++/3 семестр/build-my_grep-Desktop_Qt_5_13_0_GCC_64bit-Debug/heap/main.cpp:304:            int a;");
    QCOMPARE(bg.getResult(), "9365 /home/gaporf/ITMO/C++/3 семестр/build-my_grep-Desktop_Qt_5_13_0_GCC_64bit-Debug/heap/main.cpp");
    QCOMPARE(bg.getResult(), "");
}

void TestBackgroundThread::findInManyFiles()
{
    BackgroundThread bg;

    bg.setFindString(QDir::currentPath() + "/poems/", "pa");
    sleep(1);

    std::vector<QString> v;
    while (true)
    {
        auto str = bg.getResult();
        if (str == "") {
            break;
        }
        v.push_back(str);
    }
    std::sort(v.begin(), v.end());
    QCOMPARE(v[0], "/home/gaporf/ITMO/C++/3 семестр/build-my_grep-Desktop_Qt_5_13_0_GCC_64bit-Debug/poems/fifth.txt:10:In her pavilion, cloth-of-gold of tissue,");
    QCOMPARE(v[8], "/home/gaporf/ITMO/C++/3 семестр/build-my_grep-Desktop_Qt_5_13_0_GCC_64bit-Debug/poems/first.txt:3:Over park, over pale,");
    QCOMPARE(v[12], "/home/gaporf/ITMO/C++/3 семестр/build-my_grep-Desktop_Qt_5_13_0_GCC_64bit-Debug/poems/fourth.txt:4:And one man in his time plays many parts,");
    QCOMPARE(v[16], "/home/gaporf/ITMO/C++/3 семестр/build-my_grep-Desktop_Qt_5_13_0_GCC_64bit-Debug/poems/second.txt:191:The one a palate hath that needs will taste,");
    QCOMPARE(v.back(), "506 /home/gaporf/ITMO/C++/3 семестр/build-my_grep-Desktop_Qt_5_13_0_GCC_64bit-Debug/poems/third.txt");
    QCOMPARE(bg.getResult(), "");
}

void TestBackgroundThread::missBinaryFiles()
{
    BackgroundThread bg;

    bg.setFindString(QDir::currentPath() + "/backgroundthread.o", "i");
    sleep(1);
    bg.getResult();
    bg.getResult();

    QCOMPARE(bg.getResult(), "");
}

void TestBackgroundThread::openFileIsToHard()
{
    BackgroundThread bg;

    bg.setFindString(QDir::currentPath() + "/openIsToHard/", "You're lose");
    sleep(1);
    bg.getResult();
    bg.getResult();

    QCOMPARE(bg.getResult(), "");
}
