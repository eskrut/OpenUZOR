#-------------------------------------------------
#
# Project created by QtCreator 2012-12-04T11:37:45
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qSbfToVtk
TEMPLATE = app

CONFIG += c++11

SOURCES += main.cpp\
        sbftovtkdialog.cpp\
        $$PWD/../sbfToVtk/src/sbfToVTK.cpp

HEADERS  += sbftovtkdialog.h\
        $$PWD/../sbfToVtk/src/sbfToVTK.h

FORMS    += sbftovtkdialog.ui

win32:LIBS +=   -L"X:/Program64/libsbf/releaseWin"\
                -L"C:/Program Files (x86)/VTK/lib/vtk-5.10"
LIBS += -L"$$(HOME)/lib"\
        -L"$$(VTK_BASE)/lib"\
        -lsbf\
        -lvtkFiltersCore-6.1\
        -lvtksys-6.1\
        -lvtkCommonCore-6.1\
        -lvtkCommonDataModel-6.1\
        -lvtkIOCore-6.1 \
        -lvtkIOExport-6.1 \
        -lvtkIOXML-6.1
unix:INCLUDEPATH +=     "$$PWD/../../../libsbf/src"\
                        "$$PWD/../sbfToVtk/src"\
                        "/usr/include/vtk-5.8" \
                        "$$(VTK_BASE)/include/vtk-6.1"
win32:INCLUDEPATH += "C:/Program Files (x86)/VTK/include/vtk-5.10"\
                    "X:/Program64/sbfToVTK/src"\
                    "X:/Program64/libsbf/src"
