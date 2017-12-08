<#
.SYNOPSIS
    Pushes a build of the firmware as a new vPack.

.DESCRIPTION
    This script simplifies the pushing of a new vpack containing the firmware image
    and related files like symbols.

.PARAMETER IgnoreBranchName

    This switch parameter will ignore the GitBranchName in the vpack.JSON file and
    won't fail with an error that your current branch doesn't match what is in the
    vpack.JSON file.

.PARAMETER DryRun

    This switch parameter will cause the script to do everything it normally does
    but won't actually do the "vpack push".  Combined with the -Verbose switch
    parameter this is useful to see the vpack command line to verify the script
    will do what you expect.

.PARAMETER IgnoreBranch

    This switch parameter will cause the script to not check if the current
    git branch matches the branch in the JSON file.

.PARAMETER IgnoreUncommittedFiles

    This switch parameter will cause the script to not check if there are
    uncommitted files in the git repository.

.INPUTS

    This script expects a file named vpack.JSON to exist in %WORKSPACE%\MsvmPkg.
    The JSON file configures the scenario and influences the behavior of this
    script. See the notes for details.

.NOTES

    The JSON input file is shown here and the individual elements are described
    below:

    {
        "vpack":
        {
            "Major": 1,
            "Minor": 0,
            "Patch": 0,
            "Prerelease": "",
            "VersionIncrementType": "Patch",
            "BaseName": "hyperv.uefi",
            "AppendArchToName": true,
            "ExpirationTimeInHours": 0
        },
        "meta":
        {
            "GitBranchName": "official/rs4",
            "AppendLatestCommitHash": true
        }
    }

    vpack.Major

    The vpack major version integer. This becomes the /Major: parameter on the
    vpack command line.

    vpack.Minor

    The vpack minor version integer. This becomes the /Minor: parameter on the
    vpack command line.

    vpack.Patch

    The vpack patch version integer. This becomes the /Patch: parameter on the
    vpack command line.

    vpack.Prerelease

    The vpack prerelease version string. This becomes the /Prerelease: parameter on the
    vpack command line.

    vpack.VersionIncrementType

    The vpack version increment type. One of [ "None" | "Major" | "Minor" | "Patch" ].
    this becomes the /VersionIncrementType: parameter on the vpack command line.

    vpack.BaseName

    Ths is vpack base name. This becomes the /Basename: parameter on the vpack command line.

    vpack.AppendArchToName

    This adds the current %WORKSPACE% firmware architecture to the specified vpack.BaseName.
    For example, "hyperv.uefi" will become "hyperv.uefi.x64" or "hyperv.uefi.aarch64".

    vpack.ExpirationTimeInHours

    This causes the /ExpirationTime: parameter to be specified in the vpack command line.
    The current time in UTC will be incremented by the ExpirationTimeInHours and then
    formatted for the vpack command line syntax.

    meta.GitBranchName

    The script will error out if the current git branch name doesn't match this string.
    This provides a check that only the release branch will be pushed as a vpack. The
    check can be overridden on with the -IgnoreBranchName script switch parameter.

    meta.AppendLastCommitHash

    When this is true the script will provide the latest git commit hash as the
    /Metadata: parameter on the vpack command line.  This will result in vpack versions
    like the following: hyperv.uefi.x64.1.0.0+774a8bde19f3f618f5da25edc62ca2ab88be5c31.
    This records in the version the commit has from which the firmware image was built.

    Scenarios

    1) Create a new vpack version of the firmware from the current release branch.

        Clean build the firmware (build cleanall, build)

        In the JSON file the Major, Minor, and Patch values should be the base
        version for this release branch. E.g., RS3 is 0.0.0, RS4 is 1.0.0.
        The VersionIncrementType should be "Patch".

        Run the script.

    2) Create first new vpack version of the firmware for a new release branch.

        Clean build the firmware (build cleanall, build)

        In the JSON file the Major, Minor, and Patch values should be the base
        version for this new release branch. You probably want to increment Major.
        Set the branch name appropriately. The VersionIncrementType can stay "Patch".

        Run the script.
#>


