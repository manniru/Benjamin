TEMPLATE = app

QT += multimedia

CONFIG += console

linux:INCLUDEPATH += ./Sources

#linux:LIBS += -lgio-2.0

DEFINES += DNN_USE_IMAGE_API
#           HAVE_CXXABI_H

QMAKE_CXXFLAGS += -std=c++14 -m64 -msse -msse2 -pthread -g -isystem

MOC_DIR = Build/.moc
RCC_DIR = Build/.rcc
OBJECTS_DIR = Build/.obj

HEADERS += \
    Sources/backend.h \
    Sources/config.h \
    Sources/enn_chapar.h

SOURCES += \
    Sources/backend.cpp \
    Sources/enn_chapar.cpp \
    Sources/main.cpp

