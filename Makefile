#GCC = arm-linux-gnueabihf-
KERN_DIR ?= /home/awe/RaspberryPi/modules/lib/modules/3.12.26+/build
PROJECT=second

obj-m += $(PROJECT).o
PWD := $(shell pwd)

default:
	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -C $(KERN_DIR) M=$(PWD)
clean:
	rm -rf *.o *.ko *~core *.order *.symvers .depend .*.cmd *.mod.c *.tmp_versions
