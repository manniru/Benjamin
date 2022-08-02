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
           Sources/mm_bar.cpp \
           Sources/mm_chapar.cpp \
           Sources/mm_watcher.cpp


RESOURCES += \
             Resources/fonts.qrc \
             Qml/ui.qrc

#OTHER_FILES += Qml/*.qml

# Additional import path used to resolve QML modules in Qt Creator's code model
#QML_IMPORT_PATH += Qml/

HEADERS += Sources/mm_config.h \
           Sources/mm_bar.h \
           Sources/mm_chapar.h \
           Sources/mm_watcher.h
