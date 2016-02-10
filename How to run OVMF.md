How to run [OVMF](http://www.tianocore.org/ovmf/) with QEMU or KVM

Pre-requisites
--------------

In order to run OVMF with QEMU, you must have QEMU version 0.9.1 or newer installed on your system.

### Get a build of OVMF.fd

The latest pre-built binaries of OVMF are available from the main [OVMF](http://www.tianocore.org/ovmf/) page, or from [OVMF downloads](http://sourceforge.net/projects/edk2/files/OVMF) area. You can also [build OVMF](How_to_build_OVMF "wikilink") if you are interested.

Choose the correct processor architecture
-----------------------------------------

Be sure to align the processor architecture for OVMF with the proper processor archtecture of QEMU.

For the IA32 build of OVMF, there is a little more choice, since the X64 processor is also compatible with IA32. Therefore, with the IA32 build of OVMF, you can use the following commands: qemu, qemu-system-i386 or qemu-system-x86\_64.

For the X64 build of OVMF, however, you can only use the qemu-system-x86\_64 command.

Setup a BIOS directory for OVMF QEMU
------------------------------------

To use OVMF with QEMU, we utilize the -L QEMU command line parameter. This paramter takes a directory path, and QEMU will load the bios.bin from this directory.

Create a directory, and cd to the directory
-------------------------------------------

For example:

    bash$ mkdir ~/run-ovmf
    bash$ cd ~/run-ovmf

Next, copy the OVMF.fd file into this directory, but rename OVMF.fd to bios.bin:

    bash$ cp /path/to/ovmf/OVMF.fd bios.bin

Next, create a directory to use as a hard disk image for QEMU
-------------------------------------------------------------

(QEMU can turn the contents of a directory into a disk image 'on-the-fly'):

    bash$ mkdir hda-contents

Run OVMF with QEMU

Run QEMU using OVMF
-------------------

Here is a sample command:

    bash$ qemu-system-x86_64 -L . -hda fat:hda-contents

If everything goes well, you should see a graphic logo, and then the UEFI shell should start.

Potential issues
----------------

### KVM

Although the latest versions of OVMF should now support the latest versions of KVM, there is still a chance that you might encounter issues when using KVM. If OVMF fails to boot on QEMU (with KVM), try disabling KVM.
