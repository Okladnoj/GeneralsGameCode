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

# 2. Filter DebugFrame files to keep only the last 600
$debugFrames = $allFiles | Where-Object { $_.Name -like "DebugFrame_*.txt" } | Sort-Object Name
$debugFramesToKeep = @()
if ($debugFrames.Count -gt 600) {
    $debugFramesToKeep = $debugFrames | Select-Object -Last 600
} else {
    $debugFramesToKeep = $debugFrames
}

# 3. Process the files
$otherFiles = $allFiles | Where-Object { $_.Name -notlike "DebugFrame_*.txt" }
$filesToProcess = $otherFiles + $debugFramesToKeep

# Remove duplicates if any
$filesToProcess = $filesToProcess | Sort-Object -Property FullName -Unique

foreach ($file in $filesToProcess) {
    $destPath = Join-Path $ArchiveDir $file.Name
    Write-Host "Processing: $($file.FullName)"
    
    # Strictly truncate large diagnostic files (except DebugFrames which we need intact)
    if ($file.Length -gt 5MB -and $file.Name -notlike "DebugFrame_*.txt") {
        Write-Host "  -> File is large ($([math]::Round($file.Length / 1MB, 2)) MB), strictly keeping 50KB head and 1MB tail..." -ForegroundColor Yellow
        
        $headSize = 50KB
        $tailSize = 1MB
        
        $fileStream = [System.IO.File]::OpenRead($file.FullName)
        
        $headBuffer = New-Object byte[] $headSize
        $headBytesRead = $fileStream.Read($headBuffer, 0, $headSize)
        
        $fileStream.Position = $fileStream.Length - $tailSize
        $tailBuffer = New-Object byte[] $tailSize
        $tailBytesRead = $fileStream.Read($tailBuffer, 0, $tailSize)
        
        $fileStream.Close()
        
        $outStream = [System.IO.File]::Create($destPath)
        $outStream.Write($headBuffer, 0, $headBytesRead)
        
        $separator = [System.Text.Encoding]::UTF8.GetBytes("`n... [CONTENT TRUNCATED BY ARCHIVE SCRIPT] ...`n")
        $outStream.Write($separator, 0, $separator.Length)
        
        $outStream.Write($tailBuffer, 0, $tailBytesRead)
        $outStream.Close()
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
