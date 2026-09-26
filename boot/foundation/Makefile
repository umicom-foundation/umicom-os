# Umicom OS minimal boot; Sammy Hegab, Umicom Foundation. Licence: MIT.
CC ?= cc
CFLAGS ?= -O2
WARNINGS = -std=c23 -Wall -Wextra -Wpedantic -Wconversion -Wshadow
all: umicom-os-init umicom-platform-check
umicom-os-init: init.c config.c supervisor.c report.c console.c boot.h
	$(CC) $(CFLAGS) $(WARNINGS) init.c config.c supervisor.c report.c console.c $(LDFLAGS) -o $@
umicom-platform-check: platform_check.c
	$(CC) $(CFLAGS) $(WARNINGS) platform_check.c $(LDFLAGS) -o $@
.PHONY: all
