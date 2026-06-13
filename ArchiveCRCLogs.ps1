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

$LogPatterns = @("crcDebug*.txt", "DebugFrame_*.txt", "sync*.txt", "*dbgview*.log", "*debugview*.txt", "*debugview*.log", "ReleaseCrashLog.txt", "DiagLog.txt", "MathPrecisionDiag.txt", "MtxDiag.txt", "LocoDiag.txt", "SlowDeathDiag.txt")
$SearchDirs = @($GameDir, "$GameDir\CRCLogs", $DocsDir, "D:\OKJI\dev\GeneralsGameCode")

$foundCount = 0

# 1. Collect all matching files
$allFiles = @()
foreach ($dir in $SearchDirs) {
    if (Test-Path $dir) {
        foreach ($pattern in $LogPatterns) {
            $files = Get-ChildItem -Path $dir -Filter $pattern -ErrorAction SilentlyContinue
            if ($files) { $allFiles += $files }
        }
    }
}

# 2. Filter DebugFrame files to keep only the last 1000
$debugFrames = $allFiles | Where-Object { $_.Name -like "DebugFrame_*.txt" } | Sort-Object Name
$debugFramesToKeep = @()
if ($debugFrames.Count -gt 1000) {
    $debugFramesToKeep = $debugFrames | Select-Object -Last 1000
} else {
    $debugFramesToKeep = $debugFrames
}

# 3. Process the files
$otherFiles = $allFiles | Where-Object { $_.Name -notlike "DebugFrame_*.txt" }
$filesToProcess = $otherFiles + $debugFramesToKeep

# Remove duplicates if any
$filesToProcess = $filesToProcess | Select-Object -Unique FullName

foreach ($file in $filesToProcess) {
    $destPath = Join-Path $ArchiveDir $file.Name
    Write-Host "Processing: $($file.FullName)"
    
    # Truncate large text logs (e.g., MtxDiag.txt) if they exceed 5MB
    if ($file.Length -gt 5MB -and $file.Name -match "\.(txt|log)$") {
        Write-Host "  -> File is large ($([math]::Round($file.Length / 1MB, 2)) MB), truncating..." -ForegroundColor Yellow
        $head = Get-Content -Path $file.FullName -TotalCount 500
        $tail = Get-Content -Path $file.FullName -Tail 5000
        
        $head | Set-Content -Path $destPath -Encoding UTF8
        Add-Content -Path $destPath -Value "`n... [CONTENT TRUNCATED BY ARCHIVE SCRIPT] ...`n" -Encoding UTF8
        $tail | Add-Content -Path $destPath -Encoding UTF8
    } else {
        Copy-Item -Path $file.FullName -Destination $ArchiveDir -Force
    }
    $foundCount++
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
