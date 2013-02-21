#-------------------------------------------------
#
# Project created by QtCreator 2012-12-04T11:37:45
#
#-------------------------------------------------

QT       += core gui

TARGET = qSbfToVtk
TEMPLATE = app


SOURCES += main.cpp\
        sbftovtkdialog.cpp\
        ../sbfToVTK/src/sbfToVTK.cpp

HEADERS  += sbftovtkdialog.h\
        ../sbfToVTK/src/sbfToVTK.h

FORMS    += sbftovtkdialog.ui

win32:LIBS +=   -L"X:/Program64/libsbf/releaseWin"\
                -L"C:/Program Files (x86)/VTK/lib/vtk-5.10"
LIBS += -lsbf\
        -lvtkFiltering\
        -lvtksys\
        -lvtkCommon\
        -lvtkIO
unix:INCLUDEPATH +=     "../../../libsbf/src"\
                        "../sbfToVTK/src"\
                        "/usr/include/vtk-5.8"
win32:INCLUDEPATH += "C:/Program Files (x86)/VTK/include/vtk-5.10"\
                    "X:/Program64/sbfToVTK/src"\
                    "X:/Program64/libsbf/src"
