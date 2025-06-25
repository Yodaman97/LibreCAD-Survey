QT       += widgets
TEMPLATE = lib
CONFIG += plugin
VERSION = 1.0.0
TARGET = $$qtLibraryTarget(surveytools)

GENERATED_DIR = ../../generated/plugin/surveytools
include(../../common.pri)

INCLUDEPATH    += ../../librecad/src/plugins

SOURCES += surveytools.cpp \
           pointmanager.cpp
           importcsvdialog.cpp

HEADERS += surveytools.h \
           pointmanager.h
           importcsvdialog.h

win32 {
    DESTDIR = ../../windows/resources/plugins
}
unix {
    macx {
        DESTDIR = ../../LibreCAD.app/Contents/Resources/plugins
    } else {
        DESTDIR = ../../unix/resources/plugins
    }
}
