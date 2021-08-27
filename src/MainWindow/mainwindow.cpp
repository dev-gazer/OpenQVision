#include "MainWindow/mainwindow.h"
#include "ui_mainwindow.h"
#include "Telegram/curl_telegram.h"
#include <iostream>
#include "ObjectDetection/videocaptured.h"
#include "QFileDialog"
#include "QMessageBox"
#include "QFile"
#include "QString"
#include "QTextStream"

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

void MainWindow::on_browseMediaButton_clicked()
{
    QString mediaName = QFileDialog::getOpenFileName(this, "Open a file", "directoryToOpen",
            "Media Files (*.mp4 *.mpeg *.mpg *jpeg *.jpg *.png )");
    ui->mediaPath->setText(mediaName);

}

void MainWindow::on_mediaPath_textChanged(const QString &arg1)
{
    if (ui->mediaPath->text() != ""){
        ui->startCamera->setText("Read File");
        ui->stopCamera->setText("Stop");
    } else {
        ui->startCamera->setText("Start Camera");
        ui->stopCamera->setText("Stop Camera");
    }
}


void MainWindow::on_startCamera_clicked()
{
    QString classesPath = this->ui->classesPath->text();
    QString configPath = this->ui->configPath->text();
    QString weightsPath = this->ui->weightsPath->text();
    QString mediaPath = this->ui->mediaPath->text();
    QString telUserId = this->ui->userIDInput->text();
    QString telBotToken = this->ui->botTokenInput->text();
    mOpencv_VideoCapture = new VideoCaptured(this, classesPath, configPath, weightsPath, mediaPath, telUserId, telBotToken);
    connect(mOpencv_VideoCapture, &VideoCaptured::newPixmapCaptured, this, [&](){
        ui->videoDisplay->setPixmap(mOpencv_VideoCapture->pixmap().scaled(640,480));
    });
    mOpencv_VideoCapture->start(QThread::HighestPriority);
    if( mediaPath.toStdString().substr(mediaPath.toStdString().find_last_of(".") + 1) != "png" &&
            mediaPath.toStdString().substr(mediaPath.toStdString().find_last_of(".") + 1) != "jpg" &&
            mediaPath.toStdString().substr(mediaPath.toStdString().find_last_of(".") + 1) != "jpeg"){
    ui->startCamera->setDisabled(true);
    ui->stopCamera->setDisabled(false);
    }
}

void MainWindow::on_stopCamera_clicked()
{
    mOpencv_VideoCapture->stop();
    mOpencv_VideoCapture->terminate();
    ui->startCamera->setDisabled(false);
    ui->stopCamera->setDisabled(true);
}

void MainWindow::on_actionAbout_OpenQVision_triggered()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("About OpenQVision");
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText("OpenQVision v1.1.1 is a computer vision project that allows developers "
                   "and/or data scientists to rapidly test their newborn neural networks "
                   "on camera streaming, video or photos. As the name itself refers to, "
                   "it is based on OpenCV (https://github.com/opencv/opencv) and Qt "
                   "framework (https://github.com/qt) and handles Tensorflow "
                   "(https://github.com/tensorflow/tensorflow) and Yolo Neural Networks "
                   "(https://github.com/pjreddie/darknet). "
                   "OpenQVision idealized and developed by Alexandre Martuscelli Faria. "
                   "Credits to Tom Ha (https://github.com/tomha85), Nico Nielsen (https://github.com/niconielsen32), "
                   "Audrey Siqueira (https://github.com/audrey-siqueira) "
                   "and C++ community on GitHub.");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    int ret = msgBox.exec();
}

