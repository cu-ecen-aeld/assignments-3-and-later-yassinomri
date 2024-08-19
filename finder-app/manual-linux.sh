#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

#set -e
#set -u

# Default output directory
DEFAULT_OUTDIR=/tmp/aeld

# Kernel repository and version
KERNEL_REPO=https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git
KERNEL_VERSION=v5.1.10
BUSYBOX_VERSION=1_33_1
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

# Take the output directory from the argument or use default
if [ $# -lt 1 ]
then
    OUTDIR=${DEFAULT_OUTDIR}
    echo "Using default directory ${OUTDIR} for output"
else
    OUTDIR=$1
    echo "Using passed directory ${OUTDIR} for output"
fi

# Create the output directory if it doesn't exist
if ! mkdir -p "${OUTDIR}"; then
    echo "Failed to create directory ${OUTDIR}"
    exit 1
fi

# Clone the kernel source if it doesn't exist
if [ ! -d "${OUTDIR}/linux" ]; then
    echo "Cloning kernel repository..."
    git clone ${KERNEL_REPO} --depth 1 --branch ${KERNEL_VERSION} ${OUTDIR}/linux
fi

cd "${OUTDIR}/linux"

# Apply the kernel patch if not already applied
if [ ! -f ".patch_applied" ]; then
    echo "Applying patch..."
    git apply --reject /home/yass/e33a814e772cdc36436c8c188d8c42d019fda639.patch
    touch .patch_applied
fi

# Clean, configure, and build the kernel
if [ ! -f "${OUTDIR}/Image" ]; then
    echo "Cleaning up the kernel source tree..."
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper

    echo "Configuring the kernel..."
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig

    echo "Building the kernel..."
    make -j$(nproc) ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}

    # Copy the kernel image to the output directory
    echo "Copying the kernel image..."
    cp arch/${ARCH}/boot/Image ${OUTDIR}/
fi

# Create the root filesystem
if [ ! -d "${OUTDIR}/rootfs" ]; then
    echo "Creating the staging directory for the root filesystem"
    mkdir -p ${OUTDIR}/rootfs/{bin,lib,lib64,home,proc,sys,dev,tmp}

    # Clone and build BusyBox
    if [ ! -d "${OUTDIR}/busybox" ]; then
        echo "Cloning BusyBox repository..."
        git clone git://busybox.net/busybox.git ${OUTDIR}/busybox
        cd ${OUTDIR}/busybox
        git checkout ${BUSYBOX_VERSION}
    else
        cd ${OUTDIR}/busybox
    fi

    echo "Building BusyBox..."
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} -j$(nproc)
    echo "Installing BusyBox..."
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install

    # Add library dependencies to rootfs
    echo "Copying library dependencies to rootfs..."
    mkdir -p ${OUTDIR}/rootfs/lib
    ${CROSS_COMPILE}ldd ${OUTDIR}/rootfs/bin/busybox | grep "=>" | awk '{print $3}' | xargs -I {} cp -v {} ${OUTDIR}/rootfs/lib/

    # Create device nodes
    echo "Creating device nodes..."
    sudo mknod -m 666 ${OUTDIR}/rootfs/dev/console c 5 1
    sudo mknod -m 666 ${OUTDIR}/rootfs/dev/null c 1 3
    sudo mknod -m 666 ${OUTDIR}/rootfs/dev/tty c 5 0

    # Copy the writer utility
    echo "Copying writer utility..."
    cd ${FINDER_APP_DIR}/writer
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} clean
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
    cp writer ${OUTDIR}/rootfs/home/

    # Copy the finder related scripts and executables
    echo "Copying finder related scripts and executables to /home directory..."
    cp ${FINDER_APP_DIR}/finder.sh ${OUTDIR}/rootfs/home/
    cp ${FINDER_APP_DIR}/conf/username.txt ${OUTDIR}/rootfs/home/
    cp ${FINDER_APP_DIR}/conf/assignment.txt ${OUTDIR}/rootfs/home/
    cp ${FINDER_APP_DIR}/finder-test.sh ${OUTDIR}/rootfs/home/

    # Modify finder-test.sh to reference conf/assignment.txt
    sed -i 's|../conf/assignment.txt|conf/assignment.txt|' ${OUTDIR}/rootfs/home/finder-test.sh

    # Copy autorun-qemu.sh script
    echo "Copying autorun-qemu.sh script..."
    cp ${FINDER_APP_DIR}/autorun-qemu.sh ${OUTDIR}/rootfs/home/
fi

# Create initramfs.cpio.gz if not already created
if [ ! -f "${OUTDIR}/initramfs.cpio.gz" ]; then
    echo "Creating initramfs.cpio.gz..."
    cd ${OUTDIR}/rootfs
    find . | cpio -H newc -o | gzip > ${OUTDIR}/initramfs.cpio.gz
fi

echo "Script completed successfully."

