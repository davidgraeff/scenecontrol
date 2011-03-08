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
	cp 05-atiremote.conf /usr/share/X11/xorg.conf.d/
	cp 40-atiremote.rules /etc/udev/rules.d/
