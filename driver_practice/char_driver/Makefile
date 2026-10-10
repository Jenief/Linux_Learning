obj-m += char_driver.o

ccflags-y += -g
KDIR?=$(HOME)/linux-6.6.21
PWD :=$(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
