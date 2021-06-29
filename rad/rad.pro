QT -= gui
QT += websockets sql

CONFIG += c++17 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += \
            ../ibra \
            ../rtlsdr \
            classes \
            /usr/local/include \
            /usr/include/libusb-1.0

macx {
INCLUDEPATH += /usr/local/include/libusb-1.0 /usr/local/include
LIBS += -L/usr/local/lib -lusb-1.0 -lsdrplay_api
}

SOURCES += \
        classes/config.cc \
        classes/fftaggregator.cc \
        classes/msgio.cc \
        classes/processor.cc \
        classes/soapyio.cc \
        classes/soapyworker.cc \
        classes/sourcemgr.cc \
        classes/sourcertlsdr.cc \
        classes/sourcesdrplay.cc \
        classes/taskfft.cc \
        classes/tester.cc \
        main.cc \
        rtlsdr/librtlsdr.c \
        rtlsdr/tuner_e4k.c \
        rtlsdr/tuner_fc0012.c \
        rtlsdr/tuner_fc0013.c \
        rtlsdr/tuner_fc2580.c \
        rtlsdr/tuner_r82xx.c

LIBS += \
        -L/usr/local/lib \
        -lfftw3 \
        -lSoapySDR \
        -lsdrplay_api \
        -lrtlsdr \
        -lusb-1.0


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    classes/config.h \
    classes/fftaggregator.h \
    classes/msgio.h \
    classes/processor.h \
    classes/soapyio.h \
    classes/soapyworker.h \
    classes/sourcebase.h \
    classes/sourcemgr.h \
    classes/sourcertlsdr.h \
    classes/sourcesdrplay.h \
    classes/taskfft.h \
    classes/tester.h \
    rtlsdr/reg_field.h \
    rtlsdr/rtl-sdr.h \
    rtlsdr/rtl-sdr_export.h \
    rtlsdr/rtlsdr_i2c.h \
    rtlsdr/tuner_e4k.h \
    rtlsdr/tuner_fc0012.h \
    rtlsdr/tuner_fc0013.h \
    rtlsdr/tuner_fc2580.h \
    rtlsdr/tuner_r82xx.h

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../ibra/release/ -libra
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../ibra/debug/ -libra
else:unix: LIBS += -L$$OUT_PWD/../ibra/ -libra

INCLUDEPATH += $$PWD/../ibra
DEPENDPATH += $$PWD/../ibra

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../ibra/release/libibra.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../ibra/debug/libibra.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../ibra/release/ibra.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../ibra/debug/ibra.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../ibra/libibra.a
