TEMPLATE = app

QT += qml quick widgets dbus

SOURCES += \
    Sources/backend.cpp \
    Sources/channel.cpp \
    Sources/main.cpp

RESOURCES += Qml/ui.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

DISTFILES += \
    Qml/ChCell.qml \
    Qml/notification.qml \
    Qml/main.qml

INCLUDEPATH +=  \
    /usr/include/glib-2.0 \
    /usr/lib/glib-2.0/include

LIBS += -lnotify \
    -lgdk_pixbuf-2.0 \
    -lgio-2.0 \
    -lgobject-2.0 \
    -lglib-2.0

QMAKE_CXXFLAGS += -pthread

HEADERS += \
    Sources/backend.h \
    Sources/channel.h \
    Sources/config.h

MOC_DIR = Build/.moc
RCC_DIR = Build/.rcc
OBJECTS_DIR = Build/.obj
UI_DIR = Build/.ui

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT
