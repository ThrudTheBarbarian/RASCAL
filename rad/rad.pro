QT -= gui
QT += websockets sql

CONFIG += c++17 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += \
        ../ibra \
		classes \
		/usr/local/include

SOURCES += \
        classes/config.cc \
        classes/fftaggregator.cc \
        classes/msgio.cc \
        classes/processor.cc \
        classes/soapyio.cc \
        classes/soapyworker.cc \
        classes/taskfft.cc \
        classes/tester.cc \
        main.cc

LIBS += \
        -L/usr/local/lib \
        -lfftw3 \
		-lSoapySDR \

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
    classes/taskfft.h \
    classes/tester.h

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
