QT       += core gui websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


INCLUDEPATH += \
        ../shared/include \
        classes \
        /usr/local/include

SOURCES += \
    classes/config.cc \
    classes/events.cc \
    classes/graph.cc \
    classes/limiters.cc \
    classes/mainwindow.cc \
    classes/msgio.cc \
    classes/sky.cc \
    classes/vcr.cc \
    classes/waterfall.cc \
    main.cc \

HEADERS += \
    classes/config.h \
    classes/events.h \
    classes/graph.h \
    classes/limiters.h \
    classes/mainwindow.h \
    classes/msgio.h \
    classes/sky.h \
    classes/vcr.h \
    classes/waterfall.h

FORMS += \
    forms/mainwindow.ui \

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
