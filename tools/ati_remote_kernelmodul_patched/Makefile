obj-m += ati_remote.o
KVERSION = $(shell uname -r)
PWD = $(shell pwd)
all:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean
patch:
	patch ati_remote.c ati_remote.c_all_keys_and_keychange.patch
install:
	install udev/40-atiremote.rules /etc/udev/rules.d/
	install modprobe.d/atiremote.conf /etc/modprobe.d/
	install ati_remote.ko /lib/modules/$(KVERSION)/kernel/drivers/input/misc/
	depmod
	service udev reload
	depmod
	modprobe ati_remote
unload:
	rmmod ati_remote
load:
	modprobe ati_remote
