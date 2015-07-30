TEMPLATE = app
TARGET = ololord

CONFIG += release console

QT = gui
BEQT = core network sql

include(../../prefix.pri)

ololordHeadersPath=$${PWD}/../../include
ololordLibsPath=$${OUT_PWD}/..

win32 {
    #If CONFIG contains "release" or "debug", set special suffix for libs' path
    releaseDebugSuffix=
    CONFIG(release, debug|release):releaseDebugSuffix=/release
    CONFIG(debug, debug|release):releaseDebugSuffix=/debug
    #Set suffix for libraries names
    libNameSuffix=0
}

INCLUDEPATH += $${ololordHeadersPath}
DEPENDPATH += $${ololordHeadersPath}
LIBS += -L$${OUT_PWD}/../lib$${releaseDebugSuffix}/ -lololord$${libNameSuffix}

SOURCES += \
    ololordajaxwebapp.cpp \
    main.cpp \
    ololordwebapp.cpp \
    ololordwebappthread.cpp

HEADERS += \
    ololordajaxwebapp.h \
    ololordwebapp.h \
    ololordwebappthread.h

contains(LORD_CONFIG, builtin_resources) {
    DEFINES += BUILTIN_RESOURCES
    RESOURCES += \
        ../../translations/ololord_translations.qrc
}

##############################################################################
################################ Installing ##################################
##############################################################################

!contains(LORD_CONFIG, no_install) {

##############################################################################
################################ Binaries ####################################
##############################################################################

target.path = $${LORD_BINARY_INSTALLS_PATH}
INSTALLS = target

} #end !contains(LORD_CONFIG, no_install)
