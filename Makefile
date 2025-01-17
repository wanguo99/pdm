include $(CURDIR)/config.mk

all: modules

modules:
	$(MAKE) -j$(nproc) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

install:
	$(MAKE) -j$(nproc) -C $(KDIR) M=$(PWD) INSTALL_MOD_PATH=$(DESTDIR) modules_install 

uninstall:
	@echo "Uninstalling modules from $(DESTDIR)"
	@rm -rf $(DESTDIR)/pdm.ko

.PHONY: all modules clean install uninstall
