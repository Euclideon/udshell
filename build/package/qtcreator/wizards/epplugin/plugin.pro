TEMPLATE = lib
TARGET = "%{ProjectName}"

win32 {
  ARCH = x86
  contains (QMAKE_TARGET.arch, x86_64) {
    ARCH = x64
  }
}
else {
  ARCH = x64
}

CONFIG(debug, debug|release) {
  DEFINES += _DEBUG
  TOOLSET = Debug_$${ARCH}
}
else {
  TOOLSET = Release_$${ARCH}
}

DESTDIR = $${PWD}/bin/$${TOOLSET}
MOC_DIR = $${PWD}/int/.moc/$${TOOLSET}
OBJECTS_DIR = $${PWD}/int/.obj/$${TOOLSET}
RCC_DIR = $${PWD}/int/.rcc/$${TOOLSET}
UI_DIR = $${PWD}/int/.ui/$${TOOLSET}

CONFIG -= qt
CONFIG += c++11 plugin

SOURCES += src/%{ActivityCpp}
HEADERS += src/%{ActivityHdr}

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
