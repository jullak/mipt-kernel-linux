obj-m += driver.o
driver-objs += phonebook_list.o phonebook_driver.o


kbuild:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD)

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
