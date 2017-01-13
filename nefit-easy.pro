#QT += core
SOURCES += mainJson.cpp  nefit-easy.cpp \
    jsonparser.cpp \
    iniparser.cpp \
    TcpClient.cpp
LIBS = -L/usr/local/lib -lstrophe -lssl -lcrypto -ljson-c
INCLUDEPATH += /usr/include/c++/4.8 /usr/include/i386-linux-gnu/c++/4.8

HEADERS += easyheader.hpp \
    jsonparser.hpp \
    iniparser.hpp \
    TcpClient.hpp
