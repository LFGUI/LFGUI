#-------------------------------------------------
#
# Project created by QtCreator 2016-01-05T05:23:18
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = example
TEMPLATE = app

CONFIG += C++11

INCLUDEPATH +=  ../external/

DESTDIR = $${PWD}

SOURCES += main.cpp\
        ../lfgui/lfgui.cpp \
        ../lfgui/image.cpp

HEADERS  += \
        ../lfgui/font.h \
        ../lfgui/key.h \
        ../lfgui/signal.h \
        ../lfgui/lfgui.h \
        ../lfgui/image.h \
        ../lfgui/slider.h \
        ../lfgui/button.h \
        ../lfgui/lfgui_wrapper_qt.h \
        ../lfgui/general.h \
        ../lfgui/label.h \
        ../lfgui/lineedit.h \
        ../external/stk_timer.h \
        ../external/cimg/CImg.h \
        ../external/stb_truetype.h \
        ../common_sample_code.h \
        example.h
