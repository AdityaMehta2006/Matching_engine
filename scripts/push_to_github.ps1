# Safe push script for matching_engine_cpp
# Usage: run this from PowerShell on your machine
#   cd C:\Users\ASUS\Projects\matching_engine_cpp
#   .\scripts\push_to_github.ps1 -RemoteUrl git@github.com:AdityaMehta2006/Matching_engine.git -CommitMessage "Your commit message"

param(
    [string]$RemoteUrl = "git@github.com:AdityaMehta2006/Matching_engine.git",
    [string]$CommitMessage = "Add C++ matching engine, server, bridge, tests and scripts",
    [switch]$ForceRemote
)

function Fail([string]$msg) {
    Write-Error $msg
    exit 1
}

# ensure git exists
try {
    $git = & git --version 2>$null
} catch {
    Fail "git is not installed or not on PATH. Install Git for Windows and ensure 'git' is on your PATH."
}
if (-not $git) {
    Fail "git is not installed or not on PATH. Install Git for Windows and ensure 'git' is on your PATH."
}
Write-Host "Detected: $git"

# move to repo root
Push-Location -Path (Join-Path $PSScriptRoot "..")
$RepoRoot = Get-Location
Write-Host "Repo root: $RepoRoot"

# ensure we are in a git repo
$inside = (& git rev-parse --is-inside-work-tree) -eq 'true' 2>$null
if (-not $inside) {
    Write-Host "No git repository detected. Initializing a new repo..."
    & git init
    if ($LASTEXITCODE -ne 0) { Fail "git init failed (exit code $LASTEXITCODE)" }
}

# show status
Write-Host "Git status (porcelain):"
& git status --porcelain

git add -A || Fail "git add failed"
# stage everything
& git add -A
if ($LASTEXITCODE -ne 0) { Fail "git add failed (exit code $LASTEXITCODE)" }

# commit (if there are staged changes)
$diff = (& git diff --cached --name-only)
if ($diff) {
    Write-Host "Committing changes:"
    Write-Host $diff
    & git commit -m "$CommitMessage"
    if ($LASTEXITCODE -ne 0) { Fail "git commit failed (exit code $LASTEXITCODE)" }
} else {
    Write-Host "No changes to commit."
}

# configure remote
$existing = (& git remote get-url origin 2>$null) 2>$null
if ($existing) {
    Write-Host "Existing origin remote: $existing"
    if ($existing -ne $RemoteUrl) {
        if ($ForceRemote) {
            Write-Host "Overwriting origin remote with $RemoteUrl"
            git remote remove origin
            git remote add origin $RemoteUrl
        } else {
            Write-Host "Keeping existing origin. Use -ForceRemote to overwrite."
        }
    }
} else {
    Write-Host "Adding origin remote: $RemoteUrl"
    & git remote add origin $RemoteUrl
    if ($LASTEXITCODE -ne 0) { Fail "Failed to add remote (exit code $LASTEXITCODE)" }
}

# determine current branch
$branch = (& git symbolic-ref --short HEAD 2>$null) -replace "\r|\n", ''
if (-not $branch) {
    Write-Host "No branch detected. Creating 'main' branch."
    & git checkout -b main
    if ($LASTEXITCODE -ne 0) { Fail "Failed to create main branch (exit code $LASTEXITCODE)" }
    $branch = 'main'
}
Write-Host "Current branch: $branch"

# push and set upstream
Write-Host "Pushing to origin/$branch..."
& git push -u origin $branch
if ($LASTEXITCODE -ne 0) { Fail "git push failed (exit code $LASTEXITCODE). Check your network/SSH credentials or remote permissions." }

Write-Host "Push complete. Verify on GitHub."

Pop-Location
