TEMPLATE = lib
TARGET = "%{ProjectName}"

ARCH = x86
contains (QMAKE_TARGET.arch, x86_64) {
  ARCH = x64
}

CONFIG(debug, debug|release) {
  DEFINES += _DEBUG
  TOOLSET = Debug_$${ARCH}
  win32 {
    DEFINES += QT_NO_DEBUG
    CONFIG -= debug debug_and_release debug_and_release_target
    CONFIG += release force_debug_info
    QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO = $$replace(QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO, "-O2", "-Od")
    QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO = $$replace(QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO, "-O2", "-Od")
  }
}
else {
  TOOLSET = Release_$${ARCH}
}

DESTDIR = $${PWD}/bin/$${TOOLSET}
MOC_DIR = $${PWD}/int/.moc/$${TOOLSET}
OBJECTS_DIR = $${PWD}/int/.obj/$${TOOLSET}
RCC_DIR = $${PWD}/int/.rcc/$${TOOLSET}
UI_DIR = $${PWD}/int/.ui/$${TOOLSET}

CONFIG += c++11 plugin
QT -= gui

SOURCES += src/%{ProjectName}.cpp
HEADERS += src/%{ProjectName}.h
RESOURCES += src/%{ProjectName}.qrc

win32 {
  EPDIR = $$(EPDIR)
  isEmpty(EPDIR):error(EPDIR environment variable is not set. If this was recently changed you may need to restart your computer to take effect.)
  INCLUDEPATH += $$(EPDIR)/include
  LIBS += -L$$(EPDIR)/lib/$${ARCH} -llibep
}
else {
  QMAKE_CFLAGS_DEBUG += -O0
  QMAKE_CXXFLAGS_DEBUG += -O0
  LIBS += -llibep
}
