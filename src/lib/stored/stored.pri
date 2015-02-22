SOURCES += \
    $${PWD}/banneduser.cpp \
    $${PWD}/postcounter.cpp \
    $${PWD}/registereduser.cpp \
    $${PWD}/thread.cpp

HEADERS += \
    $${PWD}/banneduser.h \
    $${PWD}/postcounter.h \
    $${PWD}/registereduser.h \
    $${PWD}/thread.h

#Processing ODB templates
mac|unix {
    ODB_PROCESSING_COMMAND=$${ODB_PREFIX}/bin/odb
    ODB_TEMPLATES=$$files($${PWD}/*.h)
} else:win32 {
    ODB_PROCESSING_COMMAND=$${ODB_PREFIX}/bin/odb.exe
    ODB_TEMPLATES=$$files($${PWD}\\*.h)
}

for(ODB_TEMPLATE, ODB_TEMPLATES) {
    ODB_TEMPLATES_STRING=$${ODB_TEMPLATES_STRING} \"$${ODB_TEMPLATE}\"
}

ODB_PROCESSING_COMMAND=$${ODB_PROCESSING_COMMAND} -d sqlite --generate-query --generate-schema --profile qt
ODB_PROCESSING_COMMAND=$${ODB_PROCESSING_COMMAND} -I \"$${QMAKE_INCDIR_QT}\"
ODB_PROCESSING_COMMAND=$${ODB_PROCESSING_COMMAND} -I \"$${QMAKE_INCDIR_QT}/QtCore\" $${ODB_TEMPLATES_STRING}

win32:ODB_PROCESSING_COMMAND=$$replace(ODB_PROCESSING_COMMAND, "/", "\\")

system($${ODB_PROCESSING_COMMAND})

HEADERS += $$files($${PWD}/*.hxx)
SOURCES += $$files($${PWD}/*.cxx)
#end processing
