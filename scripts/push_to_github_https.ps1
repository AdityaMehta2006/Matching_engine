<#
HTTPS push helper. Use this if you prefer HTTPS instead of SSH.
Usage:
  cd C:\Users\ASUS\Projects\matching_engine_cpp
  .\scripts\push_to_github_https.ps1 -RemoteUrl "https://github.com/AdityaMehta2006/Matching_engine.git" -CommitMessage "Add project files"
This script will prompt for credentials if needed. For automation, configure Git Credential Manager or use a PAT.
#>
param(
    [string]$RemoteUrl = "https://github.com/AdityaMehta2006/Matching_engine.git",
    [string]$CommitMessage = "Add C++ matching engine, server, bridge, tests and scripts",
    [switch]$ForceRemote
)

function Fail([string]$m) { Write-Error $m; exit 1 }

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
# stage & commit
& git add -A
if ($LASTEXITCODE -ne 0) { Fail "git add failed" }
$diff = (& git diff --cached --name-only) 2>$null
if ($diff) {
    & git commit -m "$CommitMessage"
    if ($LASTEXITCODE -ne 0) { Fail "git commit failed" }
} else {
    Write-Host "No changes to commit."
}

# remote
$existing = $null
try { $existing = (& git remote get-url origin 2>$null) } catch { $existing = $null }
if ($existing) {
    Write-Host "Existing origin: $existing"
    if ($existing -ne $RemoteUrl) {
        if ($ForceRemote) { & git remote remove origin; & git remote add origin $RemoteUrl } else { Write-Host "Keeping existing origin. Use -ForceRemote to overwrite." }
    }
} else { & git remote add origin $RemoteUrl }

# branch
$branch = $null
try { $branch = (& git symbolic-ref --short HEAD 2>$null) -replace "\r|\n", '' } catch { $branch = $null }
if (-not $branch) { & git checkout -b main; $branch = 'main' }

# push
Write-Host "Pushing to $RemoteUrl ($branch)"
& git push -u origin $branch
if ($LASTEXITCODE -ne 0) { Fail "git push failed. If this is due to authentication, create a Personal Access Token (PAT) and use it when prompted." }

Write-Host "Done."
Pop-Location
