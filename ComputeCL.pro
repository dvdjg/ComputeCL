TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

win32:{
    INCLUDEPATH += "C:/Program Files (x86)/AMD APP SDK/2.9-1/include"
    INCLUDEPATH += C:/Programa/boost_1_57_0
    INCLUDEPATH += "C:/Users/DavidJurado/Documents/Programa/compute/include"
    LIBS += "-LC:/Programa/boost_1_57_0/stage/lib"

    win32-*:contains(QMAKE_HOST.arch, x86_64):{
        LIBS += "-LC:/Program Files (x86)/AMD APP SDK/2.9-1/lib/x86_64"
        message("64 bits build")
    } else {
        LIBS += "-LC:/Program Files (x86)/AMD APP SDK/2.9-1/lib/x86"
        message("32 bits build")
    }
} else {
    INCLUDEPATH += /home/puesto/Programa/compute/include /opt/AMDAPPSDK-3.0-0-Beta/include
    LIBS += "-L/opt/AMDAPPSDK-3.0-0-Beta/lib/x86"

    linux-*:contains(QMAKE_HOST.arch, x86_64):{
        message("64 bits build")
    } else {
        message("32 bits build")
    }
}

LIBS += -lOpenCL

SOURCES += main.cpp

include(deployment.pri)
qtcAddDeployment()
