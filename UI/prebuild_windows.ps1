# find and copy the files needed for building the UI project.
# called by the build target in UI.csproj.
# these steps are stored as a script for easier reading and editing.

param (
    [string]$OutDir,           # $(ProjectDir)..\bin\$(RuntimeIdentifier)\$(Configuration), like bin\win-x64\Release
    [string]$ProjectDir,
    [string]$RuntimeIdentifier # win-x64, win-arm64, etc.
)

$ErrorActionPreference = "Stop"

try {

    $MissingParams = foreach ($Name in "ProjectDir", "OutDir", "RuntimeIdentifier") {
        if ([string]::IsNullOrWhiteSpace((Get-Variable $Name -ValueOnly))) {
            $Name
        }
    }

    if ($MissingParams) {
        $List = $MissingParams -join ", "
        throw [System.ArgumentException] "Required parameter(s) '[$List]' are missing or empty."
    }

    # clean paths (PowerShell handles trailing slashes/dots automatically with Get-Item)
    $ProjectDir = $ProjectDir.TrimEnd('\')
    if ($OutDir -match '^[a-zA-Z]:') {
        $FullOutDir = $OutDir
    } else {
        $FullOutDir = Join-Path "$ProjectDir" $OutDir
    }

    Write-Host "[PREBUILD] Project Dir: $ProjectDir"
    Write-Host "[PREBUILD] Target Dir : $FullOutDir"

    # ensure FullOutDir exists and move there
    if (!(Test-Path $FullOutDir)) { New-Item -ItemType Directory -Path $FullOutDir | Out-Null }
    if (!(Test-Path $FullOutDir)) {
        throw [System.IO.DirectoryNotFoundException] "$FullOutDir does not exist"
    }
    Set-Location $FullOutDir

    # set up Dependencies folder
    $DepsFolder = Join-Path $FullOutDir "Dependencies"
    if (Test-Path $DepsFolder) { Remove-Item -Recurse -Force $DepsFolder }
    New-Item -ItemType Directory -Path $DepsFolder | Out-Null

    # 1. copy external dlls managed by NuGet package manager.
    # in web CI builds these are installed by 'dotnet restore'.
    $Libs = @("libHarfBuzzSharp.dll", "libSkiaSharp.dll")
    $NuGetBase = "$env:USERPROFILE\.nuget\packages"
    $SpecificPathPart = "runtimes\$RuntimeIdentifier\native"

    foreach ($Lib in $Libs) {
        $FoundFile = Get-ChildItem -Path $NuGetBase -Filter $Lib -Recurse | 
                 Where-Object { $_.FullName -like "*$SpecificPathPart*" } | 
                 Select-Object -First 1

    if ($FoundFile -and (Test-Path $FoundFile.FullName)) {
        $LibSourcePath = $FoundFile.FullName
        Write-Host "[PREBUILD] Copying NuGet dll $LibSourcePath"
        Copy-Item $LibSourcePath -Destination $DepsFolder
    } else {
            $troubleshooting = "Please rebuild or restore packages. (Right-click solution in Visual Studio -> click 'Restore NuGet Packages', or run 'dotnet restore' in shell"

            $SearchPattern = Join-Path $NuGetBase "*\$SpecificPathPart"
            $DllSourceFolder = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($SearchPattern)

            throw [System.IO.FileNotFoundException] "Required file '$Lib' for $RuntimeIdentifier not found in '$DllSourceFolder'. $troubleshooting"
        }
    }

    # 2. copy MesenCore.dll output from building the Core c++ project
    $Dll = "MesenCore.dll"
    $DllSourcePath = Join-Path $FullOutDir $Dll

    if (!(Test-Path $DllSourcePath)) {
        Write-Host "[PREBUILD] $Dll not found in $DllSourcePath . Checking fallback path"
        # Fallback: Check if the file is one level up or if the OutDir had a double-slash issue
        $ParentDir = Split-Path $FullOutDir -Parent
        $FoundFile = Get-ChildItem -Path $ParentDir -Filter $Dll -Recurse | Select-Object -First 1

        if ($FoundFile) {
            $DllSourcePath = $FoundFile.FullName
        }
    }

    if ($DllSourcePath -and (Test-Path $DllSourcePath)) {
        Write-Host "[PREBUILD] Copying $Dll from: $DllSourcePath"
        Copy-Item $DllSourcePath -Destination $DepsFolder
    } else {
        $ParentPath = Split-Path $DllSourcePath -Parent
        $DllSourceFolder = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($ParentPath)
        throw [System.IO.FileNotFoundException] "Required file '$Dll' not found in '$DllSourceFolder'. Please try rebuilding the solution."
    }

    # 3. copy files that exist in source control
    Write-Host "[PREBUILD] Copying other dependencies from $ProjectDir\Dependencies"
    Copy-Item -Path "$ProjectDir\Dependencies\*" -Destination $DepsFolder -Recurse -Force

    # zip files
    $ZipPath = Join-Path $FullOutDir "Dependencies.zip"
    if (Test-Path $ZipPath) { Remove-Item $ZipPath }

    Write-Host "[PREBUILD] Creating Zip $ZipPath..."
    Compress-Archive -Path $DepsFolder -DestinationPath $ZipPath -Force

    # move instead of copy so we don't leave copies of the zip lying around
    Move-Item $ZipPath -Destination (Join-Path $ProjectDir "Dependencies.zip") -Force
    Write-Host "[PREBUILD] Success."
}
catch {

    # format an error message that shows the error inside Visual Studio.
    # using this specific format turns this into a clickable link in the Error List window or tab.
    # Format: path\to\file.ps1(line): error: message
    # https://learn.microsoft.com/en-us/visualstudio/msbuild/msbuild-diagnostic-format-for-tasks

    $file = $_.InvocationInfo.ScriptName
    $line = $_.InvocationInfo.ScriptLineNumber
    $errorMessage = "$file($line): error: $($_.Exception.Message)"

    # write to regular output so Visual Studio picks up + parses the message
    Write-Host $errorMessage

    exit 1
}
