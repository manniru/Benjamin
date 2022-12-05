TEMPLATE = app

QT += multimedia

CONFIG += console

INCLUDEPATH += ../PNN
win32:INCLUDEPATH += ../PNN/lua

linux:LIBS += -pthread -lm -ldl \
              -LKaldi/Libs -lportaudio -lasound -lrt -ljack -lfst
win32:LIBS += -L../PNN/libs -lFstWin64 -lPortAudio -lwinmm -llua54

DEFINES += HAVE_MKL \
           HAVE_CXXABI_H \
           CNN_USE_SSE \
           NDEBUG

QMAKE_CXXFLAGS += -std=c++14 -m64 -msse3 -pthread -mavx
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3

QMAKE_LFLAGS_RELEASE -= -O1

MOC_DIR = Build/.moc
RCC_DIR = Build/.rcc
OBJECTS_DIR = Build/.obj

OTHER_FILES = BaTool.conf

HEADERS += \
    Sources/ab_chapar.h \
    Sources/ab_mbr_base.h \
    Sources/ab_recorder.h \
    Sources/ab_wav_writer.h \
    Sources/backend.h \
    Sources/config.h \
    Sources/ta_ini.h

SOURCES += \
    Sources/ab_chapar.cpp \
    Sources/ab_mbr_base.cpp \
    Sources/ab_recorder.cpp \
    Sources/ab_wav_writer.cpp \
    Sources/backend.cpp \
    Sources/main.cpp \
    Sources/ta_ini.cpp

win32:HEADERS +=

win32:SOURCES +=

HEADERS += \
    Sources/ab_lua.h

SOURCES += \
    Sources/ab_lua.cpp
