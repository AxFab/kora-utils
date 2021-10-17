#      This file is part of the KoraOS project.
#  Copyright (C) 2015-2021  <Fabien Bavent>
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU Affero General Public License as
#  published by the Free Software Foundation, either version 3 of the
#  License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Affero General Public License for more details.
#
#  You should have received a copy of the GNU Affero General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
topdir ?= $(shell readlink -f $(dir $(word 1,$(MAKEFILE_LIST))))
gendir ?= $(shell pwd)

include $(topdir)/make/global.mk
srcdir := $(topdir)/src

all: bins
install: install-all

CFLAGS ?= -Wall -Wextra -Wno-unused-parameter -ggdb

ifneq ($(sysdir),)
CFLAGS_l += -I$(sysdir)/include
LFLAGS_l += -L$(sysdir)/lib
endif

include $(topdir)/make/build.mk

define util
UTILS+=$(1)
$(1): $(bindir)/$(1)
install-$(1): $(prefix)/bin/$(1)
$(bindir)/$(1): $(srcdir)/$(2)/$(1).c $(srcdir)/utils.h
	$(S) mkdir -p $$(dir $$@)
	$(Q) echo "    LD  $$@"
	$(V) $(CC) -o $$@ $$< $(CFLAGS_$(3)) $(LFLAGS_$(3))

$(prefix)/bin/$(1): $(bindir)/$(1)
	$(S) mkdir -p $$(dir $$@)
	$(V) $(INSTALL) $$< $$@
endef

# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
CFLAGS_u += $(CFLAGS) -I$(topdir)/include -fPIC -fno-builtin $(CFLAGS_l)
CMDS := $(shell basename -s .c -a $(wildcard $(srcdir)/*.c))
$(foreach cmd,$(CMDS),$(eval $(call util,$(cmd),,u)))

CFLAGS_z += $(CFLAGS_u)
LFLAGS_z += $(LFLAGS_l) -lz
OTHS := $(shell basename -s .c -a $(wildcard $(srcdir)/zl/*.c))
$(foreach cmd,$(OTHS),$(eval $(call util,$(cmd),zl,z)))

CFLAGS_g += $(CFLAGS_u)
LFLAGS_g += $(LFLAGS_l) -lgfx -lpng -lm -lz
APPS := $(shell basename -s .c -a $(wildcard $(srcdir)/ui/*.c))
$(foreach cmd,$(APPS),$(eval $(call util,$(cmd),ui,g)))

# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

bins: $(UTILS)

install-all: $(patsubst %,install-%,$(CMDS)) \
	$(patsubst %,install-%,$(OTHS)) \
	$(patsubst %,install-%,$(APPS))
