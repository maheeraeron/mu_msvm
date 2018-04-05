##
## Script to Build Hyper-V UEFI firmware
##
##
## Copyright Microsoft Corporation, 2018
##
import os, sys

#
#==========================================================================
# PLATFORM BUILD ENVIRONMENT CONFIGURATION
#
SCRIPT_PATH = os.path.dirname(os.path.abspath(__file__))
WORKSPACE_PATH = os.path.dirname(SCRIPT_PATH)
REQUIRED_REPOS = ('SM_UDK', "SM_UDK_INTERNAL")
PROJECT_SCOPE = ('hyperv',)

MODULE_PKGS = ('SM_UDK', "SM_UDK_INTERNAL", "MsvmPkg")
MODULE_PKG_PATHS = ";".join(os.path.join(WORKSPACE_PATH, pkg_name) for pkg_name in MODULE_PKGS)
#
#==========================================================================
#
    
# Smallest 'main' possible. Please don't add unnecessary code.
if __name__ == '__main__':
  # Do a quick check to see whether we're trying to setup.
  if "--SETUP" in (arg.upper() for arg in sys.argv):
    sys.path.append(WORKSPACE_PATH)
    import bootstrap_repo
    bootstrap_repo.bootstrap()

  # If we're not trying to setup, we should assume that we have
  # the correct prerequisites and are good to go.
  try:
    sys.path.append(os.path.join(WORKSPACE_PATH, 'SM_UDK', 'UefiBuild'))
    import CommonBuildEntry
  except ImportError:
    raise RuntimeError("Environment is not in a state to build! Please run '--SETUP'.")

  # Now that we have access to the entry, hand off to the common code.
  CommonBuildEntry.build_entry(SCRIPT_PATH, WORKSPACE_PATH, REQUIRED_REPOS, PROJECT_SCOPE, MODULE_PKGS, MODULE_PKG_PATHS)
