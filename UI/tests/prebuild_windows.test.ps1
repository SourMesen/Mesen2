# test that all needed scenarios of the prebuild_windows script work.

Describe "Mesen Prebuild Logic Verification" {
    BeforeAll {
        # set up sandbox in Temp
        $Sandbox = Join-Path $env:TEMP "Mesen_CI_Sandbox"
        if (Test-Path $Sandbox) { Remove-Item $Sandbox -Recurse -Force -ErrorAction SilentlyContinue }
        New-Item -Path $Sandbox -ItemType Directory -Force | Out-Null

        # map T: to this sandbox so Join-Path ..\.. works correctly
        if (Get-PSDrive T -ErrorAction SilentlyContinue) { Remove-PSDrive T }
        New-PSDrive -Name T -PSProvider FileSystem -Root $Sandbox | Out-Null

        $script:OldProfile = $env:USERPROFILE
        $env:USERPROFILE = "T:\Users\testuser"

        $PREBUILD_SCRIPT = "$PSScriptRoot/../prebuild_windows.ps1"
        $RUNTIME_ID = "win-x64"

        $script:TestNumber = 0
    }

    AfterAll {
        $env:USERPROFILE = $script:OldProfile
        if (Get-PSDrive T -ErrorAction SilentlyContinue) { Remove-PSDrive T }
    }

    Context "Full Mock Isolation" {
        BeforeEach {
            # mock all actions that might cause changes on disk
            Mock Compress-Archive { }
            Mock Copy-Item { }
            Mock Get-Item { param($Path) return [PSCustomObject]@{ FullName = $Path } }
            Mock Move-Item { }
            Mock New-Item { return [PSCustomObject]@{ FullName = "MockedDir" } }
            Mock Remove-Item { }
            Mock Set-Location { }
            Mock Test-Path { return $true }

            Mock Get-ChildItem {
                param($Path, $Filter)
                if ($Path -like "*\.nuget\*") {
                    return [PSCustomObject]@{ FullName = Join-Path $env:USERPROFILE ".nuget\packages\runtimes\$RUNTIME_ID\native\$Filter" }
                }
                if ($Filter -eq "MesenCore.dll") {
                    return [PSCustomObject]@{ FullName = Join-Path $env:USERPROFILE "code\mesen\bin\$RUNTIME_ID\Release\MesenCore.dll" }
                }
            }
            $error.Clear()

            $script:TestNumber += 1
            Write-Host "-------"
            Write-Host "[Test] $TestNumber" -ForegroundColor Cyan
        }

        It "Should handle Relative OutDir (Local Style)" {
            $params = @{
                ProjectDir = Join-Path $env:USERPROFILE "code\UI"
                OutDir     = "..\bin\Release"
                RuntimeIdentifier = $RUNTIME_ID
            }
            { & $PREBUILD_SCRIPT @params } | Should -Not -Throw
        }

        It "Should handle Absolute OutDir (CI Style)" {
            $params = @{
                ProjectDir = Join-Path $env:USERPROFILE "code\UI"
                OutDir     = Join-Path $env:USERPROFILE "code\bin\Release"
                RuntimeIdentifier = $RUNTIME_ID
            }
            { & $PREBUILD_SCRIPT @params } | Should -Not -Throw
        }

        It "Matches the vcxproj OutDir structure" {
            $Repo = "T:\MesenRepo"
            $ProjectDir = "$Repo\UI"

            Mock Test-Path { return $true }
            Mock Copy-Item { }
            Mock Compress-Archive { }
            Mock Move-Item { }
            Mock New-Item { [PSCustomObject]@{ FullName = "MockedDir" } }
            Mock Set-Location { }

            $params = @{
                ProjectDir        = $ProjectDir
                OutDir            = "..\bin\$RUNTIME_ID\Release"
                RuntimeIdentifier = $RUNTIME_ID
            }

            { & $PREBUILD_SCRIPT @params } | Should -Not -Throw
        }

        It "Should verify the zip ends up in the ProjectDir" {
            $ProjectDir = Join-Path $env:USERPROFILE "code\mesen\UI"

            $params = @{
                ProjectDir = $ProjectDir
                OutDir     = "T:\Some\Other\Path"
                RuntimeIdentifier = $RUNTIME_ID
            }
            { & $PREBUILD_SCRIPT @params } | Should -Not -Throw

            $ExpectedDestination = Join-Path $ProjectDir "Dependencies.zip"

            Assert-MockCalled Move-Item -ParameterFilter { 
                $Destination -eq $ExpectedDestination
            }
        }

        It "Should raise exception on empty paths" {
            $params = @{ ProjectDir = ""; OutDir = ""; RuntimeIdentifier = "win-x64" }

            & $PREBUILD_SCRIPT @params 2>&1

            $error.Count | Should -BeGreaterThan 0
            $error[0].Exception.GetType().FullName | Should -Be "System.ArgumentException"
            $LASTEXITCODE | Should -Be 1
        }

        It "Should raise exception when NuGet DLL is missing" {
            Mock Get-ChildItem { return $null } -ParameterFilter { $Path -like "*\.nuget\*" }

            $params = @{
                ProjectDir = "T:\Mesen"
                OutDir     = "bin"
                RuntimeIdentifier = $RUNTIME_ID
            }

            & $PREBUILD_SCRIPT @params 2>&1

            $error.Count | Should -BeGreaterThan 0
            $error[0].Exception.GetType().FullName | Should -Be "System.IO.FileNotFoundException"
            $LASTEXITCODE | Should -Be 1
        }

        It "Should print error when MesenCore.dll is missing" {
            Mock Test-Path { 
                param($Path) 
                if ($Path -like "*MesenCore.dll") { return $false }
                return $true 
            }
            Mock Get-ChildItem { return $null } -ParameterFilter { $Filter -eq "MesenCore.dll" }

            $params = @{
                ProjectDir = "T:\Mesen"
                OutDir     = "bin"
                RuntimeIdentifier = $RUNTIME_ID
            }

            & $PREBUILD_SCRIPT @params 2>&1

            $error.Count | Should -BeGreaterThan 0
            $error[0].Exception.GetType().FullName | Should -Be "System.IO.FileNotFoundException"
            $LASTEXITCODE | Should -Be 1
        }
    }
}
