<#
Simple helper: stage all changes, commit with a message, and push current branch.
Usage:
  cd C:\Users\ASUS\Projects\matching_engine_cpp
  .\scripts\push_append.ps1 -Message "Describe changes"

This script assumes you have git installed and authenticated (SSH or HTTPS).
#>
param(
    [string]$Message = "Append: small fixes and CI",
    [switch]$Force
)

function Fail([string]$m){ Write-Error $m; exit 1 }

try { $v = & git --version 2>$null } catch { Fail "git not found. Install Git for Windows and try again." }
Write-Host "Detected: $v"

Push-Location -Path (Join-Path $PSScriptRoot "..")

Write-Host "Staging all changes..."
& git add -A
if ($LASTEXITCODE -ne 0) { Fail "git add failed (exit code $LASTEXITCODE)" }

$staged = (& git diff --cached --name-only) 2>$null
if (-not $staged) { Write-Host "No staged changes to commit."; Pop-Location; exit 0 }

Write-Host "Committing with message: $Message"
& git commit -m "$Message"
if ($LASTEXITCODE -ne 0) { Fail "git commit failed (exit code $LASTEXITCODE)" }

# determine branch
$branch = (& git symbolic-ref --short HEAD 2>$null) -replace "\r|\n", ''
if (-not $branch) { Write-Host "No branch detected; creating 'main'"; & git checkout -b main; $branch = 'main' }

Write-Host "Pushing to origin/$branch..."
& git push -u origin $branch
if ($LASTEXITCODE -ne 0) { Fail "git push failed (exit code $LASTEXITCODE)" }

Write-Host "Push complete. Verify on GitHub."
Pop-Location
