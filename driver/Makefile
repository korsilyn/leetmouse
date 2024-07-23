KERNELRELEASE ?= $(shell uname -r)
KERNEL_DIR ?= /lib/modules/$(KERNELRELEASE)/build
PWD := $(shell pwd)

obj-m += leetmouse.o
leetmouse-objs := usbmouse.o accel.o util.o

ccflags-y += -mhard-float -mpreferred-stack-boundary=4

all:
	cp -n $(PWD)/config.sample.h $(PWD)/config.h
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

install:
	cp -n $(PWD)/config.sample.h $(PWD)/config.h
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules_install

clean:
	rm $(PWD)/config.h
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean

insmod:
	sudo insmod leetmouse.ko

rmmod:
	sudo rmmod leetmouse.ko

