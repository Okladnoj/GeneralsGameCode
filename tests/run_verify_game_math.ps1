<#
.SYNOPSIS
    Builds and runs verify_game_math.c, then removes every build artifact.

.DESCRIPTION
    Compiles verify_game_math.c against the GameMath library from an existing
    CMake build tree, runs it, and leaves only the dump files in -OutDir
    (the tests folder by default).

    Every intermediate - object file, import library, export file and the
    executable itself - is created in a temporary directory outside the
    repository and deleted afterwards, whether the run succeeds or fails.

.PARAMETER BuildDir
    CMake build tree holding the GameMath dependency.
    Default: <repo>\build\win32

.PARAMETER Config
    GameMath build configuration to link against. Default: Release

.PARAMETER OutDir
    Where the dump files are written. Default: the tests folder.

.PARAMETER KeepExe
    Copy the executable to -OutDir instead of discarding it. Off by default;
    the point of this script is not to leave binaries behind.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File tests\run_verify_game_math.ps1

.NOTES
    Needs a 32-bit MSVC toolchain: the dumps characterise the x86 game build,
    and _controlfp_s(_PC_24) only means anything there.
#>
[CmdletBinding()]
param(
    [string]$BuildDir,
    [string]$Config = 'Release',
    [string]$OutDir,
    [switch]$KeepExe
)

$ErrorActionPreference = 'Stop'

$testsDir = $PSScriptRoot
$repoRoot = Split-Path $testsDir -Parent

if (-not $BuildDir) { $BuildDir = Join-Path $repoRoot 'build\win32' }
if (-not $OutDir)   { $OutDir   = $testsDir }

$source  = Join-Path $testsDir 'verify_game_math.c'
$include = Join-Path $BuildDir '_deps\gamemath-src\include'
$gmLib   = Join-Path $BuildDir "_deps\gamemath-build\$Config\gm.lib"

# ---------- inputs ----------

if (-not (Test-Path $source)) {
    throw "Source not found: $source"
}
if (-not (Test-Path $include)) {
    throw "GameMath headers not found: $include`nConfigure the build tree first, or pass -BuildDir."
}
if (-not (Test-Path $gmLib)) {
    throw "GameMath library not found: $gmLib`nBuild the gm target first, or pass -BuildDir / -Config."
}
if (-not (Test-Path $OutDir)) {
    New-Item -ItemType Directory -Path $OutDir -Force | Out-Null
}

$source  = (Resolve-Path $source).Path
$include = (Resolve-Path $include).Path
$gmLib   = (Resolve-Path $gmLib).Path
$OutDir  = (Resolve-Path $OutDir).Path

# ---------- toolchain ----------

# vcvarsall shells out to vswhere to locate the Windows SDK, but its own
# directory is not necessarily on PATH. Put it there, or the SDK lib paths
# come back incomplete and the link fails on CRT symbols.
$installerDir = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer'
$vswhere      = Join-Path $installerDir 'vswhere.exe'

$vsRoot = $null
if (Test-Path $vswhere) {
    $found = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if ($found) { $vsRoot = ([string]$found).Trim() }
}

if (-not $vsRoot) {
    foreach ($base in @($env:ProgramFiles, ${env:ProgramFiles(x86)})) {
        if (-not $base) { continue }
        foreach ($year in @('2022', '2019')) {
            foreach ($ed in @('Enterprise', 'Professional', 'Community', 'BuildTools')) {
                $probe = Join-Path $base "Microsoft Visual Studio\$year\$ed"
                if (Test-Path (Join-Path $probe 'VC\Auxiliary\Build\vcvarsall.bat')) {
                    $vsRoot = $probe
                    break
                }
            }
            if ($vsRoot) { break }
        }
        if ($vsRoot) { break }
    }
}

if (-not $vsRoot) {
    throw 'No Visual Studio installation with the C++ toolchain was found.'
}

$vcvarsall = Join-Path $vsRoot 'VC\Auxiliary\Build\vcvarsall.bat'
if (-not (Test-Path $vcvarsall)) {
    throw "vcvarsall.bat not found under: $vsRoot"
}

Write-Host "toolchain : $vsRoot"
Write-Host "gm.lib    : $gmLib"
Write-Host "output    : $OutDir"

