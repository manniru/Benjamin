TEMPLATE = app

QT += dbus
QT += multimedia

CONFIG += console

SOURCES += Sources/*.cpp
HEADERS += Sources/*.h


linux:INCLUDEPATH += /usr/include/glib-2.0 \
                     /usr/lib/glib-2.0/include \
                     /usr/include/gstreamer-1.0 \
                     ../../Kaldi/src \
                     ../../Kaldi/tools/openfst/src/include \
                     ../../Kaldi/tools/portaudio/install/include \
                     /opt/intel/mkl/include

linux:LIBS += -lgio-2.0 \
              -lgobject-2.0 \
              -lglib-2.0 \
              -pthread \
              -lgstreamer-1.0 \
              -L/opt/intel/mkl/lib/intel64 \
              -lmkl_intel_lp64 -lmkl_sequential -lmkl_core \
              -liomp5 -lpthread -lm -ldl \
              -LKaldi/Libs \
              -lkaldi-online2 -lkaldi-online \
              -lkaldi-decoder -lkaldi-lat -lkaldi-fstext \
              -lkaldi-hmm -lkaldi-feat -lkaldi-transform \
              -lkaldi-gmm  -lkaldi-tree -lkaldi-util \
              -lkaldi-matrix -lkaldi-base -lportaudio -lasound -lrt -ljack -lfst

DEFINES += HAVE_MKL \
           HAVE_CXXABI_H

QMAKE_CXXFLAGS += -std=c++14 -m64 -msse -msse2 -pthread -g -isystem

MOC_DIR = Build/.moc
RCC_DIR = Build/.rcc
OBJECTS_DIR = Build/.obj

