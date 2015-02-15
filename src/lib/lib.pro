TEMPLATE = lib
TARGET = ololord

VERSION = 0.1.0
VER_MAJ = 0
VER_MIN = 1
VER_PAT = 0

CONFIG += release

QT = gui
BEQT = core

include(../../prefix.pri)

DEFINES += OLOLORD_BUILD_LIB

SOURCES += \
    cache.cpp \
    database.cpp \
    ololordapplication.cpp \
    settingslocker.cpp \
    tools.cpp \
    translator.cpp

HEADERS += \
    cache.h \
    database.h \
    global.h \
    ololordapplication.h \
    settingslocker.h \
    tools.h \
    translator.h \

include(board/board.pri)
include(controller/controller.pri)
include(plugin/plugin.pri)
include(route/route.pri)
include(stored/stored.pri)

#Processing CppCMS templates
mac|unix {
    CPPCMS_PROCESSING_COMMAND=$${CPPCMS_PREFIX}/bin/cppcms_tmpl_cc
} else:win32 {
    CPPCMS_PROCESSING_COMMAND=$${CPPCMS_PREFIX}/bin/cppcms_tmpl_cc.exe
}

CPPCMS_TEMPLATES=$$files($${PWD}/template/*)
for(CPPCMS_TEMPLATE, CPPCMS_TEMPLATES) {
    CPPCMS_TEMPLATES_STRING=$${CPPCMS_TEMPLATES_STRING} \"$${CPPCMS_TEMPLATE}\"
}

system($${CPPCMS_PROCESSING_COMMAND} $${CPPCMS_TEMPLATES_STRING} -o \"$${PWD}/compiled_templates.cpp\")

SOURCES += compiled_templates.cpp
#end processing

contains(LORD_CONFIG, builtin_resources) {
    DEFINES += OLOLORD_BUILTIN_RESOURCES
    RESOURCES += \
        ololord_res.qrc \
        ololord_static.qrc \
        ololord_static_css.qrc \
        ololord_static_img.qrc \
        ololord_static_js.qrc \
        ololord_static_video.qrc
}

##############################################################################
################################ Installing ##################################
##############################################################################

!contains(LORD_CONFIG, no_install) {

contains(LORD_CONFIG, libs) {

##############################################################################
################################ Libs ########################################
##############################################################################

target.path = $${LORD_LIBS_INSTALLS_PATH}
INSTALLS = target

}

} #end !contains(LORD_CONFIG, no_install)
