param(
    [ValidateSet('Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel')]
    [string]$Configuration = 'Release'
)

$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path

Push-Location $Root
try {
    & "$Root\generate_msvc.ps1"
    if ($LASTEXITCODE -ne 0) {
        throw "MSVC project generation failed with exit code $LASTEXITCODE."
    }

    $Selected = (Get-Content "$Root\build\.active_generator" -Raw).Trim()
    $BuildDirectory = "$Root\build\$Selected"
    $Executable = "$BuildDirectory\$Configuration\EpochGuiRoundedCorners.exe"

    & cmake --build $BuildDirectory --config $Configuration --parallel
    if ($LASTEXITCODE -ne 0) {
        throw "MSVC build failed with exit code $LASTEXITCODE. No executable was produced."
    }

    & ctest --test-dir $BuildDirectory -C $Configuration --output-on-failure
    if ($LASTEXITCODE -ne 0) {
        throw "Geometry contract tests failed with exit code $LASTEXITCODE."
    }

    if (-not (Test-Path $Executable)) {
        throw "MSVC reported success, but the expected executable was not found: $Executable"
    }

    Write-Host "Executable: $Executable"
}
finally {
    Pop-Location
}
