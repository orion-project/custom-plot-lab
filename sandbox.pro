QT += core gui widgets

include("custom-plot-lab.pri")

# orion (https://github.com/orion-project/orion-qt)
include($$_PRO_FILE_PWD_/orion/orion.pri)

CONFIG += c++11

SOURCES += \
    main.cpp \
    Sandbox.cpp

HEADERS += \
    Sandbox.h
