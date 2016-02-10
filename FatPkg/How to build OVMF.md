Building [OVMF](http://www.tianocore.org/ovmf/) is fairly easy once you have installed a few pre-requisites. Please note, however, that building OVMF is not required if you are only interested in [running OVMF](How_to_run_OVMF "wikilink"), since we have provided pre-built binaries of OVMF.

Build Pre-requisites
--------------------

-   A edk2 build tree capable of building UEFI images
    -   If you are new to edk2 building then these [getting started](Getting_Started_with_EDK_II#Step-by-step_Instructions "wikilink") instructions may be helpful.
-   An ASL compiler configured in your edk2 build tree
    -   Either the IASL compiler or the Microsoft ASL compiler can be used
        -   For Unix-like operating systems, IASL is the only option:
            -   For newer Linux distributions, you should be able to install iasl via the distribution's package management system. (This [page](Using_EDK_II_with_Native_GCC "wikilink") may help.)
            -   Or, the [Unix-like systems getting started guide](Unix-like_systems "wikilink") includes details for installing IASL as well.
        -   For Windows, you can download pre-built version of IASL compiler from <http://www.acpica.org>.
        -   For Windows, you can also download a pre-built version of the Microsoft ASL compiler from <http://www.acpi.info>.

Choosing which version of OVMF to build
---------------------------------------

First decide which version of OVMF you will build. You can choose to build the IA32 processor architecture and/or the X64 processor architecture. You should take into account the processor architectures which your toolchain is capable of building. Depending upon which you select, you should modify the ACTIVE\_PLATFORM and TARGET\_ARCH in Conf/target.txt.

| ACTIVE\_PLATFORM           | TARGET\_ARCH | PEI code | DXE/UEFI code |
|----------------------------|--------------|----------|---------------|
| OvmfPkg/OvmfPkgIa32.dsc    | IA32         | IA32     | IA32          |
| OvmfPkg/OvmfPkgIa32X64.dsc | IA32 X64     | IA32     | X64           |
| OvmfPkg/OvmfPkgX64.dsc     | X64          | X64      | X64           |

Once you have modified Conf/target.txt, you can run the build command.

    bash$ build

If successful, you should now have a OVMF.Fd file under the Build sub-directory. The exact directory under the Build directory will depend upon the toolchain, dsc and processor architecture.

You can use OVMF.Fd to [run OVMF](How_to_run_OVMF "wikilink").

See Also
--------

-   <https://wiki.ubuntu.com/UEFI-howto>
