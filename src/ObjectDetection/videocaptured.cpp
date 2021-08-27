#include "videocaptured.h"
#include <QDebug>
#include "ui_mainwindow.h"
#include "MainWindow/mainwindow.h"
#include <iostream>
#include <fstream>
#include <opencv2/videoio.hpp>
#include <stdio.h>
#include <cstdio>
#include <numeric>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/dnn/all_layers.hpp>
#include "ObjectDetection/dataStructures.h"
#include <Telegram/curl_telegram.h>
#include <string>
#include <QDir>

using namespace std;
using namespace dnn;

VideoCaptured::VideoCaptured(QObject *parent, QString classesPath, QString configPath, QString weightsPath, QString mediaPath, QString telUserId, QString telBotToken):
    QThread {parent},
    classesPath {classesPath},
    configPath {configPath},
    weightsPath {weightsPath},
    mediaPath {mediaPath},
    telUserId {telUserId},
    telBotToken {telBotToken}
{
}

void VideoCaptured::run(){
    string classesFile = classesPath.toStdString();
    string modelConfig = configPath.toStdString();
    string modelWeights = weightsPath.toStdString();
    string mediaFile = mediaPath.toStdString();
    string basePath;
    string telegramUser = telUserId.toStdString();
    string telegramBotToken = telBotToken.toStdString();

    const size_t last_slash_idx = classesFile.rfind('/');
    if (std::string::npos != last_slash_idx)
    {
        basePath = classesFile.substr(0, last_slash_idx);
        basePath = basePath + "/";
    }

    if(mediaPath != ""){
        if (mediaFile.substr(mediaFile.find_last_of(".") + 1) == "png" ||
                mediaFile.substr(mediaFile.find_last_of(".") + 1) == "jpg" ||
                mediaFile.substr(mediaFile.find_last_of(".") + 1) == "jpeg"){
            //case it is an image
            Mat img = imread(mediaFile);

            if(classesFile.substr(classesFile.find_last_of(".") + 1) != "" &&
                    modelConfig.substr(modelConfig.find_last_of(".") + 1) == "cfg" &&
                    modelWeights.substr(modelWeights.find_last_of(".") + 1) == "weights"){
                //case yolo model for image

                vector<string> classes;
                ifstream ifs(classesFile.c_str());
                string line;
                while (getline(ifs,line)) classes.push_back(line);
                dnn::Net net = dnn::readNetFromDarknet(modelConfig, modelWeights);
                net.setPreferableBackend(dnn::DNN_BACKEND_OPENCV);
                net.setPreferableTarget(dnn::DNN_TARGET_CPU);

                cv::Mat blob;
                double scaleFactor = 1.0/255.0;

                cv::Size size = cv::Size(416,416);
                cv::Scalar mean = cv::Scalar(0,0,0);
                bool swapRB = false;
                bool crop = false;
                cv::dnn::blobFromImage(img, blob, scaleFactor, size, mean, swapRB, crop);

                vector<cv::String> names;
                vector<int> outLayers = net.getUnconnectedOutLayers();
                vector<cv::String> layerNames = net.getLayerNames();

                names.resize(outLayers.size());
                for(size_t i = 0; i<outLayers.size(); ++i){
                    names[i] = layerNames[outLayers[i] - 1];
                }
                vector<cv::Mat> netOutput;
                net.setInput(blob);
                net.forward(netOutput,names);

                float confThreshold = 0.8;
                vector<int> classIds;
                vector<float> confidences;
                vector<cv::Rect> boxes;

                for (size_t i = 0; i<netOutput.size(); ++i){
                    float* data = (float*)netOutput[i].data;
                    for(int j = 0; j<netOutput[i].rows; ++j, data+= netOutput[i].cols){
                        cv::Mat scores = netOutput[i].row(j).colRange(5, netOutput[i].cols);
                        cv::Point classId;
                        double confidence;

                        minMaxLoc(scores,0,&confidence,0,&classId);
                        if(confidence > confThreshold){
                            cv::Rect box; int cx, cy;
                            cx = (int)(data[0]*img.cols);
                            cy = (int)(data[1]*img.rows);
                            box.width = (int)(data[2]*img.cols);
                            box.height = (int)(data[3]*img.rows);
                            box.x = cx - box.width/2;
                            box.y = cy - box.height/2;

                            boxes.push_back(box);
                            classIds.push_back(classId.x);
                            confidences.push_back((float)confidence);
                        }
                    }
                }
                float nmsThreshold = 0.2;
                vector<int> indices;
                cv::dnn::NMSBoxes(boxes,confidences,confThreshold,nmsThreshold,indices);
                vector<boundingBox> bBoxes;
                for (auto it = indices.begin(); it != indices.end(); ++it){
                    boundingBox bBox;
                    bBox.roi = boxes[*it];
                    bBox.classID = classIds[*it];
                    bBox.confidence = confidences[*it];
                    bBox.boxID = (int)bBoxes.size();

                    bBoxes.push_back(bBox);
                }

                cv::Mat visImg = img.clone();
                for (auto it = bBoxes.begin(); it != bBoxes.end(); ++it){
                    int top, left, width, height;
                    top = (*it).roi.y;
                    left = (*it).roi.x;
                    width = (*it).roi.width;
                    height = (*it).roi.height;
                    rectangle(visImg,cv::Point(left,top),cv::Point(left+width,top+height),cv::Scalar(0,0,255),3);

                    string label = format("%.2f", (*it).confidence);
                    label = classes[((*it).classID)] + ":" + label;
                    int baseLine;
                    cv::Size labelSize = getTextSize(label,cv::FONT_HERSHEY_DUPLEX,0.5,1,&baseLine);
                    top = max(top, labelSize.height);
                    rectangle(visImg,cv::Point(left,top-round(1.5*labelSize.height)),cv::Point(left + round(1.5*labelSize.width),top),Scalar(0,0,255),cv::FILLED);
                    putText(visImg,label,cv::Point(left,top),cv::FONT_HERSHEY_DUPLEX,0.75,cv::Scalar(255,255,255),2);

                }
                mPixmap = cvMatToQPixmap(visImg);
                emit newPixmapCaptured();

        } else if (classesFile.substr(classesFile.find_last_of(".") + 1) != "" &&
                    modelConfig.substr(modelConfig.find_last_of(".") + 1) == "pbtxt" &&
                    modelWeights.substr(modelWeights.find_last_of(".") + 1) == "pb"){
                //case tensorflow model for image
                vector<string> class_names;
                ifstream ifs(string(classesFile).c_str());
                string line;
                while (getline(ifs, line))
                {
                    class_names.push_back(line);
                }
                string network = "TensorFlow";
                Net net = dnn::readNet(modelWeights, modelConfig, network);
                float min_confidence_score = 0.6;
                Mat blob = blobFromImage(img, 1.0/255, Size(416, 416), Scalar(127.5, 127.5, 127.5), true, true);
                net.setInput(blob);
                Mat output = net.forward();
                Mat results(output.size[2], output.size[3], CV_32F, output.ptr<float>());
                for (int i = 0; i < results.rows; i++){
                    int class_id = int(results.at<float>(i, 1));
                    float confidence = results.at<float>(i, 2);

                    if (confidence > min_confidence_score){
                        int bboxX = int(results.at<float>(i, 3) * img.cols);
                        int bboxY = int(results.at<float>(i, 4) * img.rows);
                        int bboxWidth = int(results.at<float>(i, 5) * img.cols - bboxX);
                        int bboxHeight = int(results.at<float>(i, 6) * img.rows - bboxY);
                        rectangle(img, Point(bboxX, bboxY), Point(bboxX + bboxWidth, bboxY + bboxHeight), Scalar(0,0,255), 2);
                        string class_name = class_names[class_id-1];
                        putText(img, class_name + " " + to_string(int(confidence*100)) + "%", Point(bboxX, bboxY - 10), cv::FONT_HERSHEY_DUPLEX,0.75,cv::Scalar(255,255,255),2);
                    }
                }

                mPixmap = cvMatToQPixmap(img);
                emit newPixmapCaptured();

            } else{
                //case no model for image
                mPixmap = cvMatToQPixmap(img);
                emit newPixmapCaptured();
        }
    }
        else{
            //case video
            VideoCapture mVideoCap(mediaFile);
            if(classesFile.substr(classesFile.find_last_of(".") + 1) != "" &&
                    modelConfig.substr(modelConfig.find_last_of(".") + 1) == "cfg" &&
                    modelWeights.substr(modelWeights.find_last_of(".") + 1) == "weights") {
                //case yolo for video
                vector<string> classes;
                ifstream ifs(classesFile.c_str());
                string line;
                while (getline(ifs,line)) classes.push_back(line);

                if(!mVideoCap.isOpened()){
                    cerr << "ERROR. Unable to open the video\n";
                }

                for(;;){
                    mVideoCap.read(mFrame);
                    cv::Mat img = mFrame;
                    dnn::Net net = dnn::readNetFromDarknet(modelConfig, modelWeights);
                    net.setPreferableBackend(dnn::DNN_BACKEND_OPENCV);
                    net.setPreferableTarget(dnn::DNN_TARGET_CPU);

                    cv::Mat blob;
                    double scaleFactor = 1.0/255.0;

                    cv::Size size = cv::Size(416,416);
                    cv::Scalar mean = cv::Scalar(0,0,0);
                    bool swapRB = false;
                    bool crop = false;
                    auto start = getTickCount();
                    cv::dnn::blobFromImage(img, blob, scaleFactor, size, mean, swapRB, crop);

                    vector<cv::String> names;
                    vector<int> outLayers = net.getUnconnectedOutLayers();
                    vector<cv::String> layerNames = net.getLayerNames();

                    names.resize(outLayers.size());
                    for(size_t i = 0; i<outLayers.size(); ++i){
                        names[i] = layerNames[outLayers[i] - 1];
                    }
                    vector<cv::Mat> netOutput;
                    net.setInput(blob);
                    net.forward(netOutput,names);
                    auto end = getTickCount();

                    float confThreshold = 0.8;
                    vector<int> classIds;
                    vector<float> confidences;
                    vector<cv::Rect> boxes;

                    for (size_t i = 0; i<netOutput.size(); ++i){
                        float* data = (float*)netOutput[i].data;
                        for(int j = 0; j<netOutput[i].rows; ++j, data+= netOutput[i].cols){
                            cv::Mat scores = netOutput[i].row(j).colRange(5, netOutput[i].cols);
                            cv::Point classId;
                            double confidence;

                            minMaxLoc(scores,0,&confidence,0,&classId);
                            if(confidence > confThreshold){
                                cv::Rect box; int cx, cy;
                                cx = (int)(data[0]*img.cols);
                                cy = (int)(data[1]*img.rows);
                                box.width = (int)(data[2]*img.cols);
                                box.height = (int)(data[3]*img.rows);
                                box.x = cx - box.width/2;
                                box.y = cy - box.height/2;

                                boxes.push_back(box);
                                classIds.push_back(classId.x);
                                confidences.push_back((float)confidence);
                            }
                        }
                    }
                    float nmsThreshold = 0.2;
                    vector<int> indices;
                    cv::dnn::NMSBoxes(boxes,confidences,confThreshold,nmsThreshold,indices);
                    vector<boundingBox> bBoxes;
                    for (auto it = indices.begin(); it != indices.end(); ++it){
                        boundingBox bBox;
                        bBox.roi = boxes[*it];
                        bBox.classID = classIds[*it];
                        bBox.confidence = confidences[*it];
                        bBox.boxID = (int)bBoxes.size();

                        bBoxes.push_back(bBox);
                    }

                    cv::Mat visImg = img.clone();
                    for (auto it = bBoxes.begin(); it != bBoxes.end(); ++it){
                        int top, left, width, height;
                        top = (*it).roi.y;
                        left = (*it).roi.x;
                        width = (*it).roi.width;
                        height = (*it).roi.height;
                        rectangle(visImg,cv::Point(left,top),cv::Point(left+width,top+height),cv::Scalar(0,0,255),3);

                        string label = format("%.2f", (*it).confidence);
                        label = classes[((*it).classID)] + ":" + label;
                        int baseLine;
                        cv::Size labelSize = getTextSize(label,cv::FONT_HERSHEY_DUPLEX,0.5,1,&baseLine);
                        top = max(top, labelSize.height);
                        rectangle(visImg,cv::Point(left,top-round(1.5*labelSize.height)),cv::Point(left + round(1.5*labelSize.width),top),Scalar(0,0,255),cv::FILLED);
                        putText(visImg,label,cv::Point(left,top),cv::FONT_HERSHEY_DUPLEX,0.75,cv::Scalar(255,255,255),2);

                    }

                    if(mFrame.empty()){
                        cerr << "ERROR! Blank frame grabbed.\n";
                    }

                    auto totalTime = (end - start) / getTickFrequency();


                    putText(visImg, "FPS: " + to_string(int(1 / totalTime)), Point(50, 50), cv::FONT_HERSHEY_DUPLEX, 1, Scalar(0, 255, 0), 2, false);
                    mPixmap = cvMatToQPixmap(visImg);
                    emit newPixmapCaptured();
                }
            } else if(classesFile.substr(classesFile.find_last_of(".") + 1) != "" &&
                      modelConfig.substr(modelConfig.find_last_of(".") + 1) == "pbtxt" &&
                      modelWeights.substr(modelWeights.find_last_of(".") + 1) == "pb"){
                //case tensorflow for video
                vector<string> class_names;
                ifstream ifs(string(classesFile).c_str());
                string line;

                while (getline(ifs, line))
                {
                    class_names.push_back(line);
                }

                string network = "TensorFlow";

                Net net = dnn::readNet(modelWeights, modelConfig, network);

                net.setPreferableBackend(DNN_BACKEND_OPENCV);
                net.setPreferableTarget(DNN_TARGET_CPU);

                float min_confidence_score = 0.6;

                while (mVideoCap.isOpened()) {

                    //Load in an image
                    mVideoCap.read(mFrame);
                    cv::Mat img = mFrame;

                    auto start = getTickCount();

                    //Create a blob from the image
                    Mat blob = blobFromImage(img, 1.0/255, Size(416, 416), Scalar(127.5, 127.5, 127.5), true, true);


                    //Set the blob to be input to the neural network
                    net.setInput(blob);

                    //Forward pass of the blob through the neural network to get the predictions
                    Mat output = net.forward();

                    auto end = getTickCount();

                    //Matrix with all the detections
                    Mat results(output.size[2], output.size[3], CV_32F, output.ptr<float>());

                    //Run through all the predictions
                    for (int i = 0; i < results.rows; i++){
                        int class_id = int(results.at<float>(i, 1));
                        float confidence = results.at<float>(i, 2);

                        //Check if the detection is over the min threshold and then draw bbox
                        if (confidence > min_confidence_score){
                            int bboxX = int(results.at<float>(i, 3) * img.cols);
                            int bboxY = int(results.at<float>(i, 4) * img.rows);
                            int bboxWidth = int(results.at<float>(i, 5) * img.cols - bboxX);
                            int bboxHeight = int(results.at<float>(i, 6) * img.rows - bboxY);
                            rectangle(img, Point(bboxX, bboxY), Point(bboxX + bboxWidth, bboxY + bboxHeight), Scalar(0,0,255), 2);
                            string class_name = class_names[class_id-1];
                            putText(img, class_name + " " + to_string(int(confidence*100)) + "%", Point(bboxX, bboxY - 10), cv::FONT_HERSHEY_DUPLEX,0.75,cv::Scalar(255,255,255),2);
                        }
                    }


                    auto totalTime = (end - start) / getTickFrequency();

                    putText(img, "FPS: " + to_string(int(1 / totalTime)), Point(50, 50), FONT_HERSHEY_DUPLEX, 1, Scalar(0, 255, 0), 2, false);
                    mPixmap = cvMatToQPixmap(img);
                    emit newPixmapCaptured();
                }

            } else {
                //case no model with video
                if (mVideoCap.isOpened()){
                    while(true){
                        auto start = getTickCount();
                        mVideoCap >> mFrame;
                        auto end = getTickCount();
                        auto totalTime = (end - start) / getTickFrequency();
                        if(!mFrame.empty()){
                            putText(mFrame, "FPS: " + to_string(int(1 / totalTime)), Point(50, 50), FONT_HERSHEY_DUPLEX, 1, Scalar(0, 255, 0), 2, false);
                            mPixmap = cvMatToQPixmap(mFrame);
                            emit newPixmapCaptured();
                        }
                    }
                }
            }
        }
    }
    else {
        //case camera
        VideoCapture mVideoCap(ID_CAMERA);
        if(classesFile.substr(classesFile.find_last_of(".") + 1) != "" &&
                modelConfig.substr(modelConfig.find_last_of(".") + 1) == "cfg" &&
                modelWeights.substr(modelWeights.find_last_of(".") + 1) == "weights") {
            //case yolo for camera
            vector<string> classes;
            ifstream ifs(classesFile.c_str());
            string line;
            while (getline(ifs,line)) classes.push_back(line);
            int countObj[classes.size()] = {};
            if(!mVideoCap.isOpened()){
                cerr << "ERROR. Unable to open the camera\n";
            }
            for(;;){
                mVideoCap.read(mFrame);
                cv::Mat img = mFrame;

                dnn::Net net = dnn::readNetFromDarknet(modelConfig, modelWeights);
                net.setPreferableBackend(dnn::DNN_BACKEND_OPENCV);
                net.setPreferableTarget(dnn::DNN_TARGET_CPU);

                cv::Mat blob;
                double scaleFactor = 1.0/255.0;

                cv::Size size = cv::Size(416,416);
                cv::Scalar mean = cv::Scalar(0,0,0);
                bool swapRB = false;
                bool crop = false;
                auto start = getTickCount();
                cv::dnn::blobFromImage(img, blob, scaleFactor, size, mean, swapRB, crop);

                vector<cv::String> names;
                vector<int> outLayers = net.getUnconnectedOutLayers();
                vector<cv::String> layerNames = net.getLayerNames();

                names.resize(outLayers.size());
                for(size_t i = 0; i<outLayers.size(); ++i){
                    names[i] = layerNames[outLayers[i] - 1];
                }
                vector<cv::Mat> netOutput;
                net.setInput(blob);
                net.forward(netOutput,names);
                auto end = getTickCount();

                float confThreshold = 0.8;
                vector<int> classIds;
                vector<float> confidences;
                vector<cv::Rect> boxes;

                for (size_t i = 0; i<netOutput.size(); ++i){
                    float* data = (float*)netOutput[i].data;
                    for(int j = 0; j<netOutput[i].rows; ++j, data+= netOutput[i].cols){
                        cv::Mat scores = netOutput[i].row(j).colRange(5, netOutput[i].cols);
                        cv::Point classId;
                        double confidence;

                        minMaxLoc(scores,0,&confidence,0,&classId);
                        if(confidence > confThreshold){
                            cv::Rect box; int cx, cy;
                            cx = (int)(data[0]*img.cols);
                            cy = (int)(data[1]*img.rows);
                            box.width = (int)(data[2]*img.cols);
                            box.height = (int)(data[3]*img.rows);
                            box.x = cx - box.width/2;
                            box.y = cy - box.height/2;

                            boxes.push_back(box);
                            classIds.push_back(classId.x);
                            confidences.push_back((float)confidence);
                        }
                    }
                }
                float nmsThreshold = 0.2;
                vector<int> indices;
                cv::dnn::NMSBoxes(boxes,confidences,confThreshold,nmsThreshold,indices);
                vector<boundingBox> bBoxes;
                for (auto it = indices.begin(); it != indices.end(); ++it){
                    boundingBox bBox;
                    bBox.roi = boxes[*it];
                    bBox.classID = classIds[*it];
                    bBox.confidence = confidences[*it];
                    bBox.boxID = (int)bBoxes.size();

                    bBoxes.push_back(bBox);
                }

                cv::Mat visImg = img.clone();
                for (auto it = bBoxes.begin(); it != bBoxes.end(); ++it){
                    int top, left, width, height;
                    top = (*it).roi.y;
                    left = (*it).roi.x;
                    width = (*it).roi.width;
                    height = (*it).roi.height;
                    rectangle(visImg,cv::Point(left,top),cv::Point(left+width,top+height),cv::Scalar(0,0,255),3);

                    string label = format("%.2f", (*it).confidence);
                    label = classes[((*it).classID)] + ":" + label;
                    int baseLine;
                    cv::Size labelSize = getTextSize(label,cv::FONT_HERSHEY_DUPLEX,0.5,1,&baseLine);
                    top = max(top, labelSize.height);
                    rectangle(visImg,cv::Point(left,top-round(1.5*labelSize.height)),cv::Point(left + round(1.5*labelSize.width),top),Scalar(0,0,255),cv::FILLED);
                    putText(visImg,label,cv::Point(left,top),cv::FONT_HERSHEY_DUPLEX,0.75,cv::Scalar(255,255,255),2);

                    if (telegramUser != "" && telegramBotToken != ""){
                        for(int i = 0; i < int (classes.size()); i++){
                            if(classes[((*it).classID)] == classes[i]){
                                countObj[i]++;
                                if (countObj[i] == 5){
                                    QDir dir(QString::fromStdString(basePath + "result/"));
                                    if (dir.exists()){
                                        cv::imwrite(basePath+"result/"+classes[i]+".png", visImg);
                                        sendTelegramPhoto(telegramUser, telegramBotToken, basePath+"result/"+classes[i]+".png", "We found your " + classes[i] +"!");
                                    } else{
                                        dir.mkdir(QString::fromStdString(basePath + "result/"));
                                        cv::imwrite(basePath+"result/"+classes[i]+".png", visImg);
                                        sendTelegramPhoto(telegramUser, telegramBotToken, basePath+"result/"+classes[i]+".png", "We found your " + classes[i] +"!");
                                    }

                                }
                            }
                        }
                    }
                }

                if(mFrame.empty()){
                    cerr << "ERROR! Blank frame grabbed.\n";
                }

                auto totalTime = (end - start) / getTickFrequency();


                putText(visImg, "FPS: " + to_string(int(1 / totalTime)), Point(50, 50), cv::FONT_HERSHEY_DUPLEX, 1, Scalar(0, 255, 0), 2, false);
                mPixmap = cvMatToQPixmap(visImg);
                emit newPixmapCaptured();
            }
        } else if(classesFile.substr(classesFile.find_last_of(".") + 1) != "" &&
              modelConfig.substr(modelConfig.find_last_of(".") + 1) == "pbtxt" &&
              modelWeights.substr(modelWeights.find_last_of(".") + 1) == "pb"){
        //case tensorflow for camera
        vector<string> class_names;
        ifstream ifs(string(classesFile).c_str());
        string line;

        while (getline(ifs, line))
        {
            class_names.push_back(line);
        }

        string network = "TensorFlow";

        Net net = dnn::readNet(modelWeights, modelConfig, network);

        net.setPreferableBackend(DNN_BACKEND_OPENCV);
        net.setPreferableTarget(DNN_TARGET_CPU);

        float min_confidence_score = 0.6;

        while (mVideoCap.isOpened()) {

            mVideoCap.read(mFrame);
            cv::Mat img = mFrame;

            auto start = getTickCount();

            Mat blob = blobFromImage(img, 1.0/255, Size(416, 416), Scalar(127.5, 127.5, 127.5), true, true);

            net.setInput(blob);

            Mat output = net.forward();

            auto end = getTickCount();

            Mat results(output.size[2], output.size[3], CV_32F, output.ptr<float>());

            for (int i = 0; i < results.rows; i++){
                int class_id = int(results.at<float>(i, 1));
                float confidence = results.at<float>(i, 2);

                if (confidence > min_confidence_score){
                    int bboxX = int(results.at<float>(i, 3) * img.cols);
                    int bboxY = int(results.at<float>(i, 4) * img.rows);
                    int bboxWidth = int(results.at<float>(i, 5) * img.cols - bboxX);
                    int bboxHeight = int(results.at<float>(i, 6) * img.rows - bboxY);
                    rectangle(img, Point(bboxX, bboxY), Point(bboxX + bboxWidth, bboxY + bboxHeight), Scalar(0,0,255), 2);
                    string class_name = class_names[class_id-1];
                    putText(img, class_name + " " + to_string(int(confidence*100)) + "%", Point(bboxX, bboxY - 10), cv::FONT_HERSHEY_DUPLEX,0.75,cv::Scalar(255,255,255),2);
                }
            }


            auto totalTime = (end - start) / getTickFrequency();

            putText(img, "FPS: " + to_string(int(1 / totalTime)), Point(50, 50), FONT_HERSHEY_DUPLEX, 1, Scalar(0, 255, 0), 2, false);
            mPixmap = cvMatToQPixmap(img);
            emit newPixmapCaptured();
        }

    } else {
        //case only camera and no model
        if (mVideoCap.isOpened()){
            while(true){
                auto start = getTickCount();
                mVideoCap >> mFrame;
                auto end = getTickCount();
                auto totalTime = (end - start) / getTickFrequency();
                if(!mFrame.empty()){
                    putText(mFrame, "FPS: " + to_string(int(1 / totalTime)), Point(50, 50), FONT_HERSHEY_DUPLEX, 1, Scalar(0, 255, 0), 2, false);
                    mPixmap = cvMatToQPixmap(mFrame);
                    emit newPixmapCaptured();
                }
            }
        }
    }
    }

}

void VideoCaptured::stop()
{
    if (mVideoCap.isOpened()){
    mVideoCap.release();
    }
}


QImage VideoCaptured::cvMatToQImage( const cv::Mat &inMat )
{
    switch ( inMat.type() )
    {
    // 8-bit, 4 channel
    case CV_8UC4:
    {
        QImage image( inMat.data,
                      inMat.cols, inMat.rows,
                      static_cast<int>(inMat.step),
                      QImage::Format_ARGB32 );

        return image;
    }

        // 8-bit, 3 channel
    case CV_8UC3:
    {
        QImage image( inMat.data,
                      inMat.cols, inMat.rows,
                      static_cast<int>(inMat.step),
                      QImage::Format_RGB888 );

        return image.rgbSwapped();
    }

        // 8-bit, 1 channel
    case CV_8UC1:
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
        QImage image( inMat.data,
                      inMat.cols, inMat.rows,
                      static_cast<int>(inMat.step),
                      QImage::Format_Grayscale8 );
#else
        static QVector<QRgb>  sColorTable;

        // only create our color table the first time
        if ( sColorTable.isEmpty() )
        {
            sColorTable.resize( 256 );

            for ( int i = 0; i < 256; ++i )
            {
                sColorTable[i] = qRgb( i, i, i );
            }
        }

        QImage image( inMat.data,
                      inMat.cols, inMat.rows,
                      static_cast<int>(inMat.step),
                      QImage::Format_Indexed8 );

        image.setColorTable( sColorTable );
#endif

        return image;
    }

    default:
        qWarning() << "ASM::cvMatToQImage() - cv::Mat image type not handled in switch:" << inMat.type();
        break;
    }

    return QImage();
}

QPixmap VideoCaptured::cvMatToQPixmap( const cv::Mat &inMat )
{
    return QPixmap::fromImage( cvMatToQImage( inMat ) );
}
