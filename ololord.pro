TEMPLATE = subdirs
SUBDIRS = src

include(prefix.pri)

##############################################################################
################################ Installing ##################################
##############################################################################

!contains(LORD_CONFIG, no_install) {

##############################################################################
################################ Translations ################################
##############################################################################

!contains(LORD_CONFIG, builtin_resources) {
    installsTranslations.files=$$files($${PWD}/translations/*.qm)
    installsTranslations.path=$${LORD_RESOURCES_INSTALLS_PATH}/translations
    INSTALLS += installsTranslations
}

##############################################################################
################################ Other resources #############################
##############################################################################

installsSrchilite.files=$$files($${PWD}/srchilite/*)
installsSrchilite.path=$${LORD_RESOURCES_INSTALLS_PATH}/srchilite
INSTALLS += installsSrchilite

##############################################################################
################################ Headers #####################################
##############################################################################

#Gets contents of an .h header
#Returns the corresponding actual header path
defineReplace(getActualHeaderInternal) {
    headerContents=$${1}
    actualHeader=$$replace(headerContents, "$${LITERAL_HASH}include", "")
    actualHeader=$$replace(actualHeader, "\\.\\./", "")
    actualHeader=$$replace(actualHeader, "\"", "")
    return($${PWD}/$${actualHeader})
}

#Gets path to a header (either .h or with no extension)
#Returns corresponding actual header path
defineReplace(getActualHeader) {
    headerPath=$${1}
    headerContents=$$cat($${headerPath})
    isEmpty(headerContents) {
        headerPath=
    } else:!equals(headerContents, $$replace(headerContents, "\\.\\./", "")) {
        headerPath=$$getActualHeaderInternal($${headerContents})
    }
    return($${headerPath})
}

#Gets include subdirectory name
#Returns a list of actual headers' paths to which headers in the given subdir point
defineReplace(getActualHeaders) {
    prefix=$${1}
    !isEmpty(prefix):prefix=$${prefix}/
    headerPaths=$$files($${PWD}/include/$${prefix}*)
    actualHeaderPaths=
    for(headerPath, headerPaths) {
        actualHeaderPath=$$getActualHeader($${headerPath})
        !isEmpty(actualHeaderPath):!equals(actualHeaderPath, $${PWD}/):actualHeaderPaths+=$${actualHeaderPath}
    }
    return($${actualHeaderPaths})
}

contains(LORD_CONFIG, headers) {
    #Global
    lordInstallsHeadersGlobal.files=$$getActualHeaders()
    lordInstallsHeadersGlobal.path=$${LORD_HEADERS_INSTALLS_PATH}
    INSTALLS += lordInstallsHeadersGlobal
    #board
    lordInstallsHeadersBoard.files=$$getActualHeaders(board)
    lordInstallsHeadersBoard.path=$${LORD_HEADERS_INSTALLS_PATH}/board
    INSTALLS += lordInstallsHeadersBoard
    #controller
    lordInstallsHeadersController.files=$$getActualHeaders(controller)
    lordInstallsHeadersController.path=$${LORD_HEADERS_INSTALLS_PATH}/controller
    INSTALLS += lordInstallsHeadersController
    #plugin
    lordInstallsHeadersPlugin.files=$$getActualHeaders(plugin)
    lordInstallsHeadersPlugin.path=$${LORD_HEADERS_INSTALLS_PATH}/plugin
    INSTALLS += lordInstallsHeadersPlugin
    #stored
    lordInstallsHeadersStored.files=$$getActualHeaders(stored)
    lordInstallsHeadersStored.path=$${LORD_HEADERS_INSTALLS_PATH}/stored
    INSTALLS += lordInstallsHeadersStored
} #end contains(LORD_CONFIG, headers)

#Depend
lordInstallsDepend.files=$${PWD}/depend.pri
lordInstallsDepend.path=$${LORD_RESOURCES_INSTALLS_PATH}
INSTALLS += lordInstallsDepend

} #end !contains(LORD_CONFIG, no_install)
