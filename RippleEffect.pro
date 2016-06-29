#-------------------------------------------------
#
# Project created by QtCreator 2016-06-26T13:15:28
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RippleEffect
TEMPLATE = app


SOURCES +=\
    RippleEffect.cpp \
    GLWidget.cpp \
    Window.cpp \
    Main.cpp

HEADERS  += \
    RippleEffect.h \
    GLWidget.h \
    Window.h \
    RippleTable.h

FORMS    += \
    Window.ui

DISTFILES +=

RESOURCES += \
    Resources.qrc
