param(
  [string]$ServerExe
)

# Determine script directory robustly
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
if(-not $ServerExe){
  $ServerExe = Join-Path $scriptDir "..\build\Release\me_server.exe"
}
$ServerExe = Resolve-Path -Path $ServerExe -ErrorAction SilentlyContinue
if(-not $ServerExe){
  Write-Error "Server executable not found at expected location. Build the project first."; exit 1
}

Write-Host "Starting me_server as job... (exe: $ServerExe)"
$srv = Start-Job -ScriptBlock { param($p) & $p } -ArgumentList $ServerExe -Name me_server_demo
Start-Sleep -Seconds 1

Write-Host "Starting WS bridge (npm run bridge)..."
Start-Job -ScriptBlock { param($d) Set-Location $d; npm run bridge } -ArgumentList $scriptDir -Name ws_bridge_demo
Start-Sleep -Milliseconds 300

Write-Host "Starting WS client (npm run client)..."
Start-Job -ScriptBlock { param($d) Set-Location $d; npm run client } -ArgumentList $scriptDir -Name ws_client_demo
Start-Sleep -Seconds 1

Write-Host "Posting a SELL and BUY to create a trade"
$body1 = @{ side='SELL'; quantity=1; price=100; symbol='DEMO' } | ConvertTo-Json
Invoke-RestMethod -Uri 'http://localhost:8080/order' -Method Post -Body $body1 -ContentType 'application/json' | ConvertTo-Json
Start-Sleep -Milliseconds 200
$body2 = @{ side='BUY'; quantity=1; price=100; symbol='DEMO' } | ConvertTo-Json
Invoke-RestMethod -Uri 'http://localhost:8080/order' -Method Post -Body $body2 -ContentType 'application/json' | ConvertTo-Json

Start-Sleep -Seconds 1

Write-Host "Collecting job outputs..."
Get-Job | ForEach-Object { Write-Host "Job:" $_.Id $_.Name $_.State; Receive-Job -Id $_.Id -Keep }

Write-Host "Shutting down server (POST /shutdown)"
Invoke-RestMethod -Uri 'http://localhost:8080/shutdown' -Method Post | ConvertTo-Json

Start-Sleep -Seconds 1
Get-Job | ForEach-Object { Write-Host "Final Job:" $_.Id $_.Name $_.State; Receive-Job -Id $_.Id -Keep }

Write-Host "Demo finished"
