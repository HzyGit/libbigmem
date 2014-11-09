obj-m+=bigmem.o

.PHONY: userspace_build userspace_clean kernel_build kernel_clean clean build

build: kernel_build userspace_build

clean: kernel_clean userspace_clean

userspace_build:
	gcc -o libbigmem.so -DUSER_SPACE -fPIC -shared bigmem.c
usespace_clean:
	-rm libbigmem.so
kernel_build:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
kernel_clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean


