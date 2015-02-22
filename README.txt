===============================================================================
 ololord
===============================================================================

Homepage: https://github.com/ololoepepe/ololord

Author: Andrey Bogdanov (ololoepepe@gmail.com)

License: GNU GPLv3 (see COPYING.txt)

ololord is an imageboard engine written in C++.

==============================================================================
 Overview
==============================================================================

ololord is a simple, lightweight imageboard engine. It provides basic
features, like Google captcha support and user (un)banning.

Functionality may be easily extended with the help of plugins.

==============================================================================
 Dependencies
==============================================================================

In order to build and use ololord you will need the same libraries and
tools as for every other project using Qt.

See: http://qt-project.org/resources/getting_started for details.

ololord is intended for use with Qt 5, or with Qt 4.8.
Support of Qt libraries' versions lower than 4.8.0 is not guaranteed.

You will also need BeQt libraries version 4.1.0 or higher.

See: https://github.com/ololoepepe/BeQt for details.

The following libraries are also required:

 * CppCMS (http://cppcms.com/wikipp/en/page/main)
 * ODB (http://www.codesynthesis.com/products/odb)
 * libcurl (http://curl.haxx.se/libcurl)
 * curlpp (https://code.google.com/p/curlpp)
 * boost (Regex) (http://www.boost.org)
 * GNU Source-Highlight (https://www.gnu.org/software/src-highlite)

==============================================================================
 Building and installing (briefly)
==============================================================================

In order to build ololord, just cd into the sources directory and execute
the following commands:

 * "qmake [BEQT_PREFIX=<path>] [CPPCMS_PREFIX=<path>] [ODB_PREFIX=<path>]
   [ODB_QT_PREFIX=<path>] [LIBCURL_PREFIX=<path>] [CURLPP_PREFIX=<path>]
   [BOOST_PREFIX=<path>] [SRCHILITE_PREFIX=<path>] [LIBMAGIC_PREFIX=<path>]"
   Here, the paths to BeQt, CppCMS, ODB, ODB-Qt, libcurl, curlpp, boost and
   GNU Source-Highlight may be specified

 * "make"
   or other similar command ("nmake", "mingw32-make", etc.)

 * "make install"
   You may need the superuser privileges in UNIX-like systems 

Note: When building from the command line, you have to configure
the environment (path to Qt libraries, etc.).

You may also use Qt Creator. After building the project, cd to
the output directory and execute the "make install" command, or
configure automatic execution of that command in Qt Creator.

See: http://qt-project.org/doc/qtcreator-2.6 for details.

==============================================================================
 Building and installing (in detail)
==============================================================================

When building ololord, you may pass some parameters to qmake:

 * "LORD_CONFIG+=builtin_resources"
   Embed resources (including translations) into executable file
   See: http://qt-project.org/doc/qt-5.0/resources.html for details

 * "LORD_CONFIG+=no_install"
   Don't install any files (building only)

 * "LORD_CONFIG+=headers"
   Install headers (required for creating plugins)

 * "LORD_CONFIG+=libs"
   Install libs (required for creating plugins)

 * "BEQT_PREFIX=<path>"
   Set path to BeQt libraries. Must be used if BeQt libraries were
   installed to a custom location

 * "CPPCMS_PREFIX=<path>"
   Set path to CppCMS libraries. Must be used if CppCMS libraries were
   installed to a custom location and must be used anyway on Windows

 * "ODB_PREFIX=<path>"
   Set path to ODB libraries. Must be used if ODB libraries were
   installed to a custom location and must be used anyway on Windows

 * "ODB_QT_PREFIX=<path>"
   Set path to ODB-Qt library. Must be used if ODB-Qt library was
   installed to a custom location and must be used anyway on Windows.
   This path defaults to ODB_PREFIX, but may be explicitly set if
   using several Qt versions and several ODB-Qt libraries linked
   against them

 * "LIBCURL_PREFIX=<path>"
   Set path to libcurl. Must be used if libcurl was installed to a
   custom location and must be used anyway on Windows

 * "CURLPP_PREFIX=<path>"
   Set path to curlpp library. Must be used if curlpp library were
   installed to a custom location and must be used anyway on Windows

 * "BOOST_PREFIX=<path>"
   Set path to boost libraries. Must be used if boost libraries were
   installed to a custom location

 * "SRCHILITE_PREFIX=<path>"
   Set path to GNU Source-Highlight library. Must be used if the library
   was installed to a custom location and must be used anyway on Windows

 * "LIBMAGIC_PREFIX=<path>"
   Set path to libmagic library. Must be used if the library was installed
   to a custom location and must be used anyway on Windows.
   Note: The recommended implementation is http://www.darwinsys.com/file/

 * "LORD_PREFIX=<path>"
   Set install path. See the note below

 * "LORD_BINARY_INSTALLS_PATH=<path>"
   Set install path for executable file. See the note below

 * "LORD_LIBS_INSTALLS_PATH=<path>"
   Set install path for headers. See the note below

 * "LORD_RESOURCES_INSTALLS_PATH=<path>"
   Set install path for resources. See the note below

 * "LORD_HEADERS_INSTALLS_PATH=<path>"
   Set install path for headers. See the note below

Note: In Windows systems Tololord is installed to the
"C:\Program files\ololord" directory (or other similar directory,
depending on your system) by default. Executable file and resources are
installed to the corresponding subdirectories.

In UNIX-like systems the directory structure during installation
looks like this:

 * "/usr/bin/ololord"
   Path to executable file

 * "/usr/lib"
   Path to libs

 * "/usr/share/ololord"
   Path to resources

 * "/usr/include/ololord"
   Path to headers

You may set another installation path. To do so,
pass the corresponding parameter to qmake (see above).

Warning: Don't use paths containing spaces. In Windows systems you may replace
directory names, containing spaces, with the corresponding short names:

See: http://support.microsoft.com/kb/142982 for details.

==============================================================================
 FAQ
==============================================================================

Q: What are ololord license restrictions?
A: There are almost no restrictions. You may use ololord as you wish,
but don't forget that this statement doesn't apply to the Qt libraries.
See: COPYING.txt, http://qt-project.org/products/licensing for details.

Q: I'm having troubles using ololord, where can I get help?
A: E-mail/Jabber: ololoepepe@gmail.com

Q: I've detected a bug/I have an idea, where can I report/post it?
A: See the answer above.

==============================================================================
 Deploying
==============================================================================

When deploying ololord, don't forget to include the resource files
(images, translations, etc. - see above).

It's recommended to build applications statically, because that helps avoid
dependency issues. You may use the "builtin_resources" parameter
in case of building your project statically.

For details, see:
http://qt-project.org/doc/qt-5.0/deployment.html#static-vs-shared-libraries

==============================================================================
 Writing plugins
==============================================================================

To write your own plugins for customizing ololord, just include the
depend.pri file installed in the resources location into your .pro file and
specify some parameters to qmake (see section Building and installing).

Currently the only supported plugin type is "board-factory" (include
plugins/BoardFactoryPluginInterface).

The boards provided by this plugin overwrites the built-in ones if their
names match.

To provide your own translations, you have to register the translations
file using Translato::registerTranslator. Do not include the locale name
suffix.

Once compiled, the plugin must be placed in the ololord user plugins
directory ("/home/user/.ololord/lib/ololord/plugins on UNIX-like systems").

==============================================================================
 Customizing
==============================================================================

To customize the appearance of your site, you may overwrite builtin CSS,
images, etc. by placing the files with the same names to the ololord user
resources location ("/home/user/.ololord/share/ololord" on UNIX-like systems).

For the file/directory structure see the sources (src/lib).

The rules displayed in the home page and boards must be located in the "rules"
subdirectory. Those files must be named like so: "rules_ru.txt",
"rules_en_US.txt" or "rules.txt" (the last name will be used as a default).
Rules displayed in the home page must be placed in the "rules/home" directory,
and rules for the boards must be placed in the "rules/board_name" directory,
where board_name is, well, the board name. The rules form the files located
in the "rules" directory itself will be prepended to the rules on each board
(but not to the rules on the home page).

Custom pages are not yet supported. They will be added in future releases in
a form of plugins.
