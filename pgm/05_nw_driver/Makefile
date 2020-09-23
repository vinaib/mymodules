BARCH := arm
PREFIX := arm-linux-gnueabihf-

obj-m := net_module.o

net_module-objs := net_driver.o

KOBJ := /home/vinai/projects/bbb/out/linux

all default: modules

modules clean help:
	make ARCH=$(BARCH) CROSS_COMPILE=$(PREFIX) -C $(KOBJ) M=$(PWD) $@

install:
	cp net_module.ko /var/lib/tftpboot/
