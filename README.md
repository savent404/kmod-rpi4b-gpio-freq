# About this project

This project is for test raspberry 4B max gpio output's frequency, using a general kernel driver model.

# Steps

- `make` or `KSOURCE=<...> make`(if cross-build)
- `mv gpio-sample.dtbo to /boot/overlay`
- `echo "dtoverlay=gpio-sample" >> /boot/config.txt`
- `reboot 0`
- `insmod gpio-sample.ko delay_mode=<0|1|2>`
- watch gpio21's output