# ---------- build and run in a throwaway directory ----------

$work = Join-Path ([System.IO.Path]::GetTempPath()) ('gamemath-verify-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $work -Force | Out-Null

try {
    $exe      = Join-Path $work 'verify_game_math.exe'
    $buildLog = Join-Path $work 'build.log'
    $runLog   = Join-Path $work 'run.log'

    # /MD is required: gm.lib is built against the dynamic CRT, and the cl
    # default drags in the static one - the mix fails to link on
    # __imp__fesetround and __except_handler4_common.
    #
    # cl runs with $work as its working directory, so the object file and the
    # linker's .lib/.exp land there too and go away with it.
    $q = [char]34
    $cl = "cl /nologo /O2 /fp:precise /MD $q$source$q /I $q$include$q $q$gmLib$q /Fe:verify_game_math.exe"
    $line = "set ${q}PATH=$installerDir;%PATH%$q && cd /d $q$work$q && call $q$vcvarsall$q x86 >nul && $cl"

    Write-Host 'building...'
    & cmd.exe /c $line > $buildLog 2>&1
    $buildCode = $LASTEXITCODE

    if ($buildCode -ne 0 -or -not (Test-Path $exe)) {
        if (Test-Path $buildLog) { Get-Content $buildLog | Write-Host }
        throw "Build failed (exit $buildCode)."
    }

    Write-Host 'running...'
    $proc = Start-Process -FilePath $exe -WorkingDirectory $work -NoNewWindow -Wait -PassThru -RedirectStandardOutput $runLog
    if ($proc.ExitCode -ne 0) {
        if (Test-Path $runLog) { Get-Content $runLog | Write-Host }
        throw "verify_game_math.exe exited with $($proc.ExitCode)."
    }

    $produced = @(Get-ChildItem -Path $work -Filter 'math-*.txt' -File)
    if ($produced.Count -eq 0) {
        throw 'The run produced no dump files.'
    }

    foreach ($f in $produced) {
        $dest = Join-Path $OutDir $f.Name
        Move-Item -LiteralPath $f.FullName -Destination $dest -Force
        Write-Host ("wrote {0} ({1:N0} bytes)" -f $f.Name, (Get-Item $dest).Length)
    }

    if ($KeepExe) {
        Copy-Item -LiteralPath $exe -Destination (Join-Path $OutDir 'verify_game_math.exe') -Force
        Write-Host 'wrote verify_game_math.exe'
    }
}
finally {
    # Everything else dies with the work directory, on success or failure.
    if (Test-Path $work) {
        Remove-Item -LiteralPath $work -Recurse -Force -ErrorAction SilentlyContinue
    }

    # A manual build following README.md drops these next to the source.
    foreach ($pat in @('*.obj', '*.exp', '*.lib', '*.pdb', '*.ilk', 'verify_game_math.exe')) {
        Get-ChildItem -Path $testsDir -Filter $pat -File -ErrorAction SilentlyContinue |
            Where-Object { -not ($KeepExe -and $_.Name -eq 'verify_game_math.exe') } |
            Remove-Item -Force -ErrorAction SilentlyContinue
    }
}

# ---------- report ----------

$pc24 = Join-Path $OutDir 'math-win-PC24.txt'
$pc53 = Join-Path $OutDir 'math-win-PC53.txt'

if ((Test-Path $pc24) -and (Test-Path $pc53)) {
    $a = @(Get-Content $pc24)
    $b = @(Get-Content $pc53)
    $n = [Math]::Min($a.Count, $b.Count)

    $differing = New-Object System.Collections.Generic.List[string]
    for ($i = 0; $i -lt $n; $i++) {
        if ($a[$i] -ne $b[$i]) {
            $name = ($a[$i] -split '\s+')[0]
            if ($name -and $name -notmatch '^[-=]') { $differing.Add($name) }
        }
    }

    Write-Host ''
    Write-Host ("_PC_24 vs _PC_53: {0} of {1} lines differ" -f $differing.Count, $n)
    $differing | Group-Object | Sort-Object Count -Descending | ForEach-Object {
        Write-Host ("  {0,-16} {1}" -f $_.Name, $_.Count)
    }
}

Write-Host ''
Write-Host 'done; no build artifacts left behind'
