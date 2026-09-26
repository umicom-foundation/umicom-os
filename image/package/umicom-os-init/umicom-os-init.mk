# Umicom OS | Sammy Hegab, Umicom Foundation | MIT
UMICOM_OS_INIT_VERSION = local
UMICOM_OS_INIT_SITE = $(BR2_EXTERNAL_UMICOM_OS_PATH)/../boot/foundation
UMICOM_OS_INIT_SITE_METHOD = local
UMICOM_OS_INIT_LICENSE = MIT
UMICOM_OS_INIT_LICENSE_FILES = LICENSE

define UMICOM_OS_INIT_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) CC="$(TARGET_CC)" CFLAGS="$(TARGET_CFLAGS)" LDFLAGS="$(TARGET_LDFLAGS) -static"
endef

define UMICOM_OS_INIT_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/umicom-os-init $(TARGET_DIR)/init
	$(INSTALL) -D -m 0755 $(@D)/umicom-platform-check $(TARGET_DIR)/usr/libexec/umicom-platform-check
endef

$(eval $(generic-package))
