QT = core testlib
CONFIG += console testcase c++17
CONFIG -= app_bundle
TEMPLATE = app
TARGET = tst_apollocapabilities
INCLUDEPATH += ../../app
SOURCES += tst_apollocapabilities.cpp ../../app/backend/apollocapabilities.cpp ../../app/backend/hostadministration.cpp
HEADERS += ../../app/backend/apollocapabilities.h ../../app/backend/hostadministration.h
