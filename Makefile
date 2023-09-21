.PHONY: all install copy clean


KSOURCE ?= /lib/modules/$(shell uname -r)/build

MODULE_NAME = gpio-sample
obj-m = gpio-sample.o

all:
	make -C $(KSOURCE) M=$(PWD) modules
	dtc -@ -I dts -O dtb -o gpio-sample.dtbo gpio-sample.dts
install: all
	make -C $(KSOURCE) M=$(PWD) modules_install
copy: all
	scp gpio-sample.dtbo gpio-sample.ko rpi:/tmp
clean:
	make -C $(KSOURCE) M=$(PWD) clean
	rm -f *.dtbo
