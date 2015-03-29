TARGET = ComputeCL
TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt rtti

CONFIG(debug, debug|release):SUFFIX=d
else:SUFFIX=

TARGET=$$TARGET$$SUFFIX

*-g++:QMAKE_TARGET.arch = $$QMAKE_HOST.arch
*-g++-32:QMAKE_TARGET.arch = x86
*-g++-64:QMAKE_TARGET.arch = x86_64

SPEC=$$basename(QMAKESPEC)
SPEC=$$replace(SPEC, win32, windows)-$$QMAKE_TARGET.arch

LIBDIR=$$PWD/../lib
staticlib {
    DEFINES += STATIC
    DESTDIR=$$LIBDIR
} else {
    # Binaries and dynamic libs goes to bin
    DESTDIR=$$PWD/../bin
    static:DESTDIR=$$DESTDIR-static
}

LIBDIR=$$LIBDIR/$$SPEC
DESTDIR=$$DESTDIR/$$SPEC

LIBS += "-L$$LIBDIR"

WD=$$PWD
win32:{
    INCLUDEPATH += "C:/Program Files (x86)/AMD APP SDK/2.9-1/include" "C:\Program Files (x86)\AMD APP SDK\3.0-0-Beta\include"
    INCLUDEPATH += C:/Programa/boost_1_57_0 "$$WD/../boost_1_57_0"
    LIBS += "-LC:/Programa/boost_1_57_0/stage/lib" "-L$$WD/../boost_1_57_0/stage/lib"

    message("arch=$$QMAKE_TARGET.arch")
    win32-*:contains(QMAKE_HOST.arch, x86_64) {
        LIBS += "-LC:/Program Files (x86)/AMD APP SDK/2.9-1/lib/x86_64" "-LC:/Program Files (x86)/AMD APP SDK/3.0-0-Beta/lib/x86_64"
    } else {
        LIBS += "-LC:/Program Files (x86)/AMD APP SDK/2.9-1/lib/x86" "-LC:/Program Files (x86)/AMD APP SDK/3.0-0-Beta/lib/x86"
    }
} else {
    INCLUDEPATH += /home/puesto/Programa/compute/include /opt/AMDAPPSDK-3.0-0-Beta/include
    LIBS += "-L/opt/AMDAPPSDK-3.0-0-Beta/lib/x86"

    linux-*:contains(QMAKE_TARGET.arch, x86_64) {
    } else {
    }
}

DEFINES += _SCL_SECURE_NO_WARNINGS
INCLUDEPATH += "$$WD/../compute/include" "$$WD/../half"

LIBS += -lOpenCL -lhalf$$SUFFIX

SOURCES += main.cpp

include(deployment.pri)
qtcAddDeployment()


