VARCH=arm
VCC=arm-linux-gnueabihf-

obj-m := char_demo.o

char_demo-objs := module.o

# arm kernel
KOBJ=/home/vinai/projects/bbb/out/linux

all default: modules

modules help clean:
	make ARCH=$(VARCH) CROSS_COMPILE=$(VCC) -C $(KOBJ) M=$(PWD) $@
	cp char_demo.ko /var/lib/tftpboot
