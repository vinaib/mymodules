obj-m = my_module.o

K_PREBUILT = /lib/modules/$(shell uname -r)/build

KSRC=/home/vinait/workplace/kernel.org

all default: modules

modules help clean:
	make -C $(KSRC) M=$(PWD) $@

