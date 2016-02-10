GitHub Help
===========

GitHub (https://help.github.com/index.html) provides step-by-step
instructions for user registration and basic features supported by
GitHub.

**GitHub EDK II Project Repositories**
--------------------------------------

The EDK II project repository is available at
<https://github.com/tianocore/edk2>.

Prebuilt Windows tools are available at
<https://github.com/tianocore/edk2-BaseTools-win32>.

Please note that FatPkg is not included in the EDKII project repository,
it is an independent repository which can be found at
<https://github.com/tianocore/edk2-FatPkg>.

Content that is not released under an accepted open source license can
be found at <https://github.com/tianocore/edk2-non-osi>.

**How to Setup the EDK II Tree**
--------------------------------

**Note:** Some of the following examples use the Multiple Workspace
feature to configure the EDK II BaseTools. More information on the
Multiple Workspace feature can be found at the following location.

-   [Multiple\_Workspace](Multiple_Workspace "wikilink")

### **Linux Support**

#### For EDKII project developers:

-   Clone the EDK II project repository
    -   git clone <https://github.com/tianocore/edk2>
-   Change to the edk2 directory
-   Build the tools
    -   make -C BaseTools
-   Run the edksetup.sh script
    -   . edksetup.sh

When the above steps are done, you can work in the edk2 directory for
code development.

#### For FatPkg developers:

-   Create a workspace directory
-   Change to the workspace directory
-   Clone the EDK II project repository
    -   git clone <https://github.com/tianocore/edk2>
-   Clone the edk2-FatPkg repository to “FatPkg”
    -   git clone <https://github.com/tianocore/edk2-FatPkg> FatPkg
-   Build the tools
    -   make -C edk2/BaseTools
-   Set environment variables
    -   WORKSPACE – The workspace directory created above
    -   PACKAGES\_PATH – Set it to \$WORKSPACE/edk2

Example:

```
export WORKSPACE=/Sample/Path
export PACKAGES_PATH=$WORKSPACE/edk2
```

-   Run the edksetup.sh script
    -   . edk2/edksetup.sh

When the above steps are done, the directory structure will look like:

```
 Sample
   └───Path (WORKSPACE)
      ├───edk2
      └───FatPkg
```

### **Windows Support**

#### For EDKII project developers:

-   Create a workspace directory
-   Change to the workspace directory
-   Clone the EDK II project repository
    -   git clone <https://github.com/tianocore/edk2>
-   Clone the edk2-BaseTools-win32 repository
    -   git clone <https://github.com/tianocore/edk2-BaseTools-win32>
-   Set environment variables:
    -   EDK\_TOOLS\_BIN – Set it as the edk2-BaseTools-win32 directory

Example:

`   set EDK_TOOLS_BIN=c:\efi\test\edk2-BaseTools-win32`

-   Change to the edk2 directory
-   Run the edksetup.bat script

When the above steps are done, the directory structure will look like:

```
 efi
   └───test (WORKSPACE)
      ├───edk2
      └───edk2-BaseTools-win32
```

#### For FatPkg developers:

-   Create a workspace directory
-   Change to the workspace directory
-   Clone the EDK II project repository
    -   git clone <https://github.com/tianocore/edk2>
-   Clone the edk2-BaseTools-win32 repository
    -   git clone <https://github.com/tianocore/edk2-BaseTools-win32>
-   Clone the edk2-FatPkg repository to “FatPkg”
    -   git clone <https://github.com/tianocore/edk2-FatPkg> FatPkg
-   Set environment variables:
    -   WORKSPACE – The workspace directory created above
    -   PACKAGES\_PATH – Set it to %WORKSPACE%\\edk2
    -   EDK\_TOOLS\_BIN – Set it to %WORKSPACE%\\edk2-BaseTools-win32

Example:

```
set WORKSPACE=c:\efi\test
set PACKAGES_PATH=%WORKSPACE%\edk2
set EDK_TOOLS_BIN=%WORKSPACE%\edk2-BaseTools-win32
```

-   Run the edksetup.bat script
    -   edk2\\edksetup.bat

When the above steps are done, the directory structure will look like:

```
efi
  └───test (WORKSPACE)
     ├───edk2
     ├───edk2-BaseTools-win32
     └───FatPkg
```

Please keep in mind that the EDK II project, FatPkg and
edk2-BaseTools-win32 are independent Git repositories. Each of these
repositories must be updated individually.

**Development Process for the EDK II Project**
----------------------------------------------

The developer process for the EDK II project is:

1.  Setup the EDK II tree if you do not have one
2.  Create and checkout a topic branch for new feature or bug fix
3.  Make changes in the working tree
4.  Break up working tree changes into independent commits that do not
    break *git bisect*
    -   [Commit-Partitioning](Commit-Partitioning "wikilink")
5.  Follow the commit message template given below when writing commit
    messages
    -   [Commit-Message-Format](Commit-Message-Format "wikilink")
6.  Use the ‘PatchCheck.py’ script under ‘edk2\\BaseTools\\Scripts’
    directory to verify the commits are correctly formatted
7.  Update the master branch (pull or fetch/merge)
8.  Rebase the topic branch onto master branch
9. Create patch (serial) to the [[edk2-devel]] mailing list or upload
   the topic branch to your forked EDK II project and send the URL
   and branch name of the fork to the above mailing list
    -   Using *git send-email* is the preferred method for posting
        patches to the mailing list
10. Modify local commits based on the review feedbacks and repeat steps
    3 to 9

The maintainer process for the EDK II project is:

1.  Determine if a patch has met the review requirements for the package
2.  Update the master branch
3.  Create and checkout an integration branch
4.  Integrate reviewed commits on the integration branch
5.  Rebase commit message to include any reviewed-by or other
    attributions
6.  Push changes to the EDKII project repository
    -   Pushing the integration branch directly to origin/master is
        preferred

**See Also**
------------

-   [Commit-Message-Format](Commit-Message-Format "wikilink")
-   [Commit-Partitioning](Commit-Partitioning "wikilink")
-   [Code-Style](Code-Style "wikilink")
