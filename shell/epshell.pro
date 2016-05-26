QT += core qml quick gui
TEMPLATE = app
TARGET = "epshell"

TOOLSET = DebugQML_x64
CONFIG(release, debug|release) {
  TOOLSET = ReleaseQML_x64
}

DESTDIR = $${PWD}/../public/bin/$${TOOLSET}

CONFIG += c++11 qml_debug

SOURCES += ../shell/src/main.cpp
HEADERS += ../shell/src/messagebox.h
INCLUDEPATH += ../libep/public/include \
               ../kernel/src

RESOURCES += ../shell/res/qml.qrc

LIB_PATH = $${PWD}/../bin/$${TOOLSET}
UD_LIB_PATH = $${PWD}/../ud/Output/lib/$${TOOLSET}
THIRDPARTY_LIB_PATH = $${PWD}/../3rdparty/
LIBS += $${LIB_PATH}/epkernel.lib \
        $${LIB_PATH}/libep.lib \
        $${LIB_PATH}/hal-qt.lib \
        $${LIB_PATH}/pcre.lib \
        $${LIB_PATH}/lua-5.3.1.lib \
        $${UD_LIB_PATH}/udPlatform.lib \
        $${UD_LIB_PATH}/udPointCloud.lib \
        $${THIRDPARTY_LIB_PATH}/assimp-3.1.1/lib/windows/x64/assimp-ep64.lib \
        ws2_32.lib \
        winmm.lib
