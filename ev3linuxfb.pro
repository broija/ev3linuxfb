TARGET = ev3linuxfb

PLUGIN_TYPE = platforms
PLUGIN_CLASS_NAME = QLinuxFbIntegrationPlugin
load(qt_plugin)

QT += core-private gui-private platformsupport-private

SOURCES = main.cpp \
    ev3screen.cpp \
    ev3integration.cpp

HEADERS = \
    ev3screen.h \
    ev3integration.h

CONFIG += qpa/genericunixfontdatabase

OTHER_FILES += ev3linuxfb.json
