#-------------------------------------------------
#
# Project created by QtCreator 2017-05-30T23:18:02
#
#-------------------------------------------------

QT += network
QT += core
QT += testlib

TARGET = gimmmapplibtest

SOURCES += gimmmconnection.cpp  \
           message.cpp \
           unittests/gimmmapplibtest.cpp

HEADERS += gimmmconnection.h    \
           message.h            \
           exponentialbackoff.h \
           unittests/gimmmapplibtest.h

