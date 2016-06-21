TEMPLATE = lib
TARGET = "%{ProjectName}"

CONFIG(debug, debug|release) {
  DEFINES += _DEBUG
}

CONFIG += c++11

SOURCES += src/%{ProjectName}.cpp
HEADERS += src/%{ProjectName}.h
RESOURCES += src/%{ProjectName}.qrc

win32 {
  ARCH = x86
  contains (QMAKE_TARGET.arch, x86_64) {
    ARCH = x64
  }

  INCLUDEPATH += $$(EPDIR)/include
  LIBS += -L$$(EPDIR)/lib/$${ARCH} -llibep
}
else {
  LIBS += -llibep
}
