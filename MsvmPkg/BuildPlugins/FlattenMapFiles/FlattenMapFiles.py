## @file FlattenMapFiles.py
# Plugin to support copying all Map files to 1 directory.
# This makes debugging easier when source level debugging is not available.
#
##
# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
# All rights reserved.
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 1. Redistributions of source code must retain the above copyright notice,
# this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
##
###
from edk2toolext.environment.plugintypes.uefi_build_plugin import IUefiBuildPlugin
import logging
import shutil
import os

class FlattenMapFiles(IUefiBuildPlugin):

    def do_post_build(self, thebuilder):
        #Path to Build output
        BuildPath = thebuilder.env.GetValue("BUILD_OUTPUT_BASE")
        #Path to where the Map files will be stored
        MapFilesPath = os.path.join(BuildPath, "MAP")

        IgnoreMapFiles = []  #make lower case

        try:
            if not os.path.isdir(MapFilesPath):
                os.mkdir(MapFilesPath)
        except:
            logging.critical("Error making Map Files directory")

        logging.critical("Copying Map Files to flat directory")
        for dirpath, dirnames, filenames in os.walk(BuildPath):
            if MapFilesPath in dirpath:
                continue
            for filename in filenames:
                fnl = filename.strip().lower()
                if(fnl.endswith(".map")):
                    if(any(e for e in IgnoreMapFiles if e in fnl)):
                        # too much info. logging.debug("Flatten  - Ignore Map files: %s" % filename)
                        pass
                    else:
                        shutil.copy(os.path.join(dirpath, filename), os.path.join(MapFilesPath, filename))
        return 0