# Umicom OS packaging only. All runtime source remains Framework-owned.
# Sammy Hegab, Umicom Foundation. Licence: MIT.
UMICOM_FRAMEWORK_PROBE_VERSION = local
UMICOM_FRAMEWORK_PROBE_SITE = $(BR2_EXTERNAL_UMICOM_OS_PATH)/../../framework
UMICOM_FRAMEWORK_PROBE_SITE_METHOD = local
UMICOM_FRAMEWORK_PROBE_LICENSE = MIT
UMICOM_FRAMEWORK_PROBE_LICENSE_FILES = examples/os_foundation/LICENSE

define UMICOM_FRAMEWORK_PROBE_BUILD_CMDS
	$(TARGET_CC) $(TARGET_CFLAGS) -std=c23 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -I$(@D)/include \
		$(@D)/examples/os_foundation/probe.c $(@D)/src/platform/boot_report.c $(@D)/src/data/data_server.c \
		$(TARGET_LDFLAGS) -static -o $(@D)/umicom-framework-probe
endef

define UMICOM_FRAMEWORK_PROBE_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/umicom-framework-probe $(TARGET_DIR)/usr/libexec/umicom-framework-probe
endef

$(eval $(generic-package))
