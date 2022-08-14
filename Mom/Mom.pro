QT += quick
QT += sql
QT += widgets
QT += core
QT += network

RCC_DIR = Build/
OBJECTS_DIR = Build/
MOC_DIR = Build/
QMLCACHE_DIR = Build/
DEFINES += "WINVER=0x0601"

CONFIG += c++11

SOURCES += Sources/main.cpp \
           Sources/mm_bar.cpp \
           Sources/mm_chapar.cpp \
           Sources/mm_keyboard.cpp \
           Sources/mm_label.cpp \
           Sources/mm_monitor.cpp \
           Sources/mm_parser.cpp \
           Sources/mm_virt.cpp \
           Sources/mm_watcher.cpp

win32:LIBS += -lKernel32 -lUser32 -lole32 \
             -luuid -loleaut32 -loleacc \
             -lDwmapi -lPsapi -lSetupapi

RESOURCES += \
             Resources/fonts.qrc \
             Qml/ui.qrc

#OTHER_FILES += Qml/*.qml

# Additional import path used to resolve QML modules in Qt Creator's code model
#QML_IMPORT_PATH += Qml/

HEADERS += Sources/mm_config.h \
           Sources/mm_bar.h \
           Sources/mm_chapar.h \
           Sources/mm_keyboard.h \
           Sources/mm_label.h \
           Sources/mm_monitor.h \
           Sources/mm_parser.h \
           Sources/mm_virt.h \
           Sources/mm_watcher.h
