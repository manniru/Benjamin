TEMPLATE = app

QT += qml quick widgets dbus

RESOURCES += Qml/ui.qrc

DISTFILES += \
    Qml/ChCell.qml \
    Qml/OpTimer.qml \
    Qml/main.qml

linux:INCLUDEPATH +=  \
    /usr/include/glib-2.0 \
    /usr/lib/glib-2.0/include

linux:LIBS += -lnotify \
    -lgdk_pixbuf-2.0 \
    -lgio-2.0 \
    -lgobject-2.0 \
    -lglib-2.0

win32:LIBS += -lUser32

SOURCES += \
    Sources/backend.cpp \
    Sources/ch_keyboard_w.cpp \
    Sources/main.cpp \
    Sources/ch_chapar.cpp

HEADERS += \
    Sources/backend.h \
    Sources/ch_keyboard_w.h \
    Sources/config.h \
    Sources/ch_chapar.h

linux:HEADERS += \
    Sources/ch_channel_l.h \
    Sources/ch_processor_l.h

win32:HEADERS += \
    Sources/ch_channel_w.h \
    Sources/ch_processor_w.h

linux:SOURCES += \
    Sources/ch_channel_l.cpp \
    Sources/ch_processor_l.cpp

win32:SOURCES += \
    Sources/ch_channel_w.cpp \
    Sources/ch_processor_w.cpp

MOC_DIR = Build/.moc
RCC_DIR = Build/.rcc
OBJECTS_DIR = Build/.obj
UI_DIR = Build/.ui

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT
