# $Id$

# -- I N I T --------------------------------------------------

AC_INIT([Synfig Studio],[0.61.09],[http://synfig.org/Bugs],[synfigstudio])
AC_REVISION

AC_CONFIG_AUX_DIR(config)
AC_CONFIG_HEADERS([config.h])
AC_CANONICAL_HOST
dnl AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE

API_VERSION=0.0

# -- V A R I A B L E S ----------------------------------------

SVN_REPOSITORY=https://synfig.svn.sourceforge.net/svnroot/synfig/synfig-studio

# -- P R O G R A M S ------------------------------------------

AC_PROG_CC
AC_GNU_SOURCE
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_CXXCPP
AC_PROG_INSTALL

AC_LANG([C++])


AC_ARG_DEBUG

case "$debug" in
	yes)
		AC_DEFINE(SHOW_EXTRA_INFO,[],[Show extra info in the about dialog])
	;;
esac

AC_ARG_OPTIMIZATION
AC_ARG_WARNINGS
AC_ARG_PROFILE_ARCS
AC_ARG_BRANCH_PROBABILITIES
AC_ARG_PROFILING

AC_ARG_ENABLE(g5opt,[
  --enable-g5opt           enable optimizations specific to G5 proc],[
    G5OPTFLAGS="-mpowerpc-gpopt -mno-multiple -mno-update -mcpu=970  -mtune=970 -falign-loops=16 -falign-functions=16"
],
[
    G5OPTFLAGS=
])

AC_WIN32_QUIRKS

AC_LIBTOOL_WIN32_DLL
AC_LIBTOOL_DLOPEN
AC_DISABLE_STATIC
AC_ENABLE_SHARED
AC_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)
AC_LIBTOOL_PATCH

## FIXME: add i18n for .desktop and other non-compiled files
## AM_GLIB_GNU_GETTEXT([external])
## AC_PROG_INTLTOOL(0.11)
ALL_LINGUAS="ca fr es ru"
AM_GNU_GETTEXT([external])
# This is here so autoreconf will run autopoint
AM_GNU_GETTEXT_VERSION([0.15])

case "$host" in
	*mingw* | *cygwin*)
		AM_CONDITIONAL(WIN32_PKG, true)
		AM_CONDITIONAL(MACOSX_PKG, false)
	;;
	*darwin*)
		AM_CONDITIONAL(WIN32_PKG, false)
		AM_CONDITIONAL(MACOSX_PKG, true)
		AC_DEFINE(USE_OPEN_FOR_URLS,[],[On MacOS X "open <URL>" works, on Linux it tries to open a virtual console.] )
	;;
	*)
		AM_CONDITIONAL(WIN32_PKG, false)
		AM_CONDITIONAL(MACOSX_PKG, false)
	;;
esac
# -- L I B R A R I E S ----------------------------------------

