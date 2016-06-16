TEMPLATE = lib
TARGET = "%{ProjectName}"

CONFIG += c++11

SOURCES += %{ProjectDirectory}/%{ProjectName}/src/%{ProjectName}.cpp
HEADERS += %{ProjectDirectory}/%{ProjectName}/headers/%{ProjectName}.h

unix {
    INCLUDEPATH += "ep"
    INCLUDEPATH += %{ProjectDirectory}/%{ProjectName}/headers
    LIBS += -llibep
}
