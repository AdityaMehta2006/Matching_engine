<#
Starts me_server, ws_bridge, and ws_client in separate PowerShell windows (foreground)
Usage: run this script from the repository root or via PowerShell execution.
#>
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition

function Start-Window([string]$title, [string]$command){
  $ps = "powershell -NoExit -NoProfile -Command \"Set-Location '$scriptDir'; $command\""
  Write-Host "Starting $title"
  Start-Process -FilePath powershell -ArgumentList "-NoExit","-NoProfile","-Command","Set-Location '$scriptDir'; $command" -WindowStyle Normal
}

Start-Window -title 'me_server' -command "Start-Process -FilePath (Resolve-Path '..\\build\\Release\\me_server.exe')"
Start-Sleep -Milliseconds 300
Start-Window -title 'ws_bridge' -command "npm run bridge"
Start-Sleep -Milliseconds 200
Start-Window -title 'ws_client' -command "npm run client"