PKG_CHECK_MODULES(GTKMM, gtkmm-2.4 gthread-2.0,[
	AC_MSG_CHECKING([for Gtk::AboutDialog::set_program_name])
	synfigstudio_save_cxxflags="$CXXFLAGS"
	CXXFLAGS="$GTKMM_CFLAGS $CXXFLAGS"
	AC_COMPILE_IFELSE(
		[
			AC_LANG_PROGRAM([
				[#include <gtkmm/aboutdialog.h>]
			],[
				[Gtk::AboutDialog about; about.set_program_name(PACKAGE_NAME)]
			])
		],[
			AC_MSG_RESULT([yes])
			AC_DEFINE(HAVE_GTK_ABOUTDIALOG_SET_PROGRAM_NAME,[],[Define if GTKmm provides Gtk::AboutDialog::set_program_name()] )
			AM_CONDITIONAL(HAVE_GTK_ABOUTDIALOG_SET_PROGRAM_NAME,true)
		],[
			AC_MSG_RESULT([no])
			AM_CONDITIONAL(HAVE_GTK_ABOUTDIALOG_SET_PROGRAM_NAME,false)
		]
	)
	CXXFLAGS="$synfigstudio_save_cxxflags"
	AC_MSG_CHECKING([for Gtk::AboutDialog::set_wrap_license])
	synfigstudio_save_cxxflags="$CXXFLAGS"
	CXXFLAGS="$GTKMM_CFLAGS $CXXFLAGS"
	AC_COMPILE_IFELSE(
		[
			AC_LANG_PROGRAM([
				[#include <gtkmm/aboutdialog.h>]
			],[
				[Gtk::AboutDialog about; about.set_wrap_license(true)]
			])
		],[
			AC_MSG_RESULT([yes])
			AC_DEFINE(HAVE_GTK_ABOUTDIALOG_SET_WRAP_LICENSE,[],[Define if GTKmm provides Gtk::AboutDialog::set_wrap_license()] )
			AM_CONDITIONAL(HAVE_GTK_ABOUTDIALOG_SET_WRAP_LICENSE,true)
		],[
			AC_MSG_RESULT([no])
			AM_CONDITIONAL(HAVE_GTK_ABOUTDIALOG_SET_WRAP_LICENSE,false)
		]
	)
	CXXFLAGS="$synfigstudio_save_cxxflags"
],[
	AC_MSG_ERROR([ ** Gtkmm-2.4 Is a required library, aborting.])
])
AC_SUBST(GTKMM_CFLAGS)
AC_SUBST(GTKMM_LIBS)

PKG_CHECK_MODULES(SYNFIG, [synfig >= 0.61.09] [ETL >= 0.04.12] sigc++-2.0,,[
	AC_MSG_ERROR([ ** Unable to set up dependent libraries (synfig >= 0.61.09, ETL >= 0.04.12)])
])
AC_SUBST(SYNFIG_CFLAGS)
AC_SUBST(SYNFIG_LIBS)

windowsys="gtkmm"
AC_DEFINE(STUDIO_WINSYS_H,"gtkmm/winsys.h",[blah])
AM_CONDITIONAL(WINDOWSYS_GTKMM,true)

AC_ARG_WITH(userdir,[
  --with-userdir=(dirname)         Relative path to user directory],[
],[
case "$host" in
	*mingw* | *cygwin*)
		with_userdir="Synfig"
	;;
	*darwin*)
		with_userdir="Library/Synfig"
	;;
	*)
		with_userdir=".synfig"
	;;
esac
])
AC_DEFINE_UNQUOTED(SYNFIG_USER_APP_DIR,"$with_userdir",[Synfig User Data Directory])

# --- FMOD CHECK -------------
AC_ARG_WITH(libfmod,[
  --with-libfmod         Enable support for FMOD],[
],[
	AC_CHECK_LIB(fmod, FSOUND_Init,[
		with_libfmod="yes"
	],[
		with_libfmod="no"
	])
])
if test $with_libfmod = "yes" ; then {
	AM_CONDITIONAL(WITH_FMOD,true)
	FMOD_LIBS="/usr/lib/libfmod.so"
	AC_DEFINE(WITH_FMOD,"I'm here damnit",[blah])
} else {
	AM_CONDITIONAL(WITH_FMOD,false)
	FMOD_LIBS=
} ; fi

