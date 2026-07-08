$ErrorActionPreference = "Continue"

$GenContextRepo = "D:\OKJI\dev\GeneralOnlineGameClient-Context"
$GameDir = "D:\SteamLibrary\steamapps\common\Command & Conquer Generals - Zero Hour"
$DocsDir = "$env:USERPROFILE\Documents\Command and Conquer Generals Zero Hour Data"

$ArchiveDir = "$GenContextRepo\windows_crc_logs"
Write-Host "Archiving CRC logs to: $ArchiveDir (V2 - Target Frame 1000)" -ForegroundColor Cyan

New-Item -ItemType Directory -Path $ArchiveDir -Force | Out-Null

$LogPatterns = @("crcDebug*.txt", "DebugFrame_*.txt", "sync*.txt", "DiagLog.txt", "MathPrecisionDiag.txt", "MtxDiag.txt", "LocoDiag.txt", "SlowDeathDiag.txt", "ValidateCachedBonesDiag.txt", "HCAnimDiag.txt", "HRawAnimDiag.txt", "AnimUpdateDiag.txt", "USODiag.txt", "AnimLoadDiag.txt", "ValidateBonesDiag2.txt", "GetBoneTransformDiag.txt")
$SearchDirs = @($GameDir, "$GameDir\CRCLogs", $DocsDir, "D:\OKJI\dev\GeneralsGameCode")

$allFiles = @()
foreach ($dir in $SearchDirs) {
    if (Test-Path $dir) {
        foreach ($pattern in $LogPatterns) {
            $files = Get-ChildItem -Path $dir -Filter $pattern -ErrorAction SilentlyContinue
            if ($files) { $allFiles += $files }
        }
    }
}

$allFiles = $allFiles | Sort-Object -Property FullName -Unique

# For DebugFrames, we keep frames roughly around the desync to avoid thousands of files.
$filesToProcess = @()
foreach ($file in $allFiles) {
    if ($file.Name -match "DebugFrame_(\d+)\.txt") {
        $frameNum = [int]$matches[1]
        if ($frameNum -le 1500) {
            $filesToProcess += $file
        }
    } else {
        $filesToProcess += $file
    }
}

foreach ($file in $filesToProcess) {
    $destPath = Join-Path $ArchiveDir $file.Name
    Write-Host "Processing: $($file.FullName)"
    
    # Take the first 75MB of diagnostic files to ensure we capture the first 1500 frames.
    $headSize = 75MB
    
    if ($file.Length -gt $headSize -and $file.Name -notlike "DebugFrame_*.txt") {
        Write-Host "  -> Extracting first 75MB for frame 1000 analysis..." -ForegroundColor Yellow
        $fileStream = [System.IO.File]::OpenRead($file.FullName)
        
        $buffer = New-Object byte[] $headSize
        $bytesRead = $fileStream.Read($buffer, 0, $headSize)
        $fileStream.Close()
        
        $outStream = [System.IO.File]::Create($destPath)
        $outStream.Write($buffer, 0, $bytesRead)
        $outStream.Close()
    } else {
        Copy-Item -Path $file.FullName -Destination $ArchiveDir -Force
    }
}
Write-Host "Done! Please commit windows_crc_logs in GeneralOnlineGameClient-Context." -ForegroundColor Green
