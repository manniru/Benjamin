TEMPLATE = app

QT += dbus
QT += multimedia

CONFIG += console

SOURCES += Sources/main.cpp \
           Sources/backend.cpp \
           Sources/bt_chapar.cpp \
           Sources/bt_confidence.cpp \
           Sources/bt_encoder.cpp \
           Sources/bt_online.cpp \
           Sources/bt_recorder.cpp \
           Sources/bt_state.cpp

linux:SOURCES += \
                 Sources/bt_channel_l.cpp

HEADERS += Sources/backend.h \
           Sources/bt_config.h \
           Sources/bt_chapar.h \
           Sources/bt_confidence.h \
           Sources/bt_encoder.h \
           Sources/bt_online.h \
           Sources/bt_recorder.h \
           Sources/bt_state.h


linux:HEADERS += \
                 Sources/bt_channel_l.h


linux:INCLUDEPATH += /usr/include/glib-2.0 \
                     /usr/lib/glib-2.0/include \
                     /usr/include/gstreamer-1.0

linux:LIBS += -lgio-2.0 \
              -lgobject-2.0 \
              -lglib-2.0 \
              -pthread \
              -lgstreamer-1.0


MOC_DIR = Build/.moc
RCC_DIR = Build/.rcc
OBJECTS_DIR = Build/.obj

