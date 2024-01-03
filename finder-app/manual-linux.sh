#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.1.10
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

if [ ! -d "${OUTDIR}" ]; then
    echo "Creating ${OUTDIR} failed"
    exit 1
fi

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # TODO: Add your kernel build steps here
    make ARCH=arm64 CROSS_COMPILE=${CROSS_COMPILE} mrproper
    make ARCH=arm64 CROSS_COMPILE=${CROSS_COMPILE} defconfig
    make -j4 ARCH=arm64 CROSS_COMPILE=${CROSS_COMPILE} all

fi

echo "Adding the Image in outdir"

echo "Creating the staging directory for the root filesystem"
pwd
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories
echo "pwd is ${PWD}"
if [ ! -d "${OUTDIR}/rootfs1" ]
then
    mkdir rootfs1
fi
cp ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image .
cd rootfs1
mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir -p usr/bin usr/lib usr/sbin
mkdir -p var/log

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
    make distclean
    make defconfig
else
    cd busybox
fi

# TODO: Make and install busybox
# make distclean
# make defconfig
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
make CONFIG_PREFIX=${OUTDIR}/rootfs1 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install

echo "Library dependencies"
pwd
${CROSS_COMPILE}readelf -a ${OUTDIR}/rootfs1/bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a ${OUTDIR}/rootfs1/bin/busybox | grep "Shared library"

# TODO: Add library dependencies to rootfs
cp /usr/aarch64-linux-gnu/lib/ld-linux-aarch64.so.1 ${OUTDIR}/rootfs1/lib
cp /usr/aarch64-linux-gnu/lib/ld-linux-aarch64.so.1 ${OUTDIR}/rootfs1/lib64
cp /usr/aarch64-linux-gnu/lib/libm.so.6 ${OUTDIR}/rootfs1/lib64
cp /usr/aarch64-linux-gnu/lib/libresolv.so.2 ${OUTDIR}/rootfs1/lib64
cp /usr/aarch64-linux-gnu/lib/libc.so.6 ${OUTDIR}/rootfs1/lib64
cp /usr/aarch64-linux-gnu/lib/libm.so.6 ${OUTDIR}/rootfs1/lib
cp /usr/aarch64-linux-gnu/lib/libresolv.so.2 ${OUTDIR}/rootfs1/lib
cp /usr/aarch64-linux-gnu/lib/libc.so.6 ${OUTDIR}/rootfs1/lib
# TODO: Make device nodes
sudo mknod -m 666 ${OUTDIR}/rootfs1/dev/null c 1 3
sudo mknod -m 666 ${OUTDIR}/rootfs1/dev/console c 5 1
# TODO: Clean and build the writer utility
cd "$FINDER_APP_DIR"

make clean
make CROSS_COMPILE=${CROSS_COMPILE}
# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
cp writer ${OUTDIR}/rootfs1/home
cp autorun-qemu.sh ${OUTDIR}/rootfs1/home
cp start-qemu-app.sh ${OUTDIR}/rootfs1/home
cp start-qemu-terminal.sh ${OUTDIR}/rootfs1/home
cp finder.sh ${OUTDIR}/rootfs1/home
sudo cp -r conf/ ${OUTDIR}/rootfs1/home/conf/
cp finder-test.sh ${OUTDIR}/rootfs1/home

# TODO: Chown the root directory
cd "$OUTDIR"

sudo chown -R root:root rootfs1

# TODO: Create initramfs.cpio.gz
cd "$OUTDIR/rootfs1"
find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio
cd "$OUTDIR"
gzip -f initramfs.cpio
