$ErrorActionPreference = "Continue"

# Configure the path to your gen-context repo here
$GenContextRepo = "D:\OKJI\dev\GeneralOnlineGameClient-Context"
$GameDir = "D:\SteamLibrary\steamapps\common\Command & Conquer Generals - Zero Hour"
$DocsDir = "$env:USERPROFILE\Documents\Command and Conquer Generals Zero Hour Data"

$ArchiveDir = "$GenContextRepo\windows_crc_logs"

Write-Host "Archiving CRC logs to: $ArchiveDir" -ForegroundColor Cyan

# Check if the repository exists
if (-not (Test-Path $GenContextRepo)) {
    Write-Host "Error: Repository $GenContextRepo not found!" -ForegroundColor Red
    Write-Host "Please edit this script and set the correct path for `$GenContextRepo" -ForegroundColor Yellow
    exit 1
}

New-Item -ItemType Directory -Path $ArchiveDir -Force | Out-Null

$LogPatterns = @("crcDebug*.txt", "DebugFrame_*.txt", "sync*.txt", "*dbgview*.log", "*debugview*.txt", "*debugview*.log", "ReleaseCrashLog.txt", "DiagLog.txt", "MathPrecisionDiag.txt", "MtxDiag.txt", "LocoDiag.txt")
$SearchDirs = @($GameDir, "$GameDir\CRCLogs", $DocsDir, "D:\OKJI\dev\GeneralsGameCode")

$foundCount = 0

foreach ($dir in $SearchDirs) {
    if (Test-Path $dir) {
        foreach ($pattern in $LogPatterns) {
            $files = Get-ChildItem -Path $dir -Filter $pattern -ErrorAction SilentlyContinue
            foreach ($file in $files) {
                Write-Host "Copying: $($file.FullName)"
                Copy-Item -Path $file.FullName -Destination $ArchiveDir -Force
                $foundCount++
            }
        }
    }
}

if ($foundCount -gt 0) {
    Write-Host "`nFound and copied $foundCount files." -ForegroundColor Green
    
    # Zip it up like the previous archive
    $ZipPath = "$GenContextRepo\windows_crc_logs.zip"
    Write-Host "Zipping logs to $ZipPath..." -ForegroundColor Cyan
    Compress-Archive -Path "$ArchiveDir\*" -DestinationPath $ZipPath -Force
    Remove-Item -Path $ArchiveDir -Recurse -Force
    
    # Push to Git
    Write-Host "Committing to gen-context repo..." -ForegroundColor Cyan
    Push-Location $GenContextRepo
    
    git add windows_crc_logs.zip
    git commit -m "Update CRC logs"
    
    # Push automatically to remote
    git push
    
    Pop-Location
    Write-Host "Done! Logs committed to repository." -ForegroundColor Green
} else {
    Write-Host "`nNo CRC logs found!" -ForegroundColor Yellow
    Remove-Item -Path $ArchiveDir -Force
}
