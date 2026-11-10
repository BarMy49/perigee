QT = core testlib
CONFIG += console testcase c++17
CONFIG -= app_bundle
TEMPLATE = app
TARGET = tst_displaylaunch
INCLUDEPATH += ../../app
SOURCES += tst_displaylaunch.cpp ../../app/backend/displaylaunchoptions.cpp ../../app/backend/apollocapabilities.cpp
HEADERS += ../../app/backend/displaylaunchoptions.h ../../app/backend/apollocapabilities.h
