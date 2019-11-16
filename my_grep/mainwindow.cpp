#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QDialog>
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    timer = new QTimer(this);
    timer->setInterval(100);
    timer->start();

    connect(timer, &QTimer::timeout, this, [this]
    {
        QString res = bg.getResult();
        if (res != "")
        {
            if (res[0].isDigit())
            {
                if (all == 0)
                {
                    all = res.toLong();
                    ui->statInfo->setText("All files for search are found! Start to parse...");
                }
                else
                {
                    QString number, parsedPath;
                    for (auto it = res.begin(); it != res.end(); it++)
                    {
                        if (*it != ' ')
                            number.append(*it);
                        else
                        {
                            it++;
                            while (it != res.end()) {
                                parsedPath.append(*it);
                                it++;
                            }
                            break;
                        }
                    }
                    cur += number.toLong();

                    ui->statInfo->setText("The file " + parsedPath + " is parsed!");
                }
                if (cur == all)
                {
                    ui->statInfo->setText("The search is finished!");
                    ui->cancelButton->setEnabled(false);
                }
                ui->progressBar->setValue(static_cast<int>((all == 0) ? 100 : cur * 100 / all));
            } else {
                ui->listWidget->addItem(res);
            }
        }
    });

    connect(ui->openButton, &QPushButton::clicked, this, [this]
    {
        QString beg = (ui->dirInfo->text() == "The path is correct!" ? ui->dirLine->text() : QDir::root().absolutePath());
        if (ui->fileRadio->isChecked())
        {
            ui->dirLine->setText(QFileDialog::getOpenFileName(this, tr("Chose file"), beg));
        } else
        {
            ui->dirLine->setText(QFileDialog::getExistingDirectory(this, tr("Chose directory"),
                                                               beg,
                                                               QFileDialog::ShowDirsOnly
                                                               | QFileDialog::DontResolveSymlinks));
        }
        isPathCorrect();
    });

    connect(ui->dirLine, &QLineEdit::textChanged, this, [this]
    {
       isPathCorrect();
    });

    connect(ui->fileRadio, &QRadioButton::clicked, this, [this]
    {
        isPathCorrect();
    });

    connect(ui->dirRadio, &QRadioButton::clicked, this, [this]
    {
        isPathCorrect();
    });

    connect(ui->findButton, &QPushButton::clicked, this, [this]
    {
        if (ui->dirInfo->text() == "The path is correct!")
        {
            all = 0;
            cur = 0;
            ui->listWidget->clear();
            ui->progressBar->setValue(0);
            ui->statInfo->setText("Starting searching...");
            bg.setFindString(ui->dirLine->text(), ui->findLine->text());
            ui->cancelButton->setEnabled(true);
            ui->findButton->setEnabled(false);
        }
    });

    connect(ui->findLine, &QLineEdit::textChanged, this, [this]
    {
       ui->findButton->setEnabled(ui->dirInfo->text() == "The path is correct!" && ui->findLine->text() != "");
    });

    connect(ui->cancelButton, &QPushButton::clicked, this, [this]
    {
        bg.cancelThread();
        ui->statInfo->clear();
        ui->cancelButton->setEnabled(false);
        ui->progressBar->setValue(0);
        ui->statInfo->setText("The search is canceled!");
    });

    connect(ui->exitButton, &QPushButton::clicked, this, [this]
    {
        QWidget::close();
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::isPathCorrect()
{
    QFileInfo info(ui->dirLine->text());
    if (info.exists() && ((ui->fileRadio->isChecked() && info.isFile()) || (ui->dirRadio->isChecked() && info.isDir())))
    {
         ui->dirInfo->setText("The path is correct!");
         ui->findButton->setEnabled(true && ui->findLine->text() != "");
    }
    else
    {
        ui->dirInfo->setText("The path is wrong!");
        ui->findButton->setEnabled(false);
    }
}
