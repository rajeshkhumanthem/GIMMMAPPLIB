#-------------------------------------------------
#
# Project created by QtCreator 2017-05-30T23:18:02
#
#-------------------------------------------------

QT += network
QT += core
QT += testlib

TARGET = gimmmlibtest

SOURCES += gimmmconnection.cpp  \
           message.cpp \
           unittests/gimmmlibtest.cpp

HEADERS += gimmmconnection.h    \
           message.h            \
           exponentialbackoff.h \
           unittests/gimmmlibtest.h

