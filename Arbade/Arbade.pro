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
    Sources/backend.h \
    Sources/bt_chapar.h \
    Sources/bt_mbr_base.h \
    Sources/bt_recorder.h \
    Sources/bt_wav_writer.h \
    Sources/config.h \
    Sources/ta_ini.h

SOURCES += \
    Sources/backend.cpp \
    Sources/bt_chapar.cpp \
    Sources/bt_mbr_base.cpp \
    Sources/bt_recorder.cpp \
    Sources/bt_wav_writer.cpp \
    Sources/main.cpp \
    Sources/ta_ini.cpp

win32:HEADERS += Sources/bt_lua.h

win32:SOURCES += Sources/bt_lua.cpp
