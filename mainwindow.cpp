#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "curl_telegram.h"
#include <iostream>
#include "videocaptured.h"
#include "QFileDialog"

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("OpenQVision");
    ui->stopCamera->setDisabled(true);

}

MainWindow::~MainWindow()
{
    delete ui;
    mOpencv_VideoCapture->terminate();
}

void MainWindow::on_browseWeightsButton_clicked()
{
    QString weightsName = QFileDialog::getOpenFileName(this, "Open a file", "directoryToOpen",
            "Weights Files (*.weights *.pb)");
    ui->weightsPath->setText(weightsName);
}

void MainWindow::on_browseConfigButton_clicked()
{
    QString configName = QFileDialog::getOpenFileName(this, "Open a file", "directoryToOpen",
            "Config Files (*.cfg *.pbtxt)");
    ui->configPath->setText(configName);
}

void MainWindow::on_browseClassesButton_clicked()
{
    QString classesName = QFileDialog::getOpenFileName(this, "Open a file", "directoryToOpen",
            "Config Files (*.names *.txt)");
    ui->classesPath->setText(classesName);
}


void MainWindow::on_startCamera_clicked()
{
    QString classesPath = this->ui->classesPath->text();
    QString configPath = this->ui->configPath->text();
    QString weightsPath = this->ui->weightsPath->text();
    mOpencv_VideoCapture = new VideoCaptured(this, classesPath, configPath, weightsPath);
    connect(mOpencv_VideoCapture, &VideoCaptured::newPixmapCaptured, this, [&](){
        ui->videoDisplay->setPixmap(mOpencv_VideoCapture->pixmap().scaled(640,480));
    });
    mOpencv_VideoCapture->start(QThread::HighestPriority);
    ui->startCamera->setDisabled(true);
    ui->stopCamera->setDisabled(false);
}

void MainWindow::on_stopCamera_clicked()
{
    mOpencv_VideoCapture->stop();
    mOpencv_VideoCapture->terminate();
    ui->startCamera->setDisabled(false);
    ui->stopCamera->setDisabled(true);
}
