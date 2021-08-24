#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include "videocaptured.h"

//QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
//QT_END_NAMESPACE

class VideoCaptured;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void on_browseConfigButton_clicked();

    void on_startCamera_clicked();

    void on_stopCamera_clicked();

    void on_browseWeightsButton_clicked();

    void on_browseClassesButton_clicked();
    void on_actionAbout_OpenQVision_triggered();

    void on_actionLicense_triggered();

private:
    VideoCaptured *mOpencv_VideoCapture;
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
