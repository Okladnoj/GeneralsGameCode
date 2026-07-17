# Deterministic-math parity run (Windows side).
# Launches the game headless with -mathCrcCheck. GameMain runs the parity dump right after
# engine init (working dir = game dir, just like a replay), writes SimulationMathCrc.txt and
# exits. The result is archived into the context repo under temp_win_math/ and pushed, so the
# macOS side can pull it and diff against .agent/temp_mac_math/.

$ErrorActionPreference = "Continue"

# Configure paths to match your setup (same as ArchiveCRCLogs.ps1).
$GenContextRepo = "D:\OKJI\dev\GeneralOnlineGameClient-Context"
$GameDir = "D:\SteamLibrary\steamapps\common\Command & Conquer Generals - Zero Hour"

$OutDir = "$GenContextRepo\temp_win_math"
$ParityFile = "$GameDir\SimulationMathCrc.txt"

$exePath = "$GameDir\generalszh.exe"
if (-not (Test-Path $exePath)) { $exePath = "$GameDir\generals.exe" }
if (-not (Test-Path $exePath)) {
    Write-Host "Error: game exe not found in $GameDir" -ForegroundColor Red
    exit 1
}

# Purge stale parity file so only this run is collected.
Remove-Item -Path $ParityFile -Force -ErrorAction SilentlyContinue

Write-Host "Running -mathCrcCheck (headless)..." -ForegroundColor Cyan
Start-Process -FilePath $exePath -ArgumentList "-headless -mathCrcCheck" -WorkingDirectory $GameDir -Wait

if (-not (Test-Path $ParityFile)) {
    Write-Host "Error: SimulationMathCrc.txt not produced in $GameDir" -ForegroundColor Red
    exit 1
}

if (-not (Test-Path $GenContextRepo)) {
    Write-Host "Error: context repo $GenContextRepo not found!" -ForegroundColor Red
    Write-Host "Please edit this script and set the correct path for `$GenContextRepo" -ForegroundColor Yellow
    exit 1
}

New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
Copy-Item -Path $ParityFile -Destination "$OutDir\SimulationMathCrc.txt" -Force
Write-Host "Copied parity log -> $OutDir" -ForegroundColor Green
Write-Host "--- aggregate CRCs ---" -ForegroundColor DarkGray
Select-String -Path $ParityFile -Pattern "SimulationMathCrc.*=" | Select-Object -Last 4 | ForEach-Object { $_.Line }

Write-Host "Committing to gen-context repo..." -ForegroundColor Cyan
Push-Location $GenContextRepo
git pull --rebase --autostash
git add "temp_win_math/SimulationMathCrc.txt"
git commit -m "Update math parity log"
git push
Pop-Location
Write-Host "Done! Math parity log pushed to repository." -ForegroundColor Green
