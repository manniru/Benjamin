TEMPLATE = app

QT += dbus

CONFIG += console

SOURCES += Sources/main.cpp \
           Sources/backend.cpp \
           Sources/bt_chapar.cpp \
           Sources/bt_state.cpp

linux:SOURCES += \
                 Sources/bt_channel_l.cpp

HEADERS += Sources/backend.h \
           Sources/bt_config.h \
           Sources/bt_chapar.h \
           Sources/bt_state.h


linux:HEADERS += \
                 Sources/bt_channel_l.h


linux:INCLUDEPATH += /usr/include/glib-2.0 \
                     /usr/lib/glib-2.0/include

linux:LIBS += -lgio-2.0 \
              -lgobject-2.0 \
              -lglib-2.0


MOC_DIR = Build/.moc
RCC_DIR = Build/.rcc
OBJECTS_DIR = Build/.obj

