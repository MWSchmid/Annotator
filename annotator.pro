#-------------------------------------------------
#
# Project created by QtCreator 2012-03-05T12:53:53
#
#-------------------------------------------------

QT       += core gui xml network ftp http widgets

TARGET = annotator
TEMPLATE = app


SOURCES += annotationDataStructure/annotationdatastructure.cpp \
    annotationDataStructure/annotationitem.cpp \
    annotationDataStructure/annotationmodel.cpp \
    annotationDataStructure/ftploader.cpp \
    annotationDataStructure/httploader.cpp \
    annotationDataStructure/annotationdatareader.cpp \
    annotationDataStructure/annotationdatawriter.cpp \
    annotationDataStructure/annotationlink.cpp \
    annotatorview.cpp \
    main.cpp

HEADERS  += annotationDataStructure/annotationdatastructure.h \
    annotationDataStructure/annotationitem.h \
    annotationDataStructure/annotationmodel.h \
    annotationDataStructure/ftploader.h \
    annotationDataStructure/httploader.h \
    annotationDataStructure/annotationdatareader.h \
    annotationDataStructure/annotationdatawriter.h \
    annotationDataStructure/annotationDefinitions.h \
    annotationDataStructure/annotationlink.h \
    annotatorview.h

FORMS    += annotatorview.ui

include(qtiocompressor-2.3_1-opensource/src/qtiocompressor.pri)

