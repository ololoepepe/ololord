#Defining ololord subdir name
isEmpty(LORD_SUBDIR_NAME):LORD_SUBDIR_NAME=ololord

#Searching for headers
lordHeadersPath=$${PWD}/../../include/$${LORD_SUBDIR_NAME}
!exists($${lordHeadersPath}):error("ololord headers not found")

#Searching for libraries
lordLibsPath=$${PWD}/../../lib
!exists($${lordLibsPath}):error("ololord lib not found")

win32 {
    #Set suffix for libraries names
    libNameSuffix=0
}

INCLUDEPATH *= $${lordHeadersPath}
DEPENDPATH *= $${lordHeadersPath}
LIBS *= -L$${lordLibsPath}/ -ololord

#CppCMS
!contains(LORD_CONFIG, no_cppcms) {
    isEmpty(CPPCMS_PREFIX) {
        mac|unix {
            CPPCMS_PREFIX=/usr
        } else:win32 {
            error(CppCMS path is not specified)
        }
    }

    INCLUDEPATH *= $${CPPCMS_PREFIX}/include
    DEPENDPATH *= $${CPPCMS_PREFIX}/include
    LIBS *= -L$${CPPCMS_PREFIX}/lib/ -lcppcms -lbooster

    #Processing CppCMS templates
    defineReplace(processCppcmsTemplates) {
        lordCppcmsTemplatesPath=$${1}
        lordCppcmsOutputPath=$${2}
        !isEmpty(lordCppcmsTemplatesPath):!isEmpty(lordCppcmsOutputPath) {    
            mac|unix {
                CPPCMS_PROCESSING_COMMAND=$${CPPCMS_PREFIX}/bin/cppcms_tmpl_cc
            } else:win32 {
                CPPCMS_PROCESSING_COMMAND=$${CPPCMS_PREFIX}/bin/cppcms_tmpl_cc.exe
            }

            CPPCMS_TEMPLATES=$$files($${lordCppcmsTemplatesPath}/*)
            for(CPPCMS_TEMPLATE, CPPCMS_TEMPLATES) {
                CPPCMS_TEMPLATES_STRING=$${CPPCMS_TEMPLATES_STRING} \"$${CPPCMS_TEMPLATE}\"
            }

            !isEmpty(CPPCMS_TEMPLATES) {
                CPPCMS_OUT_STRING=\"$${lordCppcmsOutputPath}/compiled_templates.cpp\"
                CPPCMS_PROCESSING_COMMAND=$${CPPCMS_PROCESSING_COMMAND} $${CPPCMS_TEMPLATES_STRING}
                CPPCMS_PROCESSING_COMMAND=$${CPPCMS_PROCESSING_COMMAND} -o $${CPPCMS_OUT_STRING}
                system($${CPPCMS_PROCESSING_COMMAND})
                SOURCES += compiled_templates.cpp
            }
        }
    }
    #end processing
}

#ODB
!contains(LORD_CONFIG, no_odb) {
    isEmpty(ODB_PREFIX) {
        mac|unix {
            ODB_PREFIX=/usr
        } else:win32 {
            error(ODB path is not specified)
        }
    }

    INCLUDEPATH *= $${ODB_PREFIX}/include
    DEPENDPATH *= $${ODB_PREFIX}/include
    LIBS *= -L$${ODB_PREFIX}/lib/ -lodb -lodb-sqlite
    
    !isEmpty(ODB_QT_PREFIX) {
        INCLUDEPATH *= $${ODB_QT_PREFIX}/include
        DEPENDPATH *= $${ODB_QT_PREFIX}/include
        LIBS *= -L$${ODB_QT_PREFIX}/lib/ -lodb-qt
    } else {
        LIBS *= -L$${ODB_PREFIX}/lib/ -lodb-qt
    }
} else {
    DEFINES += OLOLORD_NO_ODB
}

#libcurl
contains(LORD_CONFIG, libcurl) {
    isEmpty(LIBCURL_PREFIX) {
        mac|unix {
            LIBCURL_PREFIX=/usr
        } else:win32 {
            error(libcurl path is not specified)
        }
    }

    INCLUDEPATH *= $${LIBCURL_PREFIX}/include
    DEPENDPATH *= $${LIBCURL_PREFIX}/include
    LIBS *= -L$${LIBCURL_PREFIX}/lib/ -lcurl

    !contains() {
        isEmpty(CURLPP_PREFIX) {
            mac|unix {
                CURLPP_PREFIX=/usr
            } else:win32 {
                error(cURLpp path is not specified)
            }
        }

        INCLUDEPATH *= $${CURLPP_PREFIX}/include
        DEPENDPATH *= $${CURLPP_PREFIX}/include
        LIBS *= -L$${CURLPP_PREFIX}/lib/ -lcurlpp
    }
}
