isEmpty(BEQT_PREFIX) {
    mac|unix {
        BEQT_PREFIX=/usr/share/beqt
    } else:win32 {
        BEQT_PREFIX=$$(systemdrive)/PROGRA~1/BeQt
    }
}
include($${BEQT_PREFIX}/share/beqt/depend.pri)

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

isEmpty(BOOST_PREFIX) {
    mac|unix {
        BOOST_PREFIX=/usr
    } else:win32 {
        error(Boost path is not specified)
    }
}

INCLUDEPATH *= $${BOOST_PREFIX}/include
DEPENDPATH *= $${BOOST_PREFIX}/include
LIBS *= -L$${BOOST_PREFIX}/lib/ -lboost_regex

isEmpty(SRCHILITE_PREFIX) {
    mac|unix {
        SRCHILITE_PREFIX=/usr
    } else:win32 {
        error(GNU Source-highlight path is not specified)
    }
}

INCLUDEPATH *= $${SRCHILITE_PREFIX}/include
DEPENDPATH *= $${SRCHILITE_PREFIX}/include
LIBS *= -L$${SRCHILITE_PREFIX}/lib/ -lsource-highlight

mac|unix {
    isEmpty(LORD_PREFIX):LORD_PREFIX=/usr
} else:win32 {
    isEmpty(LORD_PREFIX):PREFIX=$$(systemdrive)/PROGRA~1/ololord
}

isEmpty(LORD_BINARY_INSTALLS_PATH):LORD_BINARY_INSTALLS_PATH=$${LORD_PREFIX}/bin
isEmpty(LORD_RESOURCES_INSTALLS_PATH):LORD_RESOURCES_INSTALLS_PATH=$${LORD_PREFIX}/share/ololord
isEmpty(LORD_HEADERS_INSTALLS_PATH):LORD_HEADERS_INSTALLS_PATH=$${LORD_PREFIX}/include/ololord
isEmpty(LORD_LIBS_INSTALLS_PATH):LORD_LIBS_INSTALLS_PATH=$${LORD_PREFIX}/lib
