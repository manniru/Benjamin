TEMPLATE = app

QT += multimedia \
      quick

CONFIG += console

INCLUDEPATH += ../PNN

linux:LIBS += -pthread -lm -ldl \
              -LKaldi/Libs -lportaudio -lasound -lrt -ljack
win32:LIBS += -L../PNN/libs -lPortAudio -lwinmm \
              -lole32 -luuid

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
QMLCACHE_DIR = ./Build

OTHER_FILES = BaTool.conf

HEADERS += \
    Sources/ab_cache.h \
    Sources/ab_audio.h \
    Sources/ab_console.h \
    Sources/ab_console_handle.h \
    Sources/ab_editor.h \
    Sources/ab_init_wsl.h \
    Sources/ab_ler_stat.h \
    Sources/ab_mbr_base.h \
    Sources/ab_phoneme.h \
    Sources/ab_recorder.h \
    Sources/ab_scene.h \
    Sources/ab_stat.h \
    Sources/ab_telegram.h \
    Sources/ab_train.h \
    Sources/ab_verify.h \
    Sources/ab_wav_reader.h \
    Sources/ab_wav_writer.h \
    Sources/ab_win_api.h \
    Sources/backend.h \
    Sources/config.h \
    Sources/ta_ini.h

SOURCES += \
    Sources/ab_cache.cpp \
    Sources/ab_audio.cpp \
    Sources/ab_console.cpp \
    Sources/ab_console_handle.cpp \
    Sources/ab_editor.cpp \
    Sources/ab_init_wsl.cpp \
    Sources/ab_ler_stat.cpp \
    Sources/ab_mbr_base.cpp \
    Sources/ab_phoneme.cpp \
    Sources/ab_recorder.cpp \
    Sources/ab_scene.cpp \
    Sources/ab_stat.cpp \
    Sources/ab_telegram.cpp \
    Sources/ab_train.cpp \
    Sources/ab_verify.cpp \
    Sources/ab_wav_reader.cpp \
    Sources/ab_wav_writer.cpp \
    Sources/ab_win_api.cpp \
    Sources/backend.cpp \
    Sources/main.cpp \
    Sources/ta_ini.cpp

win32:HEADERS +=

win32:SOURCES += ../PNN/aj_dllgen.cpp

HEADERS +=

SOURCES +=

RESOURCES += \
             Qml/ui.qrc \
             Resources/fonts.qrc

OTHER_FILES += Qml/*.qml \
               Qml/Queries/*.qml

QML_IMPORT_PATH += Qml/
win32:RC_ICONS += icon.ico
