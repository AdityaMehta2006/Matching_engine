<#
Stops me_server, ws_bridge, and ws_client started by start-all/demo scripts.
#>
Write-Host "Stopping background jobs named me_server_demo, ws_bridge_demo, ws_client_demo..."
Get-Job -Name me_server_demo,ws_bridge_demo,ws_client_demo -State Running -ErrorAction SilentlyContinue | Stop-Job -Force | Remove-Job -Force

Write-Host "Killing me_server.exe, node (bridge/client) processes if present..."
Get-Process -Name me_server -ErrorAction SilentlyContinue | ForEach-Object { Stop-Process -Id $_.Id -Force }
Get-Process -Name node -ErrorAction SilentlyContinue | ForEach-Object { Stop-Process -Id $_.Id -Force }

Write-Host "Stopped."
