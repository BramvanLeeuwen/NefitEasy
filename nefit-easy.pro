#QT += core
SOURCES += mainJson.cpp  nefit-easy.cpp \
    jsonparser.cpp \
    iniparser.cpp \
    Tcp.cpp
LIBS = -L/usr/local/lib -lstrophe -lssl -lcrypto -ljson-c
INCLUDEPATH += /usr/include/c++/4.8 /usr/include/i386-linux-gnu/c++/4.8

HEADERS += nefit-easy.h \
    jsonparser.hpp \
    iniparser.hpp \
    Tcp.hpp \
    main.hpp
