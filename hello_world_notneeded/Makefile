# Makefile for the Hello World app on itel i2163 (SC6531E).
#
# Drop this directory (hello/) into a clone of the fpdoom tree, next to
# fptest/ and fpmenu/, then run:
#
#   make TOOLCHAIN=/path/to/arm-buildroot-linux-gnueabi
#
# (or TOOLCHAIN=arm-linux-gnueabi if you installed gcc-arm-linux-gnueabi
# from your distro). Produces hello.bin, which can be sideloaded via
# SD-card boot (rename to fpbin/fpmain.bin) or USB RAM load.

NAME = hello
TWO_STAGE = 0
SYS_EXTRA = sdio microfat sfc
APP_SRCS = hello
APP_OBJS1 = $(APP_SRCS:%=$(OBJDIR)/app/%.o)
APP_OBJS2 =
# The freestanding build has no stack-protector runtime, so disable it.
EXTRA_CFLAGS = -fno-stack-protector
include ../buildinc.make
APP_CFLAGS += -std=c99 -pedantic
APP_CFLAGS += -I$(SYSDIR)
