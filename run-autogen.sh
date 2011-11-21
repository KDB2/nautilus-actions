#!/bin/sh

autogen_target=${autogen_target:-normal}
srcdir=$(cd ${0%/*}; pwd)

# a nautilus-actions-x.y may remain after an aborted make distcheck
# such a directory breaks gnome-autogen.sh generation
# so clean it here
for d in $(find . -maxdepth 1 -type d -name 'nautilus-actions-*'); do
	chmod -R u+w $d
	rm -fr $d
done

[ "${autogen_target}" = "normal" ] &&
	exec ${srcdir}/autogen.sh \
		--prefix=$(pwd)/install \
		--sysconfdir=/etc \
		--with-nautilus-extdir=$(pwd)/install/lib/nautilus \
		--disable-schemas-install \
		--disable-deprecated \
		--disable-gtk-doc \
		--disable-html-manuals \
		--disable-pdf-manuals \
		$*

# 'doc' mode: enable deprecated, manuals and gtk-doc
[ "${autogen_target}" = "doc" ] &&
	exec ${srcdir}/autogen.sh \
		--prefix=$(pwd)/install \
		--sysconfdir=/etc \
		--with-nautilus-extdir=$(pwd)/install/lib/nautilus \
		--disable-schemas-install \
		--enable-deprecated \
		--enable-gtk-doc \
		--enable-gtk-doc-pdf \
		--enable-html-manuals \
		--enable-pdf-manuals \
		$*

# 'distcheck' mode: disable deprecated, enable manuals and gtk-doc
[ "${autogen_target}" = "distcheck" ] &&
	exec ${srcdir}/autogen.sh \
		--prefix=$(pwd)/install \
		--sysconfdir=/etc \
		--with-nautilus-extdir=$(pwd)/install/lib/nautilus \
		--disable-schemas-install \
		--disable-deprecated \
		--enable-gtk-doc \
		--enable-gtk-doc-pdf \
		--enable-html-manuals \
		--enable-pdf-manuals \
		$*

# Build with Gtk+ 3 (actually a 2.97.x unstable version)
# installed in ~/.local/jhbuild
#
# Note that building with Gtk 3.0 not only requires that we have a
# Gtk+ 3 available library, but also that all our required libraries
# only depend of Gtk+ 3.0. In our case, we have:
# $ grep gtk+-2.0 /usr/lib/pkgconfig/*
#   libnautilus-extension.pc:Requires: glib-2.0 gio-2.0 gtk+-2.0
#   unique-1.0.pc:Requires: gtk+-2.0
#
[ "${autogen_target}" = "jhbuild" ] &&
	export autogen_prefix=${HOME}/data/jhbuild/run &&
	PKG_CONFIG_PATH=${autogen_prefix}/lib/pkgconfig \
	LD_LIBRARY_PATH=${autogen_prefix}/lib \
		exec ./autogen.sh \
			--prefix=${autogen_prefix} \
			--sysconfdir=/etc \
			--disable-schemas-install \
			$*
