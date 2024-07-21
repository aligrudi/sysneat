#!/bin/sh
# Sysneat script

if test "$1" = "up"; then
	/bin/mount -o remount,ro /

	echo 0 >/proc/sys/kernel/ctrl-alt-del

	/sbin/fsck -A -T -C -a
	if test "$?" -gt 1; then
		echo
		echo "FSCK FAILED!"
		echo
		echo "+ starting a shell for examination"
		echo "+ system reboots after logging out"
		echo "+ to remount for writing: mount -n -o remount,rw /"
		/bin/sh
		/bin/mount -o remount,ro /
		/bin/umount -a -r
		kill -INT 1
		exit 0
	fi

	/bin/mount -o remount,rw /
	/bin/mount -a -O no_netdev

	/sbin/hwclock --hctosys
	/bin/dmesg >/var/log/boot

	/etc/rc.local
fi

if test "$1" = "down"; then
	/sbin/hwclock --systohc

	if ! /bin/mount -o remount,ro /; then
		/bin/sh
	fi
	if ! /bin/umount -a -d -r; then
		/bin/sh
	fi
fi
