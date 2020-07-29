VARCH=arm
VCC=arm-linux-gnueabihf-

obj-m := print_defs.o

print_defs-objs := my_module.o

# x86 kernel source
KSRC=/home/vinait/workplace/kernel.org

# arm kernel
KOBJ=/home/vinai/projects/bbb/out/linux

all default: modules copy

#modules help clean:\
	make -C $(KSRC)  M=$(PWD) $@

modules help clean:
	make ARCH=$(VARCH) CROSS_COMPILE=$(VCC) -C $(KOBJ) M=$(PWD) $@

copy:	
	cp print_defs.ko /var/lib/tftpboot
