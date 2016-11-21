QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = example
TEMPLATE = app

CONFIG += C++11

QMAKE_CXXFLAGS += -O2# -mavx

INCLUDEPATH +=  ../external/

DESTDIR = $${PWD}

#DEFINES += LFGUI_SEPARATE_COLOR_CHANNELS

SOURCES += main.cpp\
        ../lfgui/lfgui.cpp \
        ../lfgui/image.cpp \
        ../lfgui/font.cpp \
        ../lfgui/window.cpp \
        ../lfgui/lineedit.cpp \
        ../lfgui/slider.cpp \
        ../common_sample_code.cpp \

HEADERS  += \
        ../lfgui/font.h \
        ../lfgui/key.h \
        ../lfgui/signal.h \
        ../lfgui/lfgui.h \
        ../lfgui/image.h \
        ../lfgui/slider.h \
        ../lfgui/button.h \
        ../lfgui/checkbox.h \
        ../lfgui/radio.h \
        ../lfgui/lfgui_wrapper_qt.h \
        ../lfgui/general.h \
        ../lfgui/label.h \
        ../lfgui/lineedit.h \
        ../lfgui/window.h \
        ../external/stk_timer.h \
        ../external/stb_truetype.h \
        ../common_sample_code.h \
        example.h
