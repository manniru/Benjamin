TEMPLATE = app

QT += multimedia

CONFIG += console

INCLUDEPATH += ../PNN
win32:INCLUDEPATH += ../PNN/lua

linux:LIBS += -pthread -lm -ldl \
              -LKaldi/Libs -lportaudio -lasound -lrt -ljack -lfst
win32:LIBS += -L../PNN/libs -lFstWin64 -lPortAudio -lwinmm -llua54

DEFINES += HAVE_MKL \
           HAVE_CXXABI_H \
           CNN_USE_SSE \
           NDEBUG

QMAKE_CXXFLAGS += -std=c++14 -m64 -msse3 -pthread -mavx
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3

QMAKE_LFLAGS_RELEASE -= -O1

MOC_DIR = Build/.moc
RCC_DIR = Build/.rcc
OBJECTS_DIR = Build/.obj

OTHER_FILES = BaTool.conf

HEADERS += \
    Sources/backend.h \
    Sources/bt_captain.h \
    Sources/bt_cfb.h \
    Sources/bt_chapar.h \
    Sources/bt_cmvn.h \
    Sources/bt_cyclic.h \
    Sources/bt_delta.h \
    Sources/bt_enn.h \
    Sources/bt_feinput.h \
    Sources/bt_fft.h \
    Sources/bt_graph_d.h \
    Sources/bt_mbr_base.h \
    Sources/bt_melbank.h \
    Sources/bt_mfcc.h \
    Sources/bt_network.h \
    Sources/bt_recorder.h \
    Sources/bt_state.h \
    Sources/bt_test.h \
    Sources/bt_token_list.h \
    Sources/bt_wav_writer.h \
    Sources/bt_window.h \
    Sources/config.h \
    Sources/kd_a_model.h \
    Sources/kd_clat_weight.h \
    Sources/kd_decodable.h \
    Sources/kd_decoder.h \
    Sources/kd_f_token.h \
    Sources/kd_fst_util.h \
    Sources/kd_gmm.h \
    Sources/kd_hmm.h \
    Sources/kd_io.h \
    Sources/kd_lattice.h \
    Sources/kd_lattice_compact.h \
    Sources/kd_lattice_det.h \
    Sources/kd_lattice_functions.h \
    Sources/kd_lattice_prune.h \
    Sources/kd_lattice_string.h \
    Sources/kd_lattice_weight.h \
    Sources/kd_levenshtein.h \
    Sources/kd_matrix.h \
    Sources/kd_mbr.h \
    Sources/kd_model.h \
    Sources/kd_online_ldecoder.h \
    Sources/kd_online.h \
    Sources/kd_t_model.h \
    Sources/kd_token.h \
    Sources/ta_ini.h

SOURCES += \
    Sources/backend.cpp \
    Sources/bt_captain.cpp \
    Sources/bt_cfb.cpp \
    Sources/bt_chapar.cpp \
    Sources/bt_cmvn.cpp \
    Sources/bt_cyclic.cpp \
    Sources/bt_delta.cpp \
    Sources/bt_enn.cpp \
    Sources/bt_feinput.cpp \
    Sources/bt_fft.cpp \
    Sources/bt_graph_d.cpp \
    Sources/bt_mbr_base.cpp \
    Sources/bt_melbank.cpp \
    Sources/bt_mfcc.cpp \
    Sources/bt_network.cpp \
    Sources/bt_recorder.cpp \
    Sources/bt_state.cpp \
    Sources/bt_test.cpp \
    Sources/bt_token_list.cpp \
    Sources/bt_wav_writer.cpp \
    Sources/bt_window.cpp \
    Sources/kd_a_model.cpp \
    Sources/kd_clat_weight.cpp \
    Sources/kd_decodable.cpp \
    Sources/kd_decoder.cpp \
    Sources/kd_f_token.cpp \
    Sources/kd_fst_util.cpp \
    Sources/kd_gmm.cpp \
    Sources/kd_hmm.cpp \
    Sources/kd_io.cpp \
    Sources/kd_lattice.cpp \
    Sources/kd_lattice_compact.cpp \
    Sources/kd_lattice_det.cpp \
    Sources/kd_lattice_functions.cpp \
    Sources/kd_lattice_prune.cpp \
    Sources/kd_lattice_string.cpp \
    Sources/kd_lattice_weight.cpp \
    Sources/kd_levenshtein.cpp \
    Sources/kd_matrix.cpp \
    Sources/kd_mbr.cpp \
    Sources/kd_model.cpp \
    Sources/kd_online_ldecoder.cpp \
    Sources/kd_online.cpp \
    Sources/kd_t_model.cpp \
    Sources/kd_token.cpp \
    Sources/main.cpp \
    Sources/ta_ini.cpp \
    Sources/td_network.cpp

win32:HEADERS += Sources/bt_lua.h

win32:SOURCES += Sources/bt_lua.cpp
