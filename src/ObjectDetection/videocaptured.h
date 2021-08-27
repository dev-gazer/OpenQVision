#ifndef VIDEOCAPTURE_H
#define VIDEOCAPTURE_H
#include <QPixmap>
#include <QImage>
#include <QThread>
#include "MainWindow/mainwindow.h"
#include <opencv4/opencv2/opencv.hpp>
#define ID_CAMERA 0
#include <QString>

class VideoCaptured : public QThread
{
    Q_OBJECT
public:
    VideoCaptured(QObject *parent = nullptr, QString classesPath = "",
                  QString configPath = "", QString weightsPath = "", QString mediaPath = "",
                  QString telUserId = "", QString telBotToken = "");
    QPixmap pixmap() const{
        return mPixmap;
    }
    QString classesPath;
    QString configPath;
    QString weightsPath;
    QString mediaPath;
    QString telUserId;
    QString telBotToken;
    void stop();
signals:
    void newPixmapCaptured();
protected:
    void run();
    QPixmap mPixmap;
    cv::Mat mFrame;
    cv::VideoCapture mVideoCap;
    QImage cvMatToQImage( const cv::Mat &inMat );
    QPixmap cvMatToQPixmap( const cv::Mat &inMat );
};

#endif // VIDEOCAPTURE_H
