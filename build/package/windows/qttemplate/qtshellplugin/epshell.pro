QT += %{IncludeQtCore}%{IncludeQtQml}%{IncludeQtQuick}%{IncludeQtGui}
TEMPLATE = lib
TARGET = "%{ProjectName}"

TOOLSET = Debug_mingw32
CONFIG(release, debug|release) {
  TOOLSET = Release_mingw32
}


CONFIG += c++11

SOURCES += %{ProjectDirectory}/%{ProjectName}/src/%{ProjectName}.cpp
HEADERS += %{ProjectDirectory}/%{ProjectName}/headers/%{ProjectName}.h

LIB_PATH = "$$(EPDIR)"
INCLUDEPATH += "$$(EPDIR)/include"
LIBS += $${LIB_PATH}/lib/$${TOOLSET}/liblibep.a
