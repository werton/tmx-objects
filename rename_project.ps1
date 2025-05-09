<#
.SYNOPSIS
    Recursively replaces text in files and renames files with specific patterns
.DESCRIPTION
    1. Replaces text content in files (including hidden folders) with UTF-8 support
    2. Renames files matching specific patterns (like source_text.cbp)
.PARAMETER Path
    Directory to search (default: current directory)
.PARAMETER SearchText
    Text to search for (in content and filenames)
.PARAMETER New_Project_Name
    Replacement text (for content and filenames)
.PARAMETER FilePattern
    File pattern to search (default: *)
.PARAMETER IncludeBinary
    Process binary files (not recommended)
.EXAMPLE
    .\Replace-Text.ps1 -SearchText "old" -New_Project_Name "new"
#>

param(
    [string]$Path = ".",
    #[Parameter(Mandatory=$true)][string]$SearchText,
    [string]$SearchText = "tmx-objects",
    [Parameter(Mandatory=$true)][string]$New_Project_Name,
    [string]$FilePattern = "*",
    [switch]$IncludeBinary
)

# Initialize counters
$totalFiles = 0
$processedFiles = 0
$totalReplacements = 0
$renamedFiles = 0

# First phase: Rename matching files
Write-Host "`nStarting file renaming process..." -ForegroundColor Yellow
$filesToRename = @('*.cbp','*.iml') # Add more patterns if needed

foreach ($pattern in $filesToRename) {
    Get-ChildItem -Path $Path -Filter "*$SearchText*$pattern" -Recurse -Force | ForEach-Object {
        $newName = $_.Name -replace [regex]::Escape($SearchText), $New_Project_Name
        $newPath = Join-Path $_.Directory.FullName $newName
        
        try {
            Rename-Item -Path $_.FullName -NewName $newName -ErrorAction Stop
            Write-Host "Renamed: $($_.FullName) → $newName" -ForegroundColor Magenta
            $renamedFiles++
        }
        catch {
            Write-Warning "Failed to rename $($_.FullName): $($_.Exception.Message)"
        }
    }
}

# Second phase: Content replacement
Write-Host "`nStarting content replacement process..." -ForegroundColor Yellow
$files = Get-ChildItem -Path $Path -Filter $FilePattern -Recurse -File -Force

foreach ($file in $files) {
    # Skip binary files unless explicitly included
    if (-not $IncludeBinary -and $file.Extension -match '\.(exe|dll|png|jpg|zip|rar|7z)$') {
        Write-Verbose "Skipping binary file: $($file.FullName)"
        continue
    }

    $totalFiles++
    $filePath = $file.FullName
    
    try {
        # Detect file encoding
        $encoding = [System.Text.Encoding]::UTF8
        $content = [System.IO.File]::ReadAllText($filePath, $encoding)
        
        if ($content.Contains($SearchText)) {
            $replaceCount = ([regex]::Matches($content, [regex]::Escape($SearchText))).Count
            $content = $content.Replace($SearchText, $New_Project_Name)
            
            # Save with original encoding
            [System.IO.File]::WriteAllText($filePath, $content, $encoding)
            
            $processedFiles++
            $totalReplacements += $replaceCount
            Write-Host "Replaced $replaceCount occurrence(s) in: $filePath" -ForegroundColor Green
        }
    }
    catch {
        Write-Warning "Error processing file: $filePath ($($_.Exception.Message))"
    }
}

# Summary
Write-Host "`nProcess completed:`n" -ForegroundColor Cyan
Write-Host "  Files scanned:        $totalFiles"
Write-Host "  Files modified:       $processedFiles"
Write-Host "  Total replacements:   $totalReplacements"
Write-Host "  Files renamed:        $renamedFiles`n"

# Wait for key press
Write-Host "Press any key to continue..."
[Console]::ReadKey($true) | Out-Null