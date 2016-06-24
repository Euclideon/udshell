TEMPLATE = lib
TARGET = "%{ProjectName}"

ARCH = x86
contains (QMAKE_TARGET.arch, x86_64) {
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

CONFIG += c++11

SOURCES += src/%{ProjectName}.cpp
HEADERS += src/%{ProjectName}.h

win32 {
  EPDIR = $$(EPDIR)
  isEmpty(EPDIR):error(EPDIR environment variable is not set. If this was recently changed you may need to restart your computer to take effect.)
  INCLUDEPATH += $$(EPDIR)/include
  LIBS += -L$$(EPDIR)/lib/$${ARCH} -llibep
}
else {
  LIBS += -llibep
}
