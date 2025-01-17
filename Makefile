include $(CURDIR)/config.mk

all: modules

modules:
	$(MAKE) -j$(nproc) -C $(KERNELDIR) M=$(CURDIR) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURDIR) clean
	$(MAKE) -C $(CURDIR)/test clean

install:
	@cp -a $(CURDIR)/pdm.ko $(DESTDIR)/
	@cp -a $(CURDIR)/test/pdm_test $(DESTDIR)/

uninstall:
	@echo "Uninstalling modules from $(DESTDIR)"
	@rm -rf $(DESTDIR)/pdm.ko
	@rm -rf $(DESTDIR)/pdm_test

.PHONY: all modules clean install uninstall test test-clean
