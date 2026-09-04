QT += widgets

CONFIG += c++17
CONFIG -= debug_and_release

TEMPLATE = app
TARGET = LightNovelSystem

SOURCES += \
    article.cpp \
    articleeditdialog.cpp \
    articlereaddialog.cpp \
    logindialog.cpp \
    main.cpp \
    mainwindow.cpp \
    noveldata.cpp \
    registerdialog.cpp \
    report.cpp \
    reportdialog.cpp \
    useraccount.cpp

HEADERS += \
    article.h \
    articleeditdialog.h \
    articlereaddialog.h \
    logindialog.h \
    mainwindow.h \
    noveldata.h \
    registerdialog.h \
    report.h \
    reportdialog.h \
    useraccount.h

FORMS += \
    articleeditdialog.ui \
    articlereaddialog.ui \
    logindialog.ui \
    mainwindow.ui \
    registerdialog.ui \
    reportdialog.ui

RESOURCES += decorations.qrc
