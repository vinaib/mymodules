#comment or uncomment the following line to disable/enable debugging
DEBUG=y

#single file syntax
obj-m := scull_char.o

# multiple files syntax
#obj-m := module.o

ifeq ($(DEBUG),y) 
	DBGFLAGS = -O -g -DSCULL_DEBUG  
else
	DBGFLAGS = -O2
endif

EXTRA_CFLAGS += $(DBGFLAGS)

scull_char-objs := my_module.o

K_PREBUILT = /lib/modules/$(shell uname -r)/build

KSRC=/home/vinait/workplace/kernel.org

all default: modules

modules help clean:
	make -C $(KSRC) M=$(PWD) $@

