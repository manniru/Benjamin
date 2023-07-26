TEMPLATE = app

CONFIG += console

INCLUDEPATH += ../PNN
#INCLUDEPATH += ./Sources

#linux:LIBS += -lgio-2.0
win32:LIBS += -lole32 -luuid

DEFINES += CNN_USE_SSE \
           CNN_USE_AVX \
           NDEBUG
#           HAVE_CXXABI_H
#           DNN_USE_IMAGE_API

QMAKE_CXXFLAGS += -std=gnu++14 -m64 -mavx -msse3 -Wa,-mbig-obj
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3

QMAKE_LFLAGS_RELEASE -= -O1

MOC_DIR = Build/.moc
RCC_DIR = Build/.rcc
OBJECTS_DIR = Build/.obj

HEADERS += \
    Sources/backend.h \
    Sources/config.h \
    Sources/enn_chapar.h \
    Sources/enn_dataset.h \
    Sources/enn_dataset_image.h \
    Sources/enn_network.h \
    Sources/enn_test.h \
    Sources/td_avepool.h \
    Sources/td_convolution.h \
    Sources/td_fc.h \
    Sources/td_layer.h \
    Sources/td_leaky_relu.h \
    Sources/td_network.h \
    Sources/td_sequential.h \
    Sources/td_softmax.h

SOURCES += \
    Sources/backend.cpp \
    Sources/enn_chapar.cpp \
    Sources/enn_dataset.cpp \
    Sources/enn_dataset_image.cpp \
    Sources/enn_network.cpp \
    Sources/enn_test.cpp \
    Sources/main.cpp \
    Sources/td_avepool.cpp \
    Sources/td_convolution.cpp \
    Sources/td_fc.cpp \
    Sources/td_layer.cpp \
    Sources/td_leaky_relu.cpp \
    Sources/td_network.cpp \
    Sources/td_sequential.cpp \
    Sources/td_softmax.cpp

win32:SOURCES += ../PNN/aj_dllgen.cpp