# Check for unordered_map or hash_map headers
AC_MSG_CHECKING([for unordered_map<int,int>::const_iterator])
AC_COMPILE_IFELSE([
	AC_LANG_PROGRAM([
		[#include <tr1/unordered_map>]
	],[
		[std::tr1::unordered_map<int,int>::const_iterator iter;]
	])],[
	AC_MSG_RESULT([yes])
	AC_DEFINE([HASH_MAP_CLASS],[HASH_MAP_NAMESPACE::unordered_map],[The template name for unordered maps.])
	AC_DEFINE([HASH_MAP_H],[<tr1/unordered_map>],[The header file to include for unordered maps.])
	AC_DEFINE([HASH_MAP_NAMESPACE],[std::tr1],[The namespace for unordered maps.])
	AC_DEFINE([FUNCTIONAL_H],[<tr1/functional>],[The header file for hash<T>.])
	AC_DEFINE([FUNCTIONAL_HASH_ON_STRING],[1],[Define to 1 if hash<T> needs to use T=String rather than T=char* to hash strings.])
],[
	AC_MSG_RESULT([no])
	AC_CHECK_HEADERS([ext/hash_map],[
		AC_DEFINE([HASH_MAP_CLASS],[HASH_MAP_NAMESPACE::hash_map],[The template name for unordered maps.])
		AC_DEFINE([HASH_MAP_H],[<ext/hash_map>],[The header file to include for unordered maps.])
		AC_DEFINE([HASH_MAP_NAMESPACE],[__gnu_cxx],[The namespace for unordered maps.])
		AC_DEFINE([FUNCTIONAL_H],[<ext/functional>],[The header file for hash<T>.])
	],[
		AC_MSG_ERROR([C++ compiler does not seem to support unordered containers])
	])
])

# Check for unordered_set or hash_set headers
AC_MSG_CHECKING([for unordered_set<int,int>::const_iterator])
AC_COMPILE_IFELSE([
	AC_LANG_PROGRAM([
		[#include <tr1/unordered_set>]
	],[
		[std::tr1::unordered_set<int,int>::const_iterator iter;]
	])],[
	AC_MSG_RESULT([yes])
	AC_DEFINE([HASH_SET_CLASS],[HASH_SET_NAMESPACE::unordered_set],[The template name for unordered sets.])
	AC_DEFINE([HASH_SET_H],[<tr1/unordered_set>],[The header file to include for unordered sets.])
	AC_DEFINE([HASH_SET_NAMESPACE],[std::tr1],[The namespace for unordered sets.])
],[
	AC_MSG_RESULT([no])
	AC_CHECK_HEADERS([ext/hash_set],[
		AC_DEFINE([HASH_SET_CLASS],[HASH_SET_NAMESPACE::hash_set],[The template name for unordered sets.])
		AC_DEFINE([HASH_SET_H],[<ext/hash_set>],[The header file to include for unordered sets.])
		AC_DEFINE([HASH_SET_NAMESPACE],[__gnu_cxx],[The namespace for unordered sets.])
	],[
		AC_MSG_ERROR([C++ compiler does not seem to support unordered containers])
	])
])

# -- H E A D E R S --------------------------------------------

AC_CHECK_HEADERS([unistd.h signal.h fcntl.h])
AC_CHECK_HEADERS([sys/types.h sys/wait.h sys/stat.h sys/time.h sys/resource.h sys/errno.h])

# -- T Y P E S & S T R U C T S --------------------------------

# -- F U N C T I O N S ----------------------------------------

AC_CHECK_FUNCS([fork])
AC_CHECK_FUNCS([kill])
AC_CHECK_FUNCS([pipe])
AC_CHECK_FUNCS([setpriority mkfifo stat]) 

# -- A R G U M E N T S ----------------------------------------

# -- O U T P U T ----------------------------------------------

AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(CXXFLAGS)
AC_SUBST(LDFLAGS)
AC_SUBST(SVN_REPOSITORY)

AC_SUBST(API_VERSION)

# fmod stuff...
AC_SUBST(FMOD_LIBS)

imageext=png
AC_SUBST(imageext)

imagedir="${datadir}/pixmaps/synfigstudio"
AC_SUBST(imagedir)

AC_CONFIG_FILES([Makefile
build_tools/Makefile
doxygen.cfg
po/Makefile.in
synfigstudio.desktop
synfigstudio.mime
synfigstudio.keys
synfigstudio.xml
synfigstudio-thumbnailer.schemas
src/Makefile
src/gtkmm/Makefile
src/synfigapp/Makefile
images/Makefile
win32inst.nsi
pkg-info/macosx/synfig-studio.info
])
AC_OUTPUT

# -- S U M M A R Y --------------------------------------------

echo "
$PACKAGE_NAME v.$VERSION
Configuration Summary
- - - - - -

Install Prefix -------------------> $prefix
Host Platform --------------------> $host
Icon Format ----------------------> $imageext
Debug Mode -----------------------> $debug
Profiling Mode -------------------> $profiling
Optimizations --------------------> $optimization
Window System --------------------> $windowsys
FMOD Enabled ---------------------> $with_libfmod

"'$'"CXX ------------------------------> '$CXX'
"'$'"CXXFLAGS -------------------------> '$CXXFLAGS'
"'$'"LDFLAGS --------------------------> '$LDFLAGS'
"'$'"SYNFIG_CFLAGS ---------------------> '$SYNFIG_CFLAGS'
"'$'"SYNFIG_LIBS -----------------------> '$SYNFIG_LIBS'
"'$'"GTKMM_CFLAGS ---------------------> '$GTKMM_CFLAGS'
"'$'"GTKMM_LIBS -----------------------> '$GTKMM_LIBS'
"