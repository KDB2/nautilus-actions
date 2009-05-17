# Nautilus Actions
#
# Copyright (C) 2005 The GNOME Foundation
# Copyright (C) 2006, 2007, 2008 Frederic Ruaudel and others (see AUTHORS)
# Copyright (C) 2009 Pierre Wieser and others (see AUTHORS)
#
# This Program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This Program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public
# License along with this Library; see the file COPYING.  If not,
# write to the Free Software Foundation, Inc., 59 Temple Place,
# Suite 330, Boston, MA 02111-1307, USA.
#
# Authors:
#   Frederic Ruaudel <grumz@grumz.net>
#   Rodrigo Moya <rodrigo@gnome-db.org>
#   Pierre Wieser <pwieser@trychlos.org>
#   and many others (see AUTHORS)
#
# pwi 2009-05-16 created from configure.in

AC_PREREQ([2.53])

# Caution: 1.11 is unstable/development version
AC_INIT([nautilus-actions],[1.11],[maintainer@nautilus-actions.org])
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE

AC_CONFIG_SRCDIR([plugin/nautilus-actions.c])

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES([
	Makefile
	icons/Makefile
	icons/16x16/Makefile
	icons/22x22/Makefile
	icons/32x32/Makefile
	icons/48x48/Makefile
	icons/scalable/Makefile
	libnautilus-actions/Makefile
	m4/Makefile
	nact/Makefile
	plugin/Makefile
	po/Makefile.in
	utils/Makefile
])

AC_CONFIG_MACRO_DIR([m4])

# pwi 2009-05-15 disabled as useless or badly explained
#AC_SUBST(ACLOCAL_AMFLAGS, "$ACLOCAL_FLAGS")

# don't agree with maintainer mode use
# see http://www.gnu.org/software/automake/manual/automake.html#index-AM_005fMAINTAINER_005fMODE-1001
# but gnome-autogen.sh forces its usage and gnome_common_init requires it
AM_MAINTAINER_MODE

# check for compiler characteristics and options
AC_PROG_CC
AC_PROG_GCC_TRADITIONAL
AC_STDC_HEADERS
AM_DISABLE_STATIC

# other traditional tools
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Gnome stuff
GNOME_COMMON_INIT
GNOME_MAINTAINER_MODE_DEFINES
NACT_GNOME_COMPILE_WARNINGS([error],[-ansi])
AC_SUBST([AM_CPPFLAGS],["${AM_CPPFLAGS} ${DISABLE_DEPRECATED}"])
AC_SUBST([AM_CFLAGS],["${AM_CFLAGS} ${WARN_CFLAGS}"])

# libtool
AM_PROG_LIBTOOL

# localization
# note that this same version is also required for in autogen.sh
IT_PROG_INTLTOOL([0.35.5])
GETTEXT_PACKAGE=${PACKAGE}
AC_SUBST([GETTEXT_PACKAGE])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE],["${GETTEXT_PACKAGE}"],[gettext package])
AM_GLIB_GNU_GETTEXT

# GConf support
AC_PATH_PROG([GCONFTOOL],[gconftool-2])
AM_GCONF_SOURCE_2

# these are the version as distributed with CentOS 4.x
# - Gnome 2.8
# - glib 2.4.7
# - autoconf 2.59
# - automake 1.9
# - intltool 0.31
#
# note that requiring through pkg-config the version number of the
# nautilus-extension library actually returns the version number of
# nautilus itself (e.g. 2.24.2) instead of the library version info
# (1.1.0) ; for the usual final user, this is very analog to requiring
# a whole gnome version
GLIB_REQUIRED=2.4.0
GTK_REQUIRED=2.4.0
GLADE_REQUIRED=2.4.0
GNOME_REQUIRED=2.7.0
NAUTILUS_EXTENSION=2.8.0
GCONF_REQUIRED=2.8.0
LIBXML_REQUIRED=2.6.0
GOBJECT_REQUIRED=2.4.0

#AC_SUBST(GLIB_REQUIRED)
#AC_SUBST(GTK_REQUIRED)
#AC_SUBST(GLADE_REQUIRED)

AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
if test "${PKG_CONFIG}" = "no"; then
	AC_MSG_ERROR([You need to install pkg-config])
fi

PKG_CHECK_MODULES([NAUTILUS_ACTIONS], \
	glib-2.0				>= ${GLIB_REQUIRED}			\
	gthread-2.0				>= ${GLIB_REQUIRED}			\
	gmodule-2.0				>= ${GLIB_REQUIRED}			\
	gtk+-2.0				>= ${GTK_REQUIRED}			\
	libglade-2.0			>= ${GLADE_REQUIRED}		\
	libgnome-2.0			>= ${GNOME_REQUIRED}		\
	libgnomeui-2.0			>= ${GNOME_REQUIRED}		\
	gconf-2.0				>= ${GCONF_REQUIRED}		\
	libnautilus-extension	>= ${NAUTILUS_EXTENSION}	\
	uuid)
AC_SUBST([NAUTILUS_ACTIONS_CFLAGS])
AC_SUBST([NAUTILUS_ACTIONS_LIBS])

# when working in a test environment, nautilus is typically installed in
# a non-standard location ; lets specify this here
NACT_NAUTILUS_EXTDIR

# building or not the commandline tools
NACT_COMMANDLINE_TOOLS

# Check for menu update function
AC_CHECK_LIB(nautilus-extension, nautilus_menu_item_new)
AC_CHECK_FUNCS(nautilus_menu_provider_emit_items_updated_signal)

# pwi 2009-05-15 disabled as useless or badly explained
#if test "x${exec_prefix}" = "xNONE"; then
#	if test "x${prefix}" = "xNONE"; then
#		if test "x${bindir}" = "xNONE"; then
#			BINDIR=`eval echo $ac_default_prefix/bin`
#		else
#			BINDIR=`eval echo $bindir`
#		fi
#	else
#		BINDIR=`eval echo $prefix/bin`
#	fi
#else
#	BINDIR=`eval echo $bindir`
#fi
#AC_SUBST(BINDIR)
#	
#if test "x${prefix}" = "xNONE"; then
#  DATADIR="${ac_default_prefix}/${DATADIRNAME}" 
#  LOCALEDIR="${DATADIR}/locale"
#  AC_DEFINE_UNQUOTED(LOCALEDIR, "${LOCALEDIR}", [Locale dir])
#else
#  DATADIR="${prefix}/${DATADIRNAME}"
#  LOCALEDIR="${DATADIR}/locale"
#  AC_DEFINE_UNQUOTED(LOCALEDIR, "${LOCALEDIR}", [Locale dir])
#fi
#AC_SUBST(DATADIR)
#AC_SUBST(LOCALEDIR)

AC_DEFINE_UNQUOTED(NAUTILUS_ACTIONS_CONFIG_VERSION, "2.0", [Version of the configuration format])
AC_DEFINE_UNQUOTED(NAUTILUS_ACTIONS_CONFIG_GCONF_BASEDIR, "/apps/nautilus-actions", [GConf base dir where all config will be stored])

AC_OUTPUT