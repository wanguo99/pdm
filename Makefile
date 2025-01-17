include $(CURDIR)/config.mk

all: modules

modules:
	$(MAKE) -j$(nproc) -C $(KERNELDIR) M=$(CURDIR) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURDIR) clean

install:
	@cp -a $(CURDIR)/pdm.ko $(DESTDIR)/

uninstall:
	@echo "Uninstalling modules from $(DESTDIR)"
	@rm -rf $(DESTDIR)/pdm.ko

.PHONY: all modules clean install uninstall
