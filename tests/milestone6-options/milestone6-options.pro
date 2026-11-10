QT = core testlib
CONFIG += console testcase c++17
CONFIG -= app_bundle
TEMPLATE = app
TARGET = tst_milestone6options
INCLUDEPATH += ../../app
SOURCES += tst_milestone6options.cpp ../../app/backend/apollocapabilities.cpp ../../app/backend/hostadministration.cpp ../../app/backend/nvapp.cpp
