KVERS = $(shell uname -r)

obj-m += hook_module.o

build: hook_module test
 
hook_module:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) modules
test:
	gcc -o test test.c
 
clean: 
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) clean
	rm ./test