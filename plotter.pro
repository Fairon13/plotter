#-------------------------------------------------
#
# Project created by QtCreator 2014-11-01T08:09:52
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = plotter-1.4.0
TEMPLATE = app

#linux-*{
#    QMAKE_CFLAGS_RELEASE += -march=native -flto -O3
#    QMAKE_CXXFLAGS_RELEASE += -march=native -flto -O3
#}

#QMAKE_CFLAGS_RELEASE   -= -O2
#QMAKE_CXXFLAGS_RELEASE -= -O2

#QMAKE_CFLAGS_RELEASE   += -g -fno-inline
#QMAKE_CXXFLAGS_RELEASE += -g -fno-inline

UI_DIR = ./build
MOC_DIR = ./build
OBJECTS_DIR = ./build
RCC_DIR = ./build
DESTDIR = ./bin

INCLUDEPATH += ./src

LIBS += -lprotobuf

SOURCES += src/main.cpp\
           src/mainwindow.cpp \
    src/datacollectorclass.cpp \
    src/plotmanagerclass.cpp \
    src/dataviewclass.cpp \
    src/qcustomplot.cpp \
    src/plotviewclass.cpp \
    src/pipedata.cpp \
    src/pipethread.cpp \
    src/addingassetdialog.cpp \
    src/assetclass.cpp \
    src/pipesignals.cpp \
    src/setrangedialog.cpp \
    src/busadaptor.cpp \
    src/pipeasset.cpp \
    src/busasset.cpp \
    src/addingbusassetdialog.cpp \
    src/dataeditorclass.cpp \
    src/datacacheclass.cpp \
    src/plotviewwrapper.cpp

HEADERS  += src/mainwindow.h \
    src/datacollectorclass.h \
    src/plotmanagerclass.h \
    src/dataviewclass.h \
    src/qcustomplot.h \
    src/plotviewclass.h \
    src/pipedata.h \
    src/pipethread.h \
    src/AboutDialog.h \
    src/addingassetdialog.h \
    src/assetclass.h \
    src/pipesignals.h \
    src/setrangedialog.h \
    src/busadaptor.h \
    src/pipeasset.h \
    src/busasset.h \
    src/addingbusassetdialog.h \
    src/dataeditorclass.h \
    src/datacacheclass.h \
    src/plotviewwrapper.h

FORMS    += src/mainwindow.ui \
    src/AboutDialog.ui \
    src/addingassetdialog.ui \
    src/setrangedialog.ui \
    src/addingbusassetdialog.ui

TRANSLATIONS = translates/plotter_ru_RU.ts

RESOURCES += src/resources.qrc
