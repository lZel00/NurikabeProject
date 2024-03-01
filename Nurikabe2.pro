TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        checks.cpp \
        main.cpp \
        solver.cpp \
        utils.cpp

HEADERS += \
    cell.h \
    solver.h \
    utils.h
