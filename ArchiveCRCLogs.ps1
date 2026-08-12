# Cross-machine replay CRC collector (Windows side).
#   -ReplayDef : run the reference replay headless with per-frame CRC dump before collecting.
#   (no flag)  : just collect and push whatever logs are already present.
# Output is zipped to windows_crc_logs.zip in the context repo and pushed, so the macOS side
# can pull it and diff crcDebug / DebugFrame by object ID against its own run.

$ErrorActionPreference = "Continue"

$RunReplayDef = $false
if ($args -contains "--rep_def" -or $args -contains "-ReplayDef") {
    $RunReplayDef = $true
}

# ============================================================================
# Configure to match your setup.
# ============================================================================
$GenContextRepo  = "D:\OKJI\dev\GeneralOnlineGameClient-Context"
$GameDir         = "D:\SteamLibrary\steamapps\common\Command & Conquer Generals - Zero Hour"
$DocsDir         = "$env:USERPROFILE\Documents\Command and Conquer Generals Zero Hour Data"
$ReferenceReplay = "00000000.rep"   # the fixed replay both machines play back

$ArchiveDir = "$GenContextRepo\windows_crc_logs"

# Log files collected after a run. crcDebug + DebugFrame carry the per-frame / per-object CRC
# that the cross-machine diff compares; sync/crash logs are kept for context.
$LogPatterns = @(
    "crcDebug*.txt",
    "DebugFrame_*.txt",
    "sync*.txt",
    "ReleaseCrashLog.txt",
    "DiagLog.txt"
)

$SearchDirs = @($GameDir, "$GameDir\CRCLogs", $DocsDir)

# Purge previous-run logs so a fresh replay starts with empty diagnostics. The CRCLogs bulk dir
# (tens of thousands of DebugFrame_*.txt) is wiped in one call instead of per-file globbing.
function Remove-PreviousLogs {
    param([string[]]$Dirs, [string[]]$Patterns, [string]$BulkDir)
    foreach ($dir in $Dirs) {
        if (-not (Test-Path $dir)) { continue }

        if ($BulkDir -and ($dir -eq $BulkDir)) {
            $count = (Get-ChildItem -Path $dir -File -ErrorAction SilentlyContinue | Measure-Object).Count
            Write-Host "  purge (bulk) -> $dir ($count files)" -ForegroundColor DarkGray
            Remove-Item -Path "$dir\*" -Recurse -Force -ErrorAction SilentlyContinue
            $script:purgedCount += $count
            continue
        }

        foreach ($pattern in $Patterns) {
            Get-ChildItem -Path $dir -Filter $pattern -File -ErrorAction SilentlyContinue | ForEach-Object {
                Remove-Item -Path $_.FullName -Force -ErrorAction SilentlyContinue
                $script:purgedCount++
            }
        }
    }
}

Write-Host "Archiving CRC logs to: $ArchiveDir" -ForegroundColor Cyan

if ($RunReplayDef) {
    $exePath = "$GameDir\generalszh.exe"
    if (-not (Test-Path $exePath)) { $exePath = "$GameDir\generals.exe" }

    if (-not (Test-Path $exePath)) {
        Write-Host "Error: Could not find game exe to run replay!" -ForegroundColor Red
        exit 1
    }

    Write-Host "Purging previous-run logs before replay..." -ForegroundColor Cyan
    $script:purgedCount = 0
    Remove-PreviousLogs -Dirs $SearchDirs -Patterns $LogPatterns -BulkDir "$GameDir\CRCLogs"
    Write-Host "  purged $script:purgedCount file(s)." -ForegroundColor DarkGray

    Write-Host "Running headless replay playback ($ReferenceReplay)..." -ForegroundColor Cyan
    Start-Process -FilePath $exePath -ArgumentList "-headless -replay $ReferenceReplay -saveDebugCRCPerFrame .\CRCLogs -keepCRCSave -logObjectCRCs -logRandom" -WorkingDirectory $GameDir -Wait
}

if (-not (Test-Path $GenContextRepo)) {
    Write-Host "Error: Repository $GenContextRepo not found!" -ForegroundColor Red
    Write-Host "Please edit this script and set the correct path for `$GenContextRepo" -ForegroundColor Yellow
    exit 1
}

New-Item -ItemType Directory -Path $ArchiveDir -Force | Out-Null

# 1. Collect all matching files.
$allFiles = @()
foreach ($dir in $SearchDirs) {
    if (Test-Path $dir) {
        foreach ($pattern in $LogPatterns) {
            $files = Get-ChildItem -Path $dir -Filter $pattern -ErrorAction SilentlyContinue
            if ($files) { $allFiles += $files }
        }
    }
}

# 2. Keep only the last 600 DebugFrame files (they are the bulk; the desync tail is what matters).
$debugFrames = $allFiles | Where-Object { $_.Name -like "DebugFrame_*.txt" } | Sort-Object Name
$debugFramesToKeep = if ($debugFrames.Count -gt 600) { $debugFrames | Select-Object -Last 600 } else { $debugFrames }

# 3. Deduplicate the final set.
$otherFiles = $allFiles | Where-Object { $_.Name -notlike "DebugFrame_*.txt" }
$filesToProcess = ($otherFiles + $debugFramesToKeep) | Sort-Object -Property FullName -Unique

$foundCount = 0
foreach ($file in $filesToProcess) {
    $destPath = Join-Path $ArchiveDir $file.Name
    Write-Host "Processing: $($file.FullName)"

    $headSize = 100KB
    $tailSize = 10MB

    # Truncate oversized diagnostic files (never DebugFrames, which must stay intact for object diff).
    if ($file.Length -gt ($headSize + $tailSize) -and $file.Name -notlike "DebugFrame_*.txt") {
        Write-Host "  -> large ($([math]::Round($file.Length / 1MB, 2)) MB), keeping 100KB head + 10MB tail..." -ForegroundColor Yellow

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

# 4. Include the reference replay itself for provenance.
$ReplaySrc = Join-Path $DocsDir "Replays\$ReferenceReplay"
if (Test-Path $ReplaySrc) {
    Write-Host "Copying reference replay: $ReferenceReplay" -ForegroundColor Green
    Copy-Item -Path $ReplaySrc -Destination (Join-Path $ArchiveDir $ReferenceReplay) -Force
    $foundCount++
}

if ($foundCount -eq 0) {
    Write-Host "`nNo CRC logs found!" -ForegroundColor Yellow
    Remove-Item -Path $ArchiveDir -Recurse -Force -ErrorAction SilentlyContinue
    exit 0
}

Write-Host "`nFound and copied $foundCount files." -ForegroundColor Green

$ZipPath = "$GenContextRepo\windows_crc_logs.zip"
Write-Host "Zipping logs to $ZipPath..." -ForegroundColor Cyan
Compress-Archive -Path "$ArchiveDir\*" -DestinationPath $ZipPath -Force
Remove-Item -Path $ArchiveDir -Recurse -Force

Write-Host "Committing to gen-context repo..." -ForegroundColor Cyan
Push-Location $GenContextRepo
git pull --rebase --autostash
git add windows_crc_logs.zip
git commit -m "Update CRC logs"
git push
Pop-Location
Write-Host "Done! Logs committed to repository." -ForegroundColor Green
