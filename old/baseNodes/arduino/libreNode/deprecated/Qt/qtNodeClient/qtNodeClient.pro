#-------------------------------------------------
#
# Project created by QtCreator 2016-11-27T19:20:48
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qtNodeClient
TEMPLATE = app

INCLUDEPATH += ../../Firmware/libreNode/

SOURCES += main.cpp\
        mainwindow.cpp \
    qtnodeclient.cpp

HEADERS  += mainwindow.h \
    ../../Firmware/libreNode/nodeclient.h \
    ../../Firmware/libreNode/qtcompat.h \
    qtnodeclient.h

FORMS    += mainwindow.ui

CONFIG += mobility network
MOBILITY = 

