QT -= gui

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++17

INCLUDEPATH += \
        /usr/local/include

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

LIBS +=  \
        -L/usr/local/lib \
		-lfftw3 \

SOURCES += \
    datablock.cc \
    datamgr.cc

HEADERS += \
    constants.h \
    datablock.h \
    datamgr.h \
    libra.h \
    preamble.h \
    properties.h \
    singleton.h \
    testable.h

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target
