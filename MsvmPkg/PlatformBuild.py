##
## Script to Build nt32 emulator UEFI firmware
##
##
## Copyright Microsoft Corporation, 2017
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

#get script path
sp = os.path.dirname(os.path.realpath(sys.argv[0]))

#get workspace path
ws = os.path.dirname(sp) #UEFI workspace will be parent
pp = os.path.join(ws, "SM_UDK")
pp += ";" + os.path.join(ws, "MsvmPkg")

#setup python path for build modules
sys.path.append(os.path.join(ws,  "SM_UDK", "MsBaseTools", "PythonTools", "Build"))

from UefiBuild import UefiBuilder



#--------------------------------------------------------------------------------------------------------
# Subclass the UEFI builder and add platform specific functionality.
#
class PlatformBuilder(UefiBuilder):
    def __init__(self, workspace, packagespath, args):


        UefiBuilder.__init__(self, workspace, packagespath, args)

    def SetPlatformEnv(self):
        logging.debug("PlatformBuilder SetPlatformEnv")

        self.env.SetValue("ACTIVE_PLATFORM", "MsvmPkg/MsvmPkgX64.dsc", "Platform Hardcoded")
        self.env.SetValue("PRODUCT_NAME", "Hyper-V", "Platform Hardcoded")
        self.env.SetValue("TARGET_ARCH", "X64", "Platform Hardcoded")
        self.env.SetValue("ARCH", "X64", "Platform hardcoded")
        self.env.SetValue("BLD_*_BUILD_UNIT_TESTS", "FALSE", "Unit Test build off by default")
        self.env.SetValue("BLD_*_BUILD_APPS", "FALSE", "App Build off by default")
        self.env.SetValue("BLD_*_SECURE_BOOT_ENABLE", "TRUE", "Support Secure Boot")

        #self.env.SetValue("BLD_*_BUILDID", "72932128", "hardcoded for easy build file")
        #self.env.SetValue("BLD_*_BUILDID_STRING", "17.1590.800", "hardcoded for easy build file")

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


#END OF CLASS

    
#--------------------------------------------------------------------------------------------------------
#
# main script function.  Setup logging and init the platform builder and go
#
if __name__ == '__main__':
    
    #setup main console as logger
    logger = logging.getLogger('')
    logger.setLevel(logging.DEBUG)
    formatter = logging.Formatter("%(levelname)s - %(message)s")
    console = logging.StreamHandler()

    #Setup the main console logger differently if VSMODE is on.  
    # This allows more debug messages out
    # 

    levelset = False
    for a in sys.argv:
        if (a == "--VSMODE"):
            console.setLevel(logging.DEBUG)
            console.setFormatter(logging.Formatter("%(message)s"))
            levelset = True
            sys.argv.remove(a)

    if(not levelset):
        console.setLevel(logging.CRITICAL)
        console.setFormatter(formatter)
    logger.addHandler(console)
    logfile = os.path.join(ws, "Build", "BUILDLOG.TXT")
    if(not os.path.isdir(os.path.dirname(logfile))):
        os.makedirs(os.path.dirname(logfile))

    filelogger = logging.FileHandler(filename=(logfile), mode='w')
    filelogger.setLevel(logging.DEBUG)
    filelogger.setFormatter(formatter)
    logging.getLogger('').addHandler(filelogger)
    logging.info("Log Started: " + datetime.strftime(datetime.now(), "%A, %B %d, %Y %I:%M%p" ))
    logging.info("Running Python version: " + str(sys.version_info))
    PB = PlatformBuilder(ws, pp, sys.argv)
    retcode = PB.Go()

    if(retcode != 0):
        logging.critical("Error")
        logging.critical("Log file at " + logfile)
    else:
        logging.critical("Success")

    #get all vars needed as we can't do any logging after shutdown otherwise our log is cleared.  
    #Log viewer
    ep = PB.env.GetValue("LaunchBuildLogProgram")
    LogOnSuccess = PB.env.GetValue("LaunchLogOnSuccess")
    LogOnError = PB.env.GetValue("LaunchLogOnError")
    
    #end logging
    logging.shutdown()
    #no more logging

    if(ep != None):
        cmd = ep + " " + logfile

    #
    # Conditionally launch the shell to show build log
    #
    #
    if( ((retcode != 0) and (LogOnError.upper() == "TRUE")) or (LogOnSuccess.upper() == "TRUE")):
        subprocess.Popen(cmd, shell=True)
        
    sys.exit(retcode)

 
