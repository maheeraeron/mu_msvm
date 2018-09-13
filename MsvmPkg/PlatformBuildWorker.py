##
## Script to Build Hyper-V UEFI firmware
##
##
## Copyright Microsoft Corporation, 2018
##
import os, sys
import stat
from optparse import OptionParser
import logging
import subprocess
import shutil
import struct
from datetime import datetime
from datetime import date
import time
from UefiBuild import UefiBuilder



#--------------------------------------------------------------------------------------------------------
# Subclass the UEFI builder and add platform specific functionality.
#
class PlatformBuilder(UefiBuilder):

    def __init__(self, workspace, packagespath, pluginlist, args):
        super(PlatformBuilder, self).__init__(workspace, packagespath, pluginlist, args)

    def SetPlatformEnv(self):
        logging.debug("PlatformBuilder SetPlatformEnv")

        self.env.SetValue("CONF_TEMPLATE_DIR", "MsvmPkg", "Conf template directory hardcoded - temporary and would go away")

        self.env.SetValue("PRODUCT_NAME", "Hyper-V", "Platform Hardcoded")
        self.env.SetValue("TOOL_CHAIN_TAG", "VSLATESTx86xASL", "Platform hardcoded")
        self.env.SetValue("BLD_*_BUILD_UNIT_TESTS", "FALSE", "Unit Test build off by default")
        self.env.SetValue("BLD_*_BUILD_APPS", "FALSE", "App Build off by default")
        self.env.SetValue("BLD_*_SECURE_BOOT_ENABLE", "TRUE", "Support Secure Boot")

        #
        # Build AARCH64 by using BUILD_ARCH=AARCH64 with PlatformBuild.py
        #
        if self.env.GetValue("BUILD_ARCH") == "AARCH64":
            logging.debug("PlatformBuilder building AARCH64")
            self.env.SetValue("ACTIVE_PLATFORM", "MsvmPkg/MsvmPkgAARCH64.dsc", "Platform Hardcoded")
            self.env.SetValue("TARGET_ARCH", "AARCH64", "Platform Hardcoded")
            self.env.SetValue("ARCH", "AARCH64", "Platform hardcoded")
        else:
            logging.debug("PlatformBuilder building X64")
            self.env.SetValue("ACTIVE_PLATFORM", "MsvmPkg/MsvmPkgX64.dsc", "Platform Hardcoded")
            self.env.SetValue("TARGET_ARCH", "X64", "Platform Hardcoded")
            self.env.SetValue("ARCH", "X64", "Platform hardcoded")

        #self.env.SetValue("BLD_*_BUILDID", "72932128", "hardcoded for easy build file")
        self.env.SetValue("BLD_*_BUILDID_STRING", "17.1590.800", "hardcoded for easy build file")    # hack to make FdReport

        self.env.SetValue("LaunchBuildLogProgram", "Notepad", "default - will fail if already set", True)
        self.env.SetValue("LaunchLogOnSuccess", "True", "default - will fail if already set", True)
        self.env.SetValue("LaunchLogOnError", "True", "default - will fail if already set", False)

        return 0

    def SetPlatformEnvAfterTarget(self):
        logging.debug("PlatformBuilder SetPlatformEnvAfterTarget")
        return 0

    def PlatformPostBuild(self):
        return 0


    #------------------------------------------------------------------
    #
    # Method to do stuff pre build.
    # This is part of the build flow.
    # Currently do nothing.
    #
    #------------------------------------------------------------------
    def PlatformPreBuild(self):
        return 0

    #
    # Main Build class supports a few methods of flashing but leaves
    # one option to Platform when FLASH_METHOD = platform.
    #
    # For this platform we don't need custom method
    #
    def PlatformFlashImage(self):
        p = os.path.join(self.env.GetValue("DEBUG_BUILD_OUTPUT_BASE"), self.env.GetValue("ARCH"))
        cmd = os.path.join(p,"secmain.exe")
        ret = self.RunCmd(cmd, workingdir=p)
        return ret