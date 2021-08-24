# OpenQVision - v.1.0.0

OpenQVision is a computer vision project that allows developers and/or data scientists to rapidly test their newborn neural networks on camera streaming. As the name itself refers to, it is based on OpenCV (https://github.com/opencv/opencv) and Qt framework (https://github.com/qt) and handles Tensorflow (https://github.com/tensorflow/tensorflow) and Yolo Neural Networks (https://github.com/pjreddie/darknet).

## Installation

### Prerequisites
- An up to date OpenCV
- LibCurl
- An up to date Qt Creator
- Telegram account and a bot (not mandatory)

#### Clone the project

```sh
$ git clone https://github.com/dev-gazer/OpenQVision.git
```

#### Go inside the project folder and launch Qt Creator.
```sh
$ cd OpenQVision
$ qtcreator .
```

#### Build and run the project using qmake.

## How to use it

After building and running OpenQVision for the first time, you should see the following window:

![alt text](https://github.com/dev-gazer/OpenQVision/blob/main/OpenQVision-MainWindow.png)

For a quick sanity check, click on Start Camera. If it opens, it is all set.

Then click on Stop Camera.

Afterwards, just browse your neural network files path (weights, config and classes) to the main window. As you press Start Camera again, OpenQVision will identify if the user is loading a Tensorflow or Yolo model and camera streaming starts coupled with the detection model.

#### Quick note on Telegram and conditional actions

Feel free to use OpenQVision with your own modules. This version conditions the object detection to trigger a Telegram Bot. If you want to use it, go to OpenQVision/src/curl_telegram.cpp and to OpenQVision/src/videocaptured.cpp and set your own Bot Token and Telegram User Id. Any problems with creating a Telegram Bot, please refer to official documentation at https://core.telegram.org/bots/api.

If you are using Yolo weights and config, the object detection will trigger the chatbot after counting a certain amount of frames with an object from your model classes. The last frame is captured in temp directory and sent with the caption "We found your <name-of-the-object>!". Something like follows:

![alt text](https://github.com/dev-gazer/OpenQVision/blob/main/telegram-bot-example.png)


## Developer
* **Alexandre Martuscelli Faria**

    **Contact:** martuscellifaria@gmail.com

## Special thanks to
* **Github C++ community for providing a way of performing get and post requests so easily in C++.

* **Tom Ha** (https://github.com/tomha85) for providing such great lecture on how to read properly Yolo Darknet in C++ OpenCV.

* **Nico Nielsen** (https://github.com/niconielsen32) for also providing great content on Computer Vision in C++ and Python. This encouraged me a lot on porting OpenQVision from Python to C++.

## License

GNU LESSER GENERAL PUBLIC LICENSE
Version 3, 29 June 2007

In any case, please refer to the LICENSE file in this folder.