TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    zfs.cpp \
    mkfs_net.cpp

HEADERS += \
    zfs.h \
    mkfs_net.h
