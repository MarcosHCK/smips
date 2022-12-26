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

noinst_DATA=\
  $(VOID)

#
# Binaries and libraries
# - sources
#

smips_SOURCES=\
  smips.c \
  $(VOID)
smips_CFLAGS=\
  $(GLIB_CFLAGS) \
  $(LUA_CFLAGS) \
  $(VOID)
smips_LDFLAGS=\
  $(GLIB_LIBS) \
  $(LUA_LIBS) \
  $(VOID)
