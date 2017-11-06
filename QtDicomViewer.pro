#-------------------------------------------------
#
# Project created by QtCreator 2017-09-14T16:10:16
#
#-------------------------------------------------

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QtDicomViewer
TEMPLATE = app


SOURCES += main.cpp \
           sources\qt_dicom_viewer.cpp \
           sources\image_viewer.cpp \

HEADERS += sources\qt_dicom_viewer.h \
           sources\image_viewer.h \

FORMS += QtDicomViewer.ui

INCLUDEPATH += sources \
               sources\widgets\algorithms \
               sources\widgets \
               C:\Tools\GDCM-2.8\include\gdcm-2.8 \
               C:\OpenCV\opencv-3.2.0-install-with-cuda\include

# OpenCV libs
win32 {
OPENCV_LIBS_PATH = C:\OpenCV\opencv-3.2.0-install-with-cuda\x64\vc14\lib

    CONFIG(debug, debug|release) {
    LIBS += -L$$OPENCV_LIBS_PATH \
                -lopencv_world320d
    }

    CONFIG(release, debug|release) {
    LIBS += -L$$OPENCV_LIBS_PATH \
                -opencv_world320
    }
}

# GDCM libs
win32 {
GDCM_LIBS_PATH = C:\Tools\GDCM-2.8\lib

    LIBS += -L$$GDCM_LIBS_PATH \
            -lgdcmcharls \
            -lgdcmCommon \
            -lgdcmDICT \
            -lgdcmDSED \
            -lgdcmexpat \
            -lgdcmgetopt \
            -lgdcmIOD \
            -lgdcmjpeg12 \
            -lgdcmjpeg8 \
            -lgdcmjpeg16 \
            -lgdcmmd5 \
            -lgdcmMEXD \
            -lgdcmMSFF \
            -lgdcmopenjp2 \
            -lgdcmzlib \
            -lsocketxx
}
