obj-m+=bigmem.o

.PHONY: kernel_build kernel_clean clean build

build: kernel_build

clean: kernel_clean

kernel_build:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
kernel_clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean


