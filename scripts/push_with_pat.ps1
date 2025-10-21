<#
One-time secure push using a Personal Access Token (PAT).
This script will:
 - prompt you for your GitHub username
 - prompt you for your PAT securely (input hidden)
 - set the origin remote to a temporary URL that embeds the PAT
 - push the current branch
 - restore the original origin remote (if any)

Security notes:
 - Do NOT paste your PAT into chat or store it in files.
 - This script only uses the PAT in memory and restores the remote after pushing.
 - However, embedding the PAT in the remote URL may be visible to other local users while the command runs. Use on a secure machine.

Usage:
  cd C:\Users\ASUS\Projects\matching_engine_cpp
  .\scripts\push_with_pat.ps1 -RemoteUrl "https://github.com/AdityaMehta2006/Matching_engine.git" -CommitMessage "Add project files"
#>
param(
    [string]$RemoteUrl = "https://github.com/AdityaMehta2006/Matching_engine.git",
    [string]$CommitMessage = "Add project files"
)

function Fail([string]$msg) { Write-Error $msg; exit 1 }

# check git
try { $v = & git --version 2>$null } catch { Fail "git not found. Install Git for Windows and try again." }
Write-Host "Detected: $v"

Push-Location -Path (Join-Path $PSScriptRoot "..")

# ensure repo
$inside = $false
try { $inside = (& git rev-parse --is-inside-work-tree) -eq 'true' } catch { $inside = $false }
if (-not $inside) { Write-Host "Initializing git repo..."; & git init; if ($LASTEXITCODE -ne 0) { Fail "git init failed" } }

# stage & commit
& git add -A; if ($LASTEXITCODE -ne 0) { Fail "git add failed" }
$diff = (& git diff --cached --name-only) 2>$null
if ($diff) { & git commit -m "$CommitMessage"; if ($LASTEXITCODE -ne 0) { Fail "git commit failed" } } else { Write-Host "No changes to commit." }

# collect credentials
$username = Read-Host "GitHub username"
Write-Host "Enter your Personal Access Token (input will be hidden)"
$securePAT = Read-Host -AsSecureString
# convert secure string to plain (kept only briefly)
$ptr = [System.Runtime.InteropServices.Marshal]::SecureStringToBSTR($securePAT)
$pat = [System.Runtime.InteropServices.Marshal]::PtrToStringBSTR($ptr)
[System.Runtime.InteropServices.Marshal]::ZeroFreeBSTR($ptr)

# save existing remote (if any)
$orig = $null
try { $orig = (& git remote get-url origin 2>$null) } catch { $orig = $null }

# build temporary remote URL embedding credentials
$encUser = [System.Uri]::EscapeDataString($username)
$encPAT = [System.Uri]::EscapeDataString($pat)
$tempUrl = $RemoteUrl -replace '^https://', "https://$encUser`:$encPAT@"

# set temporary remote
if ($orig) {
    Write-Host "Temporarily overwriting origin to use PAT (will be restored)"
    & git remote set-url origin $tempUrl
    if ($LASTEXITCODE -ne 0) { Fail "Failed to set temporary origin URL (exit code $LASTEXITCODE)" }
} else {
    Write-Host "Adding origin with PAT (will be removed after push)"
    & git remote add origin $tempUrl
    if ($LASTEXITCODE -ne 0) { Fail "Failed to add temporary origin URL" }
}

# determine branch
$branch = $null
try { $branch = (& git symbolic-ref --short HEAD 2>$null) -replace "\r|\n", '' } catch { $branch = $null }
if (-not $branch) { & git checkout -b main; $branch = 'main' }
Write-Host "Pushing branch: $branch"

# push
& git push -u origin $branch
$pushExit = $LASTEXITCODE

# restore remote
if ($orig) {
    Write-Host "Restoring original origin URL"
    & git remote set-url origin $orig
    if ($LASTEXITCODE -ne 0) { Write-Warning "Failed to restore original origin URL (exit code $LASTEXITCODE)" }
} else {
    Write-Host "Removing temporary origin remote"
    & git remote remove origin
}

# clear sensitive variables
$pat = $null
try { $null = $securePAT } catch { }

if ($pushExit -ne 0) { Fail "git push failed (exit code $pushExit)" }

Write-Host "Push complete. Remote restored."
Pop-Location
