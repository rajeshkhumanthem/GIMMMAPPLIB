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
           gimmmmessage.cpp \
           unittests/gimmmapplibtest.cpp

HEADERS += gimmmconnection.h    \
           gimmmmessage.h            \
           exponentialbackoff.h \
           unittests/gimmmapplibtest.h

