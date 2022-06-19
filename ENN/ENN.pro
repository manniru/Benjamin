TEMPLATE = app

CONFIG += console

linux:INCLUDEPATH += ./Sources

#linux:LIBS += -lgio-2.0

DEFINES += DNN_USE_IMAGE_API \
           CNN_USE_SSE \
           CNN_USE_AVX \
           NDEBUG
#           HAVE_CXXABI_H

QMAKE_CXXFLAGS += -std=gnu++14 -m64 -mavx -msse3 -g -isystem 03
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3

QMAKE_LFLAGS_RELEASE -= -O1

MOC_DIR = Build/.moc
RCC_DIR = Build/.rcc
OBJECTS_DIR = Build/.obj

HEADERS += \
    Sources/backend.h \
    Sources/config.h \
    Sources/enn_chapar.h \
    Sources/enn_dataset.h \
    Sources/enn_network.h \
    Sources/enn_test.h

SOURCES += \
    Sources/backend.cpp \
    Sources/enn_chapar.cpp \
    Sources/enn_dataset.cpp \
    Sources/enn_network.cpp \
    Sources/enn_test.cpp \
    Sources/main.cpp

