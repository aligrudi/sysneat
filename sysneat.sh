#!/bin/sh
# Sysneat script

SYSUSR="/etc/sysneat.user"

# system is up
sysneat_up() {
	/bin/mount -o remount,ro /
	echo 0 >/proc/sys/kernel/ctrl-alt-del

	/sbin/fsck -A -T -C1 -p
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

	test -x $SYSUSR && $SYSUSR up
}

# system is going down
sysneat_down() {
	test -x $SYSUSR && $SYSUSR down
}

# system is down
sysneat_halt() {
	/sbin/hwclock --systohc

	if ! /bin/umount -a -d -r; then
		/bin/sh
	fi
}

sysneat_"$@"