void MainWindow::on_actionLicense_triggered()
{
    QMessageBox msgBox;
    QString msg = "GNU LESSER GENERAL PUBLIC LICENSE"
                  "                       Version 3, 29 June 2007\n"
                  ""
                  "Copyright (c) 2021 Alexandre Martuscelli Faria\n"
                  ""
                  "Copyright (C) 2007 Free Software Foundation, Inc. http://fsf.org\n"
                  "Everyone is permitted to copy and distribute verbatim copies "
                  "of this license document, but changing it is not allowed.\n"
                  ""
                  ""
                  "This version of the GNU Lesser General Public License incorporates "
                  "the terms and conditions of version 3 of the GNU General Public "
                  "License, supplemented by the additional permissions listed below.\n"
                  ""
                  "  0. Additional Definitions."
                  ""
                  "As used herein, 'this License' refers to version 3 of the GNU Lesser "
                  "General Public License, and the 'GNU GPL' refers to version 3 of the GNU "
                  "General Public License.\n"
                  ""
                  "'The Library' refers to a covered work governed by this License, "
                  "other than an Application or a Combined Work as defined below.\n"
                  ""
                  "An 'Application' is any work that makes use of an interface provided "
                  "by the Library, but which is not otherwise based on the Library.\n"
                  "Defining a subclass of a class defined by the Library is deemed a mode "
                  "of using an interface provided by the Library.\n"
                  ""
                  "A 'Combined Work' is a work produced by combining or linking an "
                  "Application with the Library.  The particular version of the Library "
                  "with which the Combined Work was made is also called the 'Linked "
                  "Version'.\n"
                  ""
                  "The 'Minimal Corresponding Source' for a Combined Work means the "
                  "Corresponding Source for the Combined Work, excluding any source code "
                  "for portions of the Combined Work that, considered in isolation, are "
                  "based on the Application, and not on the Linked Version.\n"
                  ""
                  "The 'Corresponding Application Code' for a Combined Work means the "
                  "object code and/or source code for the Application, including any data "
                  "and utility programs needed for reproducing the Combined Work from the "
                  "Application, but excluding the System Libraries of the Combined Work.\n"
                  ""
                  "1. Exception to Section 3 of the GNU GPL.\n"
                  ""
                  "You may convey a covered work under sections 3 and 4 of this License "
                  "without being bound by section 3 of the GNU GPL.\n"
                  ""
                  "2. Conveying Modified Versions.\n"
                  ""
                  "If you modify a copy of the Library, and, in your modifications, a "
                  "facility refers to a function or data to be supplied by an Application "
                  "that uses the facility (other than as an argument passed when the "
                  "facility is invoked), then you may convey a copy of the modified "
                  "version:\n"
                  ""
                  "   a) under this License, provided that you make a good faith effort to "
                  "   ensure that, in the event an Application does not supply the "
                  "   function or data, the facility still operates, and performs "
                  "   whatever part of its purpose remains meaningful, or\n"
                  ""
                  "   b) under the GNU GPL, with none of the additional permissions of "
                  "   this License applicable to that copy.\n"
                  ""
                  "3. Object Code Incorporating Material from Library Header Files.\n"
                  ""
                  "The object code form of an Application may incorporate material from "
                  "a header file that is part of the Library.  You may convey such object "
                  "code under terms of your choice, provided that, if the incorporated "
                  "material is not limited to numerical parameters, data structure "
                  "layouts and accessors, or small macros, inline functions and templates "
                  "(ten or fewer lines in length), you do both of the following:\n"
                  ""
                  "   a) Give prominent notice with each copy of the object code that the "
                  "   Library is used in it and that the Library and its use are "
                  "   covered by this License.\n"
                  ""
                  "   b) Accompany the object code with a copy of the GNU GPL and this license "
                  "   document.\n"
                  ""
                  "  4. Combined Works.\n"
                  ""
                  "You may convey a Combined Work under terms of your choice that, "
                  "taken together, effectively do not restrict modification of the "
                  "portions of the Library contained in the Combined Work and reverse "
                  "engineering for debugging such modifications, if you also do each of "
                  "the following:\n"
                  ""
                  "   a) Give prominent notice with each copy of the Combined Work that "
                  "   the Library is used in it and that the Library and its use are "
                  "   covered by this License.\n"
                  ""
                  "   b) Accompany the Combined Work with a copy of the GNU GPL and this license "
                  "   document.\n"
                  ""
                  "   c) For a Combined Work that displays copyright notices during "
                  "   execution, include the copyright notice for the Library among "
                  "   these notices, as well as a reference directing the user to the "
                  "   copies of the GNU GPL and this license document.\n"
                  ""
                  "   d) Do one of the following:\n"
                  ""
                  "       0) Convey the Minimal Corresponding Source under the terms of this "
                  "       License, and the Corresponding Application Code in a form "
                  "       suitable for, and under terms that permit, the user to "
                  "       recombine or relink the Application with a modified version of "
                  "       the Linked Version to produce a modified Combined Work, in the "
                  "       manner specified by section 6 of the GNU GPL for conveying "
                  "       Corresponding Source.\n"
                  ""
                  "       1) Use a suitable shared library mechanism for linking with the "
                  "       Library.  A suitable mechanism is one that (a) uses at run time "
                  "       a copy of the Library already present on the user's computer "
                  "       system, and (b) will operate properly with a modified version "
                  "       of the Library that is interface-compatible with the Linked "
                  "       Version.\n"
                  ""
                  "   e) Provide Installation Information, but only if you would otherwise "
                  "   be required to provide such information under section 6 of the "
                  "   GNU GPL, and only to the extent that such information is "
                  "   necessary to install and execute a modified version of the "
                  "   Combined Work produced by recombining or relinking the "
                  "   Application with a modified version of the Linked Version. (If "
                  "   you use option 4d0, the Installation Information must accompany "
                  "   the Minimal Corresponding Source and Corresponding Application "
                  "   Code. If you use option 4d1, you must provide the Installation "
                  "   Information in the manner specified by section 6 of the GNU GPL "
                  "   for conveying Corresponding Source.)\n"
                  ""
                  "5. Combined Libraries.\n"
                  ""
                  "You may place library facilities that are a work based on the "
                  "Library side by side in a single library together with other library "
                  "facilities that are not Applications and are not covered by this "
                  "License, and convey such a combined library under terms of your "
                  "choice, if you do both of the following:\n"
                  ""
                  "   a) Accompany the combined library with a copy of the same work based "
                  "   on the Library, uncombined with any other library facilities, "
                  "   conveyed under the terms of this License.\n"
                  ""
                  "   b) Give prominent notice with the combined library that part of it "
                  "   is a work based on the Library, and explaining where to find the "
                  "   accompanying uncombined form of the same work.\n"
                  ""
                  "6. Revised Versions of the GNU Lesser General Public License.\n"
                  ""
                  "The Free Software Foundation may publish revised and/or new versions "
                  "of the GNU Lesser General Public License from time to time. Such new "
                  "versions will be similar in spirit to the present version, but may "
                  "differ in detail to address new problems or concerns.\n"
                  ""
                  "Each version is given a distinguishing version number. If the "
                  "Library as you received it specifies that a certain numbered version "
                  "of the GNU Lesser General Public License 'or any later version' "
                  "applies to it, you have the option of following the terms and "
                  "conditions either of that published version or of any later version "
                  "published by the Free Software Foundation. If the Library as you "
                  "received it does not specify a version number of the GNU Lesser "
                  "General Public License, you may choose any version of the GNU Lesser "
                  "General Public License ever published by the Free Software Foundation.\n"
                  ""
                  "If the Library as you received it specifies that a proxy can decide "
                  "whether future versions of the GNU Lesser General Public License shall "
                  "apply, that proxy's public statement of acceptance of any version is "
                  "permanent authorization for you to choose that version for the "
                  "Library.";
    msgBox.setWindowTitle("GNU Lesser General Public License v3.0");
    msgBox.setText("OpenQVision follows the GNU LGPL v3.0");
    msgBox.setDetailedText(msg);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    int ret = msgBox.exec();
}
