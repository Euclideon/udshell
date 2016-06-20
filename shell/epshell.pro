QT += core qml quick gui widgets
TEMPLATE = app
TARGET = "epshell"

CONFIG(debug, debug|release) {
  CONFIG += debugEP
}
else {
  CONFIG += releaseEP
}

win32:debugEP:TOOLSET = DebugQML_x64
win32:releaseEP:TOOLSET = ReleaseQML_x64

unix:debugEP:TOOLSET = Debug_
unix:releaseEP:TOOLSET = Release_

debugEP:DEFINES += _DEBUG

DESTDIR = $${PWD}/../
build_pass:DESTDIR = $${PWD}/../public/bin/$${TOOLSET}

CONFIG += c++11 qml_debug

SOURCES += src/main.cpp
HEADERS += src/messagebox.h \
           src/dinkey.h
INCLUDEPATH += ../libep/public/include \
               ../kernel/src

RESOURCES += res/res.qrc \
             qml/components/components.qrc \
             qml/platform/controls/controls.qrc \
             qml/platform/themes/themes.qrc \

unix:CONFIG += no_lflags_merge

LIB_PATH = $${PWD}/../bin/$${TOOLSET}
UD_LIB_PATH = $${PWD}/../ud/Output/lib/$${TOOLSET}
LIBS += -L$${LIB_PATH} -llibep \
        -L$${LIB_PATH} -lepkernel \
        -L$${LIB_PATH} -lhal-qt \
        -L$${LIB_PATH} -lepkernel \
        -L$${UD_LIB_PATH} -ludPointCloud \
        -L$${UD_LIB_PATH} -ludPlatform \
        -L$${LIB_PATH} -llua-5.3.1

win32 {
  THIRDPARTY_LIB_PATH = $${PWD}/../3rdparty/
  LIBS += -L$${THIRDPARTY_LIB_PATH}/assimp-3.1.1/lib/windows/x64 -lassimp-ep64 \
          -L$${LIB_PATH} -lpcre \
          -lws2_32 \
          -lwinmm
}
unix {
  LIBS += -L$${PWD}/../bin/amd64/ -lassimp-ep \
          -lpcre \
          -ldl \
          -lpthread
}
