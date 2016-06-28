QT += core qml quick gui widgets
TEMPLATE = app
TARGET = "epshell"

win32 {
  ARCH = x86
  contains (QMAKE_TARGET.arch, x86_64) {
    ARCH = x64
    CONFIG += x64
  }
}

CONFIG(debug, debug|release) {
  DEFINES += _DEBUG
  TOOLSET = Debug_$${ARCH}
}
else {
  TOOLSET = Release_$${ARCH}
}

DESTDIR = $${PWD}/../
build_pass:DESTDIR = $${PWD}/bin/$${TOOLSET}
MOC_DIR = $${PWD}/int/.moc/$${TOOLSET}
OBJECTS_DIR = $${PWD}/int/.obj/$${TOOLSET}
RCC_DIR = $${PWD}/int/.rcc/$${TOOLSET}
UI_DIR = $${PWD}/int/.ui/$${TOOLSET}

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
  LIBS += -L$${LIB_PATH} -lpcre \
          -lws2_32 \
          -lwinmm
  x64:LIBS += -L$${THIRDPARTY_LIB_PATH}/assimp-3.1.1/lib/windows/x64 -lassimp-ep64
  else:LIBS += -L$${THIRDPARTY_LIB_PATH}/assimp-3.1.1/lib/windows/x32 -lassimp-ep32
}
unix {
  LIBS += -L$${PWD}/../bin/amd64/ -lassimp-ep \
          -lpcre \
          -ldl \
          -lpthread
}
