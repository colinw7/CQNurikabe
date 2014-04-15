TEMPLATE = app

TARGET = CQNurikabe

DEPENDPATH += .

#CONFIG += debug

# Input
SOURCES += \
CNurikabe.cpp \
CQNurikabe.cpp

HEADERS += \
CNurikabe.h \
CQNurikabe.h \
Puzzles.h \
help.xpm

DESTDIR     = ../bin
OBJECTS_DIR = ../obj
LIB_DIR     = ../lib

INCLUDEPATH += \
../include \
../../COS/include \
.

unix:LIBS += \
-L$$LIB_DIR \
