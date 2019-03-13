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
NAME=kora-utils
VERSION=0.0-$(GIT)

CFLAGS += -Wall -Wextra -Wno-unused-parameter -Wno-char-subscripts
CFLAGS += -Wno-multichar
CFLAGS += -fno-builtin -fPIC
CFLAGS += -D_DATE_=\"'$(DATE)'\" -D_VTAG_=\"'$(VERSION)'\"
CFLAGS += -ggdb


COV_FLAGS += --coverage -fprofile-arcs -ftest-coverage


# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
# We define compilation modes and associated flags
std_CFLAGS += $(CFLAGS)
std_CFLAGS += -I$(topdir)/include

std_CFLAGS += -I$(topdir)/../kernel/include
std_CFLAGS += -I$(topdir)/../kernel/arch/$(target_arch)/include
std_CFLAGS += -I$(topdir)/../kernel/os/$(target_os)

$(eval $(call ccpl,std))


# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
define utilo
$(1)_src-y += $(srcdir)/$(1).c
DV_UTILS += $(bindir)/$(1)
endef
define util
$(eval $(call utilo,$1))
$(eval $(call link,$1,std))
endef

$(eval $(call util,basename))
$(eval $(call util,cat))
$(eval $(call util,ls))


