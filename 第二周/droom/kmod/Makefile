# kmod/Makefile (内核模块编译)
ifneq ($(KERNELRELEASE),)
    obj-m := droomkmod.o
    droomkmod-objs := src/kmod.o
else
    KERNELDIR ?= /lib/modules/$(shell uname -r)/build
    PWD := $(shell pwd)

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

install:
	install -m 644 droomkmod.ko /lib/modules/$(shell uname -r)/extra
	depmod -a

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
endif
