QT += quick
QT += sql
QT += widgets
QT += core
QT += network

RCC_DIR = Build/
OBJECTS_DIR = Build/
MOC_DIR = Build/
QMLCACHE_DIR = Build/

CONFIG += c++11

SOURCES += Sources/main.cpp \
           Sources/bb_chapar.cpp \
           Sources/bb_bar.cpp


RESOURCES += \
             Resources/fonts.qrc \
             Qml/ui.qrc

#OTHER_FILES += Qml/*.qml

# Additional import path used to resolve QML modules in Qt Creator's code model
#QML_IMPORT_PATH += Qml/

HEADERS += Sources/bb_config.h \
           Sources/bb_chapar.h \
           Sources/bb_bar.h
