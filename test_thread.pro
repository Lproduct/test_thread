#-------------------------------------------------
#
# Project created by QtCreator 2015-08-20T11:54:38
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = test_thread
TEMPLATE = app


SOURCES += main.cpp\
    mainwindow.cpp \
    mythread.cpp \
    threadic.cpp \
    Object.cpp \
    calibrationwindow.cpp \
    displaycalibwin.cpp \
    config.cpp \
    tools.cpp

HEADERS  += mainwindow.h \
    mythread.h \
    threadic.h \
    Object.h \
    calibrationwindow.h \
    displaycalibwin.h \
    config.h \
    tools.h

FORMS    += mainwindow.ui \
    calibrationwindow.ui \
    displaycalibwin.ui \
    tools.ui

INCLUDEPATH += C:\\mvIMPACT_Acquire

LIBS += -LC:\\mvIMPACT_Acquire\\lib \
-lmvDeviceManager \
-lmvDisplay

INCLUDEPATH += C:\\Users\\User\\Lvil\\Work\\AIEA_Camera\\opencv\\build\\include

LIBS += -LC:\\Users\\User\\Lvil\\Work\\AIEA_Camera\\mybuild2\\lib\\Debug \
-lopencv_calib3d2411d \
-lopencv_contrib2411d \
-lopencv_core2411d \
-lopencv_features2d2411d \
-lopencv_flann2411d \
-lopencv_gpu2411d \
-lopencv_haartraining_engined \
-lopencv_highgui2411d \
-lopencv_imgproc2411d \
-lopencv_legacy2411d \
-lopencv_ml2411d \
-lopencv_nonfree2411d \
-lopencv_objdetect2411d \
-lopencv_ocl2411d \
-lopencv_photo2411d \
-lopencv_stitching2411d \
-lopencv_superres2411d \
-lopencv_ts2411d \
-lopencv_video2411d \
-lopencv_videostab2411d

RESOURCES += \
    image.qrc
