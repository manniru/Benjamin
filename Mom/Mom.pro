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
           Sources/mm_api.cpp \
           Sources/mm_app_launcher.cpp \
           Sources/mm_bar.cpp \
           Sources/mm_channel.cpp \
           Sources/mm_chapar.cpp \
           Sources/mm_downloader.cpp \
           Sources/mm_key_emulator.cpp \
           Sources/mm_key_executer.cpp \
           Sources/mm_keyboard.cpp \
           Sources/mm_label.cpp \
           Sources/mm_lua.cpp \
           Sources/mm_monitor.cpp \
           Sources/mm_music.cpp \
           Sources/mm_nt_user.cpp \
           Sources/mm_parser.cpp \
           Sources/mm_sound.cpp \
           Sources/mm_state.cpp \
           Sources/mm_usage.cpp \
           Sources/mm_virt.cpp \
           Sources/mm_watcher.cpp \
           Sources/mm_win32.cpp \
           Sources/mm_win32_win.cpp \
           Sources/mm_win_manager.cpp \
           Sources/ta_ini.cpp

win32:LIBS += -L../PNN/libs \
             -lKernel32 -lUser32 -lole32 \
             -luuid -loleaut32 -loleacc \
             -lDwmapi -lPsapi -lSetupapi \
             -lPowrProf -llua54 -lPdh

INCLUDEPATH += ../PNN/lua

RESOURCES += \
             Resources/fonts.qrc \
             Qml/ui.qrc

#OTHER_FILES += Qml/*.qml

# Additional import path used to resolve QML modules in Qt Creator's code model
#QML_IMPORT_PATH += Qml/

HEADERS += Sources/mm_config.h \
           Sources/mm_api.h \
           Sources/mm_app_launcher.h \
           Sources/mm_bar.h \
           Sources/mm_channel.h \
           Sources/mm_chapar.h \
           Sources/mm_downloader.h \
           Sources/mm_key_emulator.h \
           Sources/mm_key_executer.h \
           Sources/mm_keyboard.h \
           Sources/mm_label.h \
           Sources/mm_lua.h \
           Sources/mm_monitor.h \
           Sources/mm_music.h \
           Sources/mm_nt_user.h \
           Sources/mm_parser.h \
           Sources/mm_sound.h \
           Sources/mm_state.h \
           Sources/mm_usage.h \
           Sources/mm_virt.h \
           Sources/mm_watcher.h \
           Sources/mm_win32.h \
           Sources/mm_win32_const.h \
           Sources/mm_win32_win.h \
           Sources/mm_win_manager.h \
           Sources/ta_ini.h \
           Sources/win_policy.h
