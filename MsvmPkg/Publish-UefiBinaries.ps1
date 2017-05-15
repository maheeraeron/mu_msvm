<#
.SYNOPSIS

    Gather and publish the Hyper-V UEFI firmware binaries into VSTS artifacts services as a vPack.

.DESCRIPTION
    This script will scrape the Edk2 build output and gather the firmware image and symbol files 
    into a temporary directory in preparation for publishing to VSTS artifacts services

.PARAMETER OsRepoRootPath
    This parameter specifies the full path to the local OS repository clone.
    It is required.
    
.PARAMETER WorkingDirectory
    Working directory for publishing.  Defaults to %temp%\uefi.
    
.PARAMETER Clean
    Pass this switch to clean the working directory before it is filled with artifacts.
    
.PARAMETER Publish
    Pass this switch to publish artifacts to a new vpack.  For now, this prints the commands you must
    run from a razzle prompt to upload the vpack.

.NOTES

#>
param
(
    [parameter(Mandatory=$True,Position=0)]
    [string]$OsRepoRootPath, 
    
    [ValidateNotNullOrEmpty()]
    [string]$WorkingDirectory = (Join-Path $env:Temp "uefi"),
    
    [switch]$Clean,
    
    [switch]$Publish
)

$OsRepoRootPath = $OsRepoRootPath.TrimEnd("\")
if ((Test-Path $OsRepoRootPath -PathType Container) -eq $False)
{
    Write-Error -Message "Specified OsRepoRootPath ($OsRepoRootPath) doesn't exist." -TargetObject $OsRepoRootPath -Category InvalidArgument
    return
}

if ((Test-Path ($OsRepoRootPath + "\.git") -PathType Container) -eq $False)
{
    Write-Error -Message "Specified OsRepoRootPath ($OsRepoRootPath) isn't a git repository." -TargetObject $OsRepoRootPath -Category InvalidArgument
    return
}

$Workspace = (Get-ChildItem env:WORKSPACE -ErrorAction Ignore).Value
if ($Workspace -eq $null)
{
    Write-Error -Message "This script must be run from the hyperv.uefi repo environment (with Edk2Setup)." -Category ResourceUnavailable
    return
}

$vpackPrefix = "hyperv.uefi"

#
# The possible firmware binary targets
#
$targets = @([pscustomobject]@{tools="VS2015xASL";uefiFlavor="RELEASE";ntFlavor="Fre";uefiarch="x64"},
             [pscustomobject]@{tools="VS2015x86xASL";uefiFlavor="RELEASE";ntFlavor="Fre";uefiarch="aarch64"})

#
# The actual target we have built
#   
$targetFileContent = Get-Content (Join-Path $Workspace "Conf\target.txt")
$targetTypeContent = $targetFileContent |? { $_ -match "^TARGET " }
$targetArchContent = $targetFileContent |? { $_ -match "^TARGET_ARCH" }
$targetType = $targetTypeContent.split('=')[1].trim()
$architecture = $targetArchContent.split('=')[1].trim()

if ($Clean -and (Test-Path $WorkingDirectory))
{
    Remove-Item $WorkingDirectory -Recurse -Force
}            

foreach ($target in $targets)
{    
    if ($($target.uefiarch) -ne $architecture)
    {
        #Write-Host "Skipping possible target $target because it is the wrong architecture ($($target.uefiarch) -ne $architecture)"
        continue
    }
    
    if ($($target.uefiFlavor) -ne $targetType)
    {
        #Write-Host "Skipping possible target $target because it is the wrong flavor ($($target.uefiFlavor) -ne $targetType)"
        continue
    }

    $uefiBuildPath = ("{0}\build\msvm{1}" -f $Workspace, $target.uefiarch)
    $objectRoot = ("{0}_{1}" -f $target.uefiFlavor, $target.tools)
    $stagingPathPrefix = ("{0}\uefi\{1}" -f $WorkingDirectory, $target.uefiarch)
    $fwSourcePath = ("{0}\{1}\FV\MSVM.fd" -f $uefiBuildPath, $objectRoot)
    $fwDestPath = ("{0}\UEFI{1}.bin" -f $WorkingDirectory, $target.uefiarch)
    $pdbSearchPath = ("{0}\{1}\*.pdb" -f $uefiBuildPath, $objectRoot)
    
    
    Write-Host "================================================================"
    Write-Host "Pulling $($target.uefiarch) $($target.uefiFlavor) UEFI Firmware Image to OS repo..."
    Write-Host "Source: $fwSourcePath"
    Write-Host "Dest:   $fwDestPath"

    if ((Test-Path $fwSourcePath) -eq $False)
    {
        Write-Host -f yellow "Warning: The following firmware image was not found."
        Write-Host -f yellow "$fwSourcePath"
        Write-Host -f yellow "SKIPPING $($target.uefiarch) $($target.uefiFlavor) UEFI Firmware Image"
        continue
    }

    if ((Test-Path $fwDestPath) -eq $False)
    {
        if (-not (Test-Path (Split-Path -parent $fwDestPath)))
        {            
            New-Item -Type dir (Split-Path -parent $fwDestPath)
        }     
            
        Write-Host "Adding new firmware image $fwDestPath"
        Copy-Item -Path $fwSourcePath -Destination $fwDestPath
    }
    else
    {
        Write-Host "Replacing existing firmware image $fwDestPath"
        Copy-Item -Path $fwSourcePath -Destination $fwDestPath
    }

    #
    # Get the list of symbol (pdb) files.
    # Exclude the intermediate vc140.pdb files.  *** This name will likely change if the compiler is revised
    # Exclude paths containing "\OUTPUT\"
    #
    $pdblist = Get-ChildItem -Recurse $pdbSearchPath -Exclude vc140.pdb | ? { $_.FullName -notmatch '\\OUTPUT\\'}

    #
    # Drop new files into output share
    #
    foreach ($pdb in $pdblist)
    {
        $pdbDestPath = ("{0}\{1}" -f $WorkingDirectory, $pdb.Name)
        if ((Test-Path $pdbDestPath) -eq $False)
        {
            Write-Host "Adding new symbol file $pdbDestPath"
            Copy-Item -Path $pdb.FullName -Destination $pdbDestPath -Force
        }
        else
        {
            Write-Host "Replacing existing symbol file $pdbDestPath"
            Copy-Item -Path $pdb.FullName -Destination $pdbDestPath -Force
        }
    }
    
    if ($Publish)
    {
        Write-Host "Run the following commands from your OS repo razzle environment to complete the publishing"        
        Write-Host "$OsRepoRootPath\utilities\uploadvpack.cmd -n $vpackPrefix.$($target.uefiarch) -d $WorkingDirectory -m $Workspace\MsvmPkg\$vpackPrefix.$($target.uefiarch).man"
        Write-Host "powershell Remove-Item $WorkingDirectory -Recurse -Force"
    }
}


