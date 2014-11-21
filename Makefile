obj-m+=bigmem.o
obj-m+=test.o

.PHONY: userspace_build userspace_clean kernel_build kernel_clean clean build
.PHONY: kernel_test userspace_test
.PHONY: tar
.PHONY: install

build: kernel_build userspace_build

clean: kernel_clean userspace_clean


userspace_build:
	gcc -o libbigmem.so -DUSER_SPACE -fPIC -shared bigmem.c
	gcc -g -DUSER_SPACE -o test -lbigmem test.c
userspace_clean:
	-rm libbigmem.so
	-rm test

kernel_build:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
kernel_clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

tar:
	tar czvf ../libbigmem.tar.gz ../libbigmem --exclude .git/ --exclude .gitignore

install:
	cp libbigmem.so /usr/lib64/
	sed "3a #define USER_SPACE" bigmem.h > /usr/include/bigmem.h
	mkdir -p /lib/modules/$(shell uname -r)/source/include/linux/bc_kern
	cp bigmem.h /lib/modules/$(shell uname -r)/source/include/linux/bc_kern
