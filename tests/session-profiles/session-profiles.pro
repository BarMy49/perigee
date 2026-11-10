QT = core gui qml testlib
CONFIG += console testcase c++17
CONFIG -= app_bundle
TEMPLATE = app
TARGET = tst_sessionprofiles
INCLUDEPATH += ../../app
SOURCES += tst_sessionprofiles.cpp ../../app/settings/streamingpreferences.cpp
HEADERS += ../../app/settings/streamingpreferences.h