[CmdletBinding()]
param(
    [switch]
    $IgnoreBranchName,

    [switch]
    $DryRun,

    [switch]
    $IgnoreUncommittedFiles
    )

###################################################################################################

function Parse-Edk2TargetFile
{
    param(
        [Parameter(Mandatory=$true)]
        [string]
        [ValidateNotNullOrEmpty()]
        $TargetFilePath,

        [Parameter(Mandatory=$true)]
        [hashtable]
        $Config
    )
    Write-Verbose "EDK2 Target File: $TargetFilePath"
    foreach ($line in Get-Content $TargetFilePath)
    {
        # skip comments and blank lines
        if (-not ($line.StartsWith('#') -or $line -match '^\s*$'))
        {
           $tokens = $line.split('=')
           $Config.Add( $tokens[0].Trim(' '), $tokens[1].Trim(' ').Replace('/','\') )
        }
    }
    return $Config
}

###################################################################################################

function Parse-Edk2PlatformFile
{
    param(
        [Parameter(Mandatory=$true)]
        [string]
        [ValidateNotNullOrEmpty()]
        $TargetFilePath,

        [Parameter(Mandatory=$true)]
        [hashtable]
        $Config
    )
    Write-Verbose "EDK2 Platform File: $TargetFilePath"
    foreach ($line in Get-Content $TargetFilePath)
    {
        # skip comments and blank lines
        if (-not ($line.StartsWith('#') -or $line -match '^\s*$'))
        {
            # use regex to match PLATFORM_NAME, OUTPUT_DIRECTORY and FLASH_DEFINITION
            if ($line -imatch '^\s*PLATFORM_NAME\s*=\s*(\S+).*$')
            {
                $Config.Add("PLATFORM_NAME", $matches[1])
            }
            if ($line -imatch '^\s*OUTPUT_DIRECTORY\s*=\s*(\S+).*$')
            {
                $Config.Add("OUTPUT_DIRECTORY", $matches[1].Replace('/','\'))
            }
            if ($line -imatch '^\s*FLASH_DEFINITION\s*=\s*(\S+).*$')
            {
                $Config.Add("FLASH_DEFINITION", $matches[1].Replace('/','\'))
            }
        }
    }
    return $Config
}

###################################################################################################

function Parse-Edk2FlashDefinitionFile
{
    param(
        [Parameter(Mandatory=$true)]
        [string]
        [ValidateNotNullOrEmpty()]
        $TargetFilePath,

        [Parameter(Mandatory=$true)]
        [hashtable]
        $Config
    )
    Write-Verbose "FlashDefinitionFile: $TargetFilePath"
    foreach ($line in Get-Content $TargetFilePath)
    {
        # skip comments and blank lines
        if (-not ($line.StartsWith('#') -or $line -match '^\s*$'))
        {
            # use regex to match FD name
            if ($line -imatch '^\[FD\.(.+)\]')
            {
                $Config.Add("FD_FILENAME", $matches[1]+".fd")
            }
        }
    }
    return $Config
}

###################################################################################################

function Get-Edk2Config
{
    param(
        [Parameter(Mandatory=$true)]
        [ValidateNotNullOrEmpty()]
        [string]
        $Workspace
     )

    $config = @{"WORKSPACE" = $Workspace}
    $config = Parse-Edk2TargetFile `
              -TargetFilePath ("{0}\conf\target.txt" -f $Workspace) `
              -Config $config
    $config = Parse-Edk2PlatformFile `
              -TargetFilePath ("{0}\{1}" -f $config.WORKSPACE, $config.ACTIVE_PLATFORM) `
              -Config $config
    $config = Parse-Edk2FlashDefinitionFile `
              -TargetFilePath ("{0}\{1}" -f $config.WORKSPACE, $config.FLASH_DEFINITION) `
              -Config $config
    Write-Verbose "EDK2 Configuration:"
    $config | Format-Table | out-string -Stream | Write-Verbose
    return $config
}

###################################################################################################

function Get-VPackConfig
{
    param(
        [Parameter(Mandatory=$true)]
        [ValidateNotNullOrEmpty()]
        [string]
        $Workspace
    )
    $vpackConfig = Get-Content -Path ($Workspace + "\msvmpkg\vpack.json") | ConvertFrom-Json
    Write-Verbose "vPack Configuration:"
    $vpackConfig | Format-List | out-string -Stream | Write-Verbose
    return $vpackConfig
}


###################################################################################################

function Get-PdbFiles
{
    Param(
        [Parameter(Mandatory=$true)]
        [hashtable]
        $Config
    )
    $pdbSearchPath = ("{0}\{1}\*.pdb" -f $Config.WORKSPACE, $Config.OUTPUT_DIRECTORY)

    # Exclude the intermediate vc140.pdb files.
    # The use of the vc???.pdb pattern might work with new versions.
    # Filter out the intermediate OUTPUT directory paths.

    $files = Get-ChildItem -Recurse $pdbSearchPath -Exclude vc???.pdb |
             ? { $_.FullName -notmatch '\\OUTPUT\\'}
    return $files
}

###################################################################################################

function Get-FirmwareImagePath
{
    Param(
        [Parameter(Mandatory=$true)]
        [hashtable]
        $Config
    )
    # Example: "E:\edk2.x64\Build\MsvmX64\RELEASE_VS2015xASL\FV\MSVM.fd"
    $fwImagePath = ("{0}\{1}\{2}_{3}\FV\{4}.fd" `
                   -f $config.WORKSPACE, $config.OUTPUT_DIRECTORY, $config.TARGET,
                   $config.TOOL_CHAIN_TAG, $config.PLATFORM_NAME)
    return (Get-ChildItem $fwImagePath).FullName
}

###################################################################################################

function New-TemporaryDirectory
{
  $tempDirectoryBase = Join-Path ([System.IO.Path]::GetTempPath()) "UEFI"
  do
  {
    $newTempDirPath = (Join-Path $tempDirectoryBase ([System.Guid]::NewGuid().ToString("B")))
  } while (Test-Path $newTempDirPath)

  $n = New-Item -ItemType Directory -Path $newTempDirPath

  Write-Verbose "Created temp directory: $newTempDirPath"

  return $newTempDirPath
}

##################################################################################################


function Run-Command
{
    param(
        [Parameter(Mandatory=$true)]
        [string]
        [ValidateNotNullOrEmpty()]
        $Executable,

        [Parameter(Mandatory=$true)]
        [string[]]
        [ValidateNotNullOrEmpty()]
        $Arguments
    )

    Write-Verbose "Run-Command: $Executable $Arguments"

    return & $Executable $Arguments
}
#################################################################################################

function Check-RepositoryState
{
    Write-Verbose "Checking git repository state..."

    $status = Run-Command -Executable "git" -Arguments ("status", "--porcelain" )

    $discard = Run-Command -Executable "git" -Arguments ( "remote", "update" )

    $LocalCommit = Run-Command -Executable "git" -Arguments ( "rev-parse", "`"@`"" )

    $RemoteCommit = Run-Command -Executable "git" -Arguments ( "rev-parse", "`"`@`{u`}`"" )

    $BaseCommit = Run-Command -Executable "git" -Arguments ( "merge-base", "`"@`"", "`"`@`{u`}`"" )

    $result = $false
    if ((-not $IgnoreUncommittedFiles) -and ($status -ne $null))
    {
        Write-Verbose "Local repo has uncommitted files"
    }
    elseif ($LocalCommit -eq $RemoteCommit)
    {
        Write-Verbose "Local repo Up-to-date with remote and no uncommitted files"
        $result = $true
    }
    elseif ($LocalCommit -eq $BaseCommit)
    {
        Write-Verbose "Remote repo has commits not in local. Need to pull."
    } elseif ($RemoteCommit -eq $BaseCommit)
    {
        Write-Verbose "Local repo has commit(s) not in remote"
    } else
    {
        Write-Verbose "Local repo has diverged from remote"
    }
    return $result
}

#################################################################################################

function Get-CurrentCommitHash
{
    $LocalCommit = Run-Command -Executable "git" -Arguments ( "rev-parse", "`"@`"" )
    Write-Verbose ("Current commit is: " + $LocalCommit)
    return $LocalCommit
}

#################################################################################################

function Get-CurrentBranch
{
    $branch = Run-Command -Executable "git" -Arguments ( "rev-parse", "--abbrev-ref", "HEAD" )
    Write-Verbose ("Current branch is: " + $branch)
    return $branch
}

#################################################################################################
function Get-UTCTimeString
{
    Param(
        [Int32]
        $HoursToOffset = 0
    )

    # vPack.exe takes a date/time string that can be parsed with DateTime.Parse(). So
    # convert now to UTC, add the offset, and format with the "u" (Universal Sortable)
    # format. Use UTC to avoid any confusion. "u" causes a Z to be put on end of string to
    # clearly indicate UTC. E.g., "2017-09-15 10:33:16Z"

    $utcTime = (Get-Date).ToUniversalTime().AddHours($HoursToOffset).ToString("u")
    return $utcTime
}

###################################################################################################

function Check-Duplicate-Commit-Hash
{
    Param(
        [Parameter(Mandatory=$true)]
        [hashtable]
        $Edk2Config,

        [Parameter(Mandatory=$true)]
        [ValidateNotNull()]
        [PsCustomObject]
        $VpackConfig,

        [string]
        $CommitHash = ""
    )

    Write-Verbose "Checking for duplicate commit in existing VPacks..."

    $vpackExePath = $Edk2Config.WORKSPACE + "\msprivate\internal\vpack\vpack.exe"

    $vpackCmdArgs = @("List")
    $vpackBaseName = $VpackConfig.vpack.BaseName
    if ($VpackConfig.vpack.AppendArchToName)
    {
        $vpackBaseName += ("." + $Edk2Config.TARGET_ARCH.ToLower())
    }
    $vpackCmdArgs += ("/BaseName:" + $vpackBaseName)
    $packList = Run-Command $vpackExePath $vpackCmdArgs
    $found = $false
    foreach ($pack in $packList)
    {
        if ($pack -Match $CommitHash)
        {
            Write-Verbose ($pack + " matches current commit")
            $found = $true
        }
    }

    return $found
}

###################################################################################################

function Push-VPack
{
    Param(
        [Parameter(Mandatory=$true)]
        [hashtable]
        $Edk2Config,

        [Parameter(Mandatory=$true)]
        [ValidateNotNull()]
        [PsCustomObject]
        $VpackConfig,

        [Parameter(Mandatory=$true)]
        [string]
        [ValidateNotNullOrEmpty()]
        $TempDirPath,

        [string]
        $CommitHash = ""
    )

    $vpackExePath = $Edk2Config.WORKSPACE + "\msprivate\internal\vpack\vpack.exe"

    $vpackCmdArgs = @("Push")
    $vpackBaseName = $VpackConfig.vpack.BaseName
    if ($VpackConfig.vpack.AppendArchToName)
    {
        $vpackBaseName += ("." + $Edk2Config.TARGET_ARCH.ToLower())
    }
    $vpackCmdArgs += ("/BaseName:" + $vpackBaseName)
    $vpackCmdArgs += ("/SourceDirectory:" + $TempDirPath)
    $vpackCmdArgs += "/ServiceType:Drop"
    $vpackCmdArgs += ("/Major:" + $VpackConfig.vpack.Major)
    $vpackCmdArgs += ("/Minor:" + $VpackConfig.vpack.Minor)
    $vpackCmdArgs += ("/Patch:" + $VpackConfig.vpack.Patch)
    if ($VpackConfig.vpack.Prerelease -ne "")
    {
        $vpackCmdArgs += ("/Prerelease:" + $VpackConfig.vpack.Prerelease)
    }
    if ($VpackConfig.meta.AppendLatestCommitHash)
    {
        $vpackCmdArgs += ("/MetaData:" + $CommitHash)
    }
    $vpackCmdArgs += ("/VersionIncrementType:" + $VpackConfig.vpack.VersionIncrementType)
    if ($VpackConfig.vpack.ExpirationTimeInHours -gt 0)
    {
        $vpackCmdArgs += ("/ExpirationTime:`"" + `
            (Get-UtcTimeString -HoursToOffset $VpackConfig.vpack.ExpirationTimeInHours))
    }
    Write-Verbose "vPack command line: $vpackExePath $vpackCmdArgs"
    if (-not $DryRun)
    {
        Run-Command $vpackExePath $vpackCmdArgs
    }
}

###################################################################################################

$tempFileDir = ""
try
{
    # Get the WORKSPACE environment variable to check that Edk2Setup.cmd has been run.

    $workspace = (Get-ChildItem env:WORKSPACE -ErrorAction Ignore).Value
    if ($workspace -eq $null)
    {
        Write-Error -Message ( `
        "This script must be run from the hyperv.uefi repository " + `
        "EDK2 build environment after running Edk2Setup." `
        ) -Category ResourceUnavailable
        return
    }
    Write-Verbose ("Workspace: " + $workspace)

    # Get the EDK2 config and the vpack config (JSON file)

    $edk2Config = Get-Edk2Config -WorkSpace $workspace
    $vpackConfig = Get-VpackConfig -Workspace $workspace

    # Check the current branch matches branch in JSON file

    if (-not $IgnoreBranchName)
    {
        $currentBranch = Get-CurrentBranch
        if ($vpackConfig.meta.GitBranchName -ne $currentBranch)
        {
            Write-Error -Message ( `
            "The local checked out branch `"{0}`" does not match the branch `"{1}`" specified in vpack.json file." `
            -f $currentBranch, $vpackConfig.meta.GitBranchName `
            ) -Category InvalidData
            return
        }
    }

    # Check the repository is up-to-date with remote

    if (-not (Check-RepositoryState))
    {
        Write-Error -Message ( `
        "The repository is not current with the remote repository." `
        ) -Category InvalidResult
        return
    }

    # If appending commit hash check to ensure current commit is not already pushed.

    if ($vpackConfig.meta.AppendLatestCommitHash)
    {
        $currentCommit = Get-CurrentCommitHash
        if (Check-Duplicate-Commit-Hash -Edk2Config $edk2Config -VpackConfig $vpackConfig -CommitHash $currentCommit)
        {
            Write-Error -Message ( `
            "The current commit already exists with a pushed VPack." `
            ) -Category InvalidResult
            return
        }
    }
    
    # Create a temp directory

    $tempFileDir = New-TemporaryDirectory

    # Construct the firmware image file paths

    $imageSourcePath = Get-FirmwareImagePath -Config $edk2Config
    $imageDestFile = ("UEFI{0}.bin" -f $edk2Config.TARGET_ARCH)
    $imageDestPath = ("{0}\{1}" -f $tempFileDir, $imageDestFile)

    # Get the list of PDB files

    $pdbSourceFiles = Get-PdbFiles -Config $edk2Config
    Write-Verbose ("Found {0} PDB files" -f $pdbSourceFiles.Length)

    # Copy the image file to the temp dir

    Write-Verbose "Copying image file to temp dir"
    $activity = "Copying files to temp DIR"
    $filecount = $pdbSourceFiles.Length + 1
    Write-Progress -Activity $activity `
                   -Status $imageDestFile `
                   -PercentComplete (1 / $filecount)
    Copy-Item -Path $imageSourcePath -Destination $imageDestPath

    # Copy the PDBs to the temp dir

    Write-Verbose "Copying PDBs to temp dir"
    foreach ($pdb in $pdbSourceFiles)
    {
        Write-Progress -Activity $activity `
                       -Status $pdb.Name `
                       -PercentComplete (($pdbSourceFiles.IndexOf($pdb) + 2) / $filecount)
        Copy-Item -Path $pdb.FullName -Destination $tempFileDir
    }
    Write-Progress -Activity $activity -Completed

    # Push the vPack

    $currentCommit = Get-CurrentCommitHash
    Push-Vpack -Edk2Config $edk2Config -VpackConfig $vpackConfig `
               -TempDirPath $tempFileDir -CommitHash $currentCommit

}
finally
{
    if ($tempFileDir -ne "")
    {
        remove-item $tempFileDir -Force -Recurse
        Write-Verbose "Deleted temp directory: $tempFileDir"
    }
}
