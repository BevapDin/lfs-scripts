#! /bin/bash

# Format of config file (colums):
# 1. col: system (mount|luks|loop)
# 2. col: device (/dev/xxx)
# 3. col: name (/dev/mapper/xxx)
# 4. col: use fstab to mount? (0|1)
# 5. col: key-file or "-"

PATH="$PATH:/sbin:/usr/sbin"

# $1 ... system
# $2 ... device
# return: 0 --> device is open; 1 --> device is not open or error
is_device_open() {
	case "$1" in
		luks)
			[ -e "$2" ]
			return $?
			;;
		loop)
			modprobe loop
			losetup "$2" 1>/dev/null 2>/dev/null
			return $?
			;;
		mount)
			mount | grep -q "$2"
			return $?
			;;
		umount)
			mount | grep -q "$2"
			return $?
			;;
	esac
	return 1
}
# Parameters: same as from config file.
cryptomount() {
	case "$1" in
		luks)
			mapper="/dev/mapper/$3"
			;;
		loop)
			mapper="/dev/loop$3"
			;;
		umount)
			mapper="$3"
			;;
		mount)
			mapper="$3"
			;;
		*)
			echo "$prog: $2: Unknown system: $1" 1>&2
			return 1
			;;
	esac
  
	echo "$prog: $2 --> $mapper [$system]"
	do_fsck=1
	if is_device_open "$1" "$mapper" ; then
		"$logger" "$prog: $2: Yet decrypted: $mapper"
		do_fsck=0
	else
		mount_crypt_device "$1" "$2" "$3" "$4" "$5" "$mapper" || return $?
		case "$1" in
			luks)
				renice 5 `pgrep kcryptd` 1>/dev/null
				;;
		esac
	fi
	if ! mount | grep -q "$mapper" ; then
		if [ "$system" != "loop" ] ; then
			if [ $do_fsck == 1 ] ; then
				"$logger" "$prog: $2: Checking $mapper ..."
				fsck -a -C -T "$mapper" || return 1
			fi
		fi
	fi
	if [ "$4" == 0 ] ; then
		return 0
	fi
	if mount | grep -q "$mapper" ; then
		"$logger" "$prog: $2: Yet mounted: $mapper"
		return 0
	fi
	"$logger" "$prog: $2: Mounting $mapper ..."
	mount "$mapper"
	return $?
}
mount_crypt_device() {
	system="$1"
	dev="$2"

	keyfile="$5"
	mapper="$6"

	# Missing device is silently ignored. This is for removable drives.
	if [ ! -e "$dev" ] ; then
		return 1
	fi

	case $system in
		umount)
			echo "$prog: $dev: Calling umount ..."
			umount "$3"
			return $?
			;;
		mount)
			echo "$prog: $dev: Calling mount ..."
			mount -o ro "$dev" "$3"
			return $?
			;;
		luks)
			# skipped, handled below
			;;
		loop)
			modprobe loop
			echo "$prog: $dev: Calling losetup ..."
			losetup "$mapper" "$dev"
			return $?
			;;
		*)
			echo "Unknown system: $system" 1>&2
			return 1
			;;
	esac

	
	pswpromt=0
	case "$keyfile" in
		-)
			if [ $interactiv == 0 ] ; then
				return 1
			fi
			echo "$prog: $dev: Calling cryptsetup - enter password ..."
			cryptsetup luksOpen "$dev" "$3"
			return $?
			;;
		*.key)
			"$logger" "$prog: $dev: Calling cryptosetup with keyfile ..."
			cryptsetup luksOpen --key-file "$keyfile" "$dev" "$3"
			return $?
			;;
		*.psw)
			"$logger" "$prog: $dev: Calling cryptosetup with keyfile ..."
			cryptsetup luksOpen --key-file - "$dev" "$3" <"$keyfile"
			return $?
			;;
		*)
			echo "Unknown key file type: $keyfile" 1>&2
			return 1
			;;
	esac
}

usage() {
  echo "Mount crypted volumes."
  echo "Usage: $0 [<options>] {<devices>}"
  echo "-c <config-file> ... alternativ config file [$CONF_FILE]."
  echo "-a ... mount all known devices."
  echo "-f ... don't call mount on any encrypted device."
  echo "-q ... (more, not absolute) logger."
  echo "-n ... non-interactiv - don't ask user. Skip automaticly all mount without keyfile."
  echo "If -a is not given only the named devices (can be hdxn or device-name) are mounted."
  exit 1
}

# Configuration-file
CONF_FILE=/etc/crypto.conf
# Program name: used for output.
prog=`basename "$0"`
# More or less output. Can be 'true' (less output) or 'echo' (more output).
logger=echo
# Mount and encrypted all known devices (from config-file).
mount_all=0
# Call the mount prog to mount encryted volumes.
call_mount=1
# Be interactiv.
interactiv=1
# Do work in parallel
parallel=0

if [ -z "$crypt_timeout" ] ; then
	crypt_timeout=120
fi
if [ -z "$crypt_tries" ] ; then
	crypt_tries=2
fi

do_this_device() {
	local device="$1"
	local name="$2"
	shift 2

	if [ "$mount_all" == 1 ] ; then
		return 0
	fi
	for d in "$@" ; do
		if [ "$device" == "$d" -o "$name" == "$d" ] ; then
			return 0
		fi
	done
	return 1
}

while [ "$#" -gt 0 ] ; do
	case "$1" in
		-n)
			interactiv=0
			shift 1
			;;
		-c)
			CONF_FILE="$2"
			shift 2
			;;
		-q)
			logger=true
			shift 1
			;;
		-f)
			call_mount=0
			shift 1
			;;
		-a)
			mount_all=1
			shift 1
			;;
		--help|-h|-\?)
			usage
			;;
		-*)
			echo "Error: Unknown option $1" 1>&2
			exit 1
			;;
		*)
			break
			;;
	esac
done

echo "Called as $0 " `date` " for $CONF_FILE"

if [ ! -r "$CONF_FILE" ] ; then
	echo "Error: Configuration file [$CONF_FILE] not readable." 1>&2
	exit 1
fi

(
	while read -u 4 system device name fstab keyfile rest ; do
		case "$system" in
			\#*|"")
				continue
				;;
		esac
		if [ "$call_mount" == 0 ] ; then
			fstab=0
		fi
		if do_this_device "$device" "$name" "$@" ; then
			cryptomount $system $device $name $fstab "$keyfile"
		fi
	done
) 4<"$CONF_FILE"

exit 0
