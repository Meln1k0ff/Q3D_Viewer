#-------------------------------------------------
#
# Project created by QtCreator 2014-09-03T09:34:44
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TRANSLATIONS += 3d_viewer_en.ts \
                   3d_viewer_ru.ts \
                   3d_viewer_ua.ts


TARGET = 3d_viewer_alpha
TEMPLATE = app

SOURCES += \
        mainwindow.cpp \
    mesh.cpp \
    obj_parser.cpp \
    filereader.cpp \
    view3d.cpp \
    stripify.cpp \
    utils.cpp \
    _3ds_parser.cpp \
    main.cpp

HEADERS  += mainwindow.h \
    mesh.h \
    filereader.h \
    vec.h \
    #filewriter.h \
    view3d.h \
    matrix.h \
    stripify.h \
    utils.h \
    _3ds.h

RESOURCES += \
    res.qrc

INCLUDEPATH += glew-1.9.0/include


QMAKE_CXXFLAGS_RELEASE = -ffast-math -std=gnu++0x -std=c++0x -O3 -fomit-frame-pointer -fopenmp
QMAKE_CXXFLAGS_DEBUG = -O0 -g -std=gnu++0x -std=c++0x
FORMS    += mainwindow.ui
LIBS    += -lGLU -lGL  -lgomp -lGLEW

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../Загрузки/qwt-6.1.0/lib/release/ -lqwt
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../Загрузки/qwt-6.1.0/lib/debug/ -lqwt
#else:unix: LIBS += -L$$PWD/../../../Загрузки/qwt-6.1.0/lib/ -lqwt

#INCLUDEPATH += Big Bad Beetleborgs$$PWD/../../../Загрузки/qwt-6.1.0
#DEPENDPATH += $$PWD/../../../Загрузки/qwt-6.1.0

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../usr/local/qwt-6.1.0/lib/release/ -lqwt
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../usr/local/qwt-6.1.0/lib/debug/ -lqwt
#else:unix: LIBS += -L$$PWD/../../../../../usr/local/qwt-6.1.0/lib/ -lqwt

#INCLUDEPATH += $$PWD/../../../../../usr/local/qwt-6.1.0/include
#DEPENDPATH += $$PWD/../../../../../usr/local/qwt-6.1.0/include

