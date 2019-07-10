#      This file is part of the KoraOS project.
#  Copyright (C) 2018  <Fabien Bavent>
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
#  This makefile is more or less generic.
#  The configuration is on `sources.mk`.
# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
topdir ?= $(shell readlink -f $(dir $(word 1,$(MAKEFILE_LIST))))
gendir ?= $(shell pwd)

include $(topdir)/make/global.mk
srcdir := $(topdir)/src

all: bins
install: install-all

CFLAGS += -Wall -Wextra -I$(srcdir)/include -fPIC -ggdb
CFLAGS += -Wno-unused-parameter -fno-builtin
ifeq ($(target_os),kora)
CFLAGS += -Dmain=_main
endif

include $(topdir)/make/build.mk

define util
UTILS+=$(1)
$(1): $(bindir)/$(1)
install-$(1): $(prefix)/bin/$(1)
$(bindir)/$(1): $(srcdir)/$(1).c
	$(S) mkdir -p $$(dir $$@)
	$(Q) echo "  LD  $$@"
	$(V) $(CC) -o $$@ $$< $(CFLAGS)
$(prefix)/bin/$(1): $(bindir)/$(1)
	$(S) mkdir -p $$(dir $$@)
	$(V) $(INSTALL) $$< $$@
endef

$(eval $(call util,basename))
$(eval $(call util,base64))
$(eval $(call util,cat))
$(eval $(call util,false))
$(eval $(call util,ls))
$(eval $(call util,true))
$(eval $(call util,pinky))
$(eval $(call util,cal))

bins: $(UTILS)

install-all: install-basename install-cat install-ls
