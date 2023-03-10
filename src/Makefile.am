# Copyright 2021-2025 MarcosHCK
# This file is part of SMIPS Assembler.
#
# SMIPS Assembler is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# SMIPS Assembler is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with SMIPS Assembler. If not, see <http://www.gnu.org/licenses/>.
#

VOID=

#
# Binaries and libraries
# - declare
#

bin_PROGRAMS=\
	smips \
	$(VOID)

noinst_PROGRAMS=\
	luac \
	$(VOID)

noinst_LTLIBRARIES=\
	libluacmpt.la \
	$(VOID)

noinst_HEADERS=\
	bank.h \
	inst.h \
	insts.h \
	load.h \
	log.h \
	luacmpt.h \
	option.h \
	options.h \
	tag.h \
	tags.h \
	$(VOID)

#
# Binaries and libraries
# - sources
#

luac_SOURCES=\
	luac.c \
	$(VOID)
luac_CFLAGS=\
	$(GIO_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(LUA_CFLAGS) \
	-D__SMIPS_SOURCE__ \
	$(VOID)
luac_LDADD=\
	libluacmpt.la \
	$(VOID)
luac_LDFLAGS=\
	$(GIO_LIBS) \
	$(GLIB_LIBS) \
	$(LUA_LIBS) \
	$(VOID)

smips_SOURCES=\
	bank.c \
	banks.c \
	bundle.c \
	inst.c \
	insts.c \
	load.c \
	log.c \
	option.c \
	options.c \
	smips.c \
	tag.c \
	tags.c \
	utils.c \
	$(VOID)
smips_CFLAGS=\
	$(GIO_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GMODULE_CFLAGS) \
	$(LUA_CFLAGS) \
	-D__SMIPS_SOURCE__ \
	$(VOID)
smips_LDADD=\
	libluacmpt.la \
	$(VOID)
smips_LDFLAGS=\
	$(GIO_LIBS) \
	$(GLIB_LIBS) \
	$(GMODULE_LIBS) \
	$(LUA_LIBS) \
	$(VOID)

libluacmpt_la_SOURCES=\
	luacmpt.c \
	$(VOID)
libluacmpt_la_CFLAGS=\
	$(LUA_CFLAGS) \
	$(VOID)
libluacmpt_la_LIBADD=\
	$(LUA_LIBS) \
	$(VOID)

smips_LUCS=\
	feed.luc \
	isa.luc \
	process.luc \
	smips.luc \
	splitters.luc \
	unit.luc \
	vector.luc \
	$(VOID)

bundle.c: $(smips_LUCS)
bundle.h: $(smips_LUCS)
$(smips_LUCS): luac

#
# Extensions
# - .lua -> .luc
# - .stringlist -> .c
# - .gresources.xml -> .c
# - .gresources.xml -> .h
#

SUFFIXES=.lua .luc .stringlist .gresources.xml

.lua.luc:
	./luac -o $@ $<

.stringlist.c:
	$(GPERF) --output-file $@ -PCI $<

.gresources.xml.c:
	$(GLIB_COMPILE_RESOURCES) --target=$@ --generate-source $<

.gresources.xml.h:
	$(GLIB_COMPILE_RESOURCES) --target=$@ --generate-header $<

CLEANFILES=\
	bundle.c \
	bundle.h \
	*.luc \
	$(VOID)
