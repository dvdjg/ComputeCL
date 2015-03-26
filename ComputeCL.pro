TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += "C:/Program Files (x86)/AMD APP SDK/2.9-1/include"
INCLUDEPATH += C:/Programa/boost_1_57_0
INCLUDEPATH += "C:/Users/DavidJurado/Documents/Programa/compute/include"

win32:{
    win32-*:contains(QMAKE_HOST.arch, x86_64):{
        LIBS += "-LC:/Program Files (x86)/AMD APP SDK/2.9-1/lib/x86_64"
        message("64 bits build")
    } else {
        LIBS += "-LC:/Program Files (x86)/AMD APP SDK/2.9-1/lib/x86"
        message("32 bits build")
    }
}

LIBS += "-LC:/Programa/boost_1_57_0/stage/lib"

LIBS += -lOpenCL

SOURCES += main.cpp

include(deployment.pri)
qtcAddDeployment()
