$ErrorActionPreference = "Stop"

Write-Host "Initializing MSVC environment and building project..." -ForegroundColor Cyan

# The command to initialize MSVC environment and then run CMake build & install
$cmd = "call `"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat`" x86 && cmake --build build/win32 --config Release --target install"
cmd.exe /c $cmd

if ($LASTEXITCODE -eq 0) {
    Write-Host "Build and install completed successfully!" -ForegroundColor Green
} else {
    Write-Host "Build failed with exit code $LASTEXITCODE." -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host "Running game with -mathCrcCheck flag..." -ForegroundColor Cyan

# Path to the installed game executable
$GameDir = "D:\SteamLibrary\steamapps\common\Command & Conquer Generals - Zero Hour"
$ExePath = "$GameDir\generals.exe"

if (-not (Test-Path $ExePath)) {
    Write-Host "Error: Game executable not found at $ExePath" -ForegroundColor Red
    exit 1
}

# Change directory to the game directory to ensure files are written there
Push-Location $GameDir

# Run the game with the special flag and wait for it to finish
Write-Host "Executing: $ExePath -mathCrcCheck"
Start-Process -FilePath $ExePath -ArgumentList "-mathCrcCheck" -WorkingDirectory $GameDir -Wait

Write-Host "Reading CRC results from SimulationMathCrc.txt..." -ForegroundColor Cyan

$ResultFile = "SimulationMathCrc.txt"
if (Test-Path $ResultFile) {
    Get-Content $ResultFile
    Write-Host ""
    Write-Host "Please share these results with your AI assistant!" -ForegroundColor Magenta
} else {
    Write-Host "Error: Result file $ResultFile was not generated. The game might have crashed or the flag wasn't recognized." -ForegroundColor Red
}

Pop-Location
