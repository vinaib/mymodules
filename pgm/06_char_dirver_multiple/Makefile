# make host  - to compile for host
# make hostclean - clean
# sudo insmod char_demo.ko
# sudo rmmmod char_demo

# make - to cross compile for bbb
# make clean
# make help
VARCH=arm
VCC=arm-linux-gnueabihf-

obj-m := char_demo.o

char_demo-objs := module.o

ccflags-y:=-DDYNAMIC_DEVICE_CREATION 

# arm kernel
#KOBJ=/home/vinai/projects/bbb/out/linux
KOBJ=/home/vinai/projects/bbb/linux/linux

# host path
HOSTPATH=/lib/modules/$(shell uname -r)/build

all default: modules

modules:
	make ARCH=$(VARCH) CROSS_COMPILE=$(VCC) -C $(KOBJ) M=$(PWD) $@
	cp char_demo.ko /var/lib/tftpboot

help clean:
	make ARCH=$(VARCH) CROSS_COMPILE=$(VCC) -C $(KOBJ) M=$(PWD) $@


host:
	make -C $(HOSTPATH) M=$(PWD) modules

hostclean:
	make -C $(HOSTPATH) M=$(PWD) clean

