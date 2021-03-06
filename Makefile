obj-m+=bigmem.o
obj-m+=test.o

.PHONY: userspace_build userspace_clean kernel_build kernel_clean clean build
.PHONY: kernel_test userspace_test
.PHONY: tar

build: kernel_build userspace_build

clean: kernel_clean userspace_clean


userspace_build:
	gcc -o libbigmem.so -DUSER_SPACE -fPIC -shared bigmem.c
	cp libbigmem.so /usr/lib64/
	gcc -g -DUSER_SPACE -o test -lbigmem test.c
userspace_clean:
	-rm libbigmem.so
	-rm test

kernel_build:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
kernel_clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

tar:
	tar czvf ../libbigmem.tar.gz ../libbigmem
