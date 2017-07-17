kmod = lego
ifneq ($(KERNELRELEASE),)
	obj-m := $(kmod).o
else

KDIR ?= /lib/modules/$$(uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$$PWD modules

clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean

.PHONY  : clean
endif