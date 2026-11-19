QT += widgets
CONFIG += c++17
CONFIG -= app_bundle

TARGET = ColorSwitcher
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    colorwidget.cpp \
    palettebutton.cpp

HEADERS += \
    mainwindow.h \
    colorwidget.h \
    palettebutton.h