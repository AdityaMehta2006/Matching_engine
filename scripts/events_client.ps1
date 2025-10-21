param(
    [string]$BaseUrl = 'http://localhost:8080',
    [int]$Polls = 5,
    [int]$DelayMs = 200
)
for($i=0;$i -lt $Polls; $i++){
    try{
        $r = Invoke-WebRequest -Uri "$BaseUrl/events/poll" -Method Get -UseBasicParsing -TimeoutSec 12 -ErrorAction SilentlyContinue
        if($r -and $r.StatusCode -eq 200){
            Write-Host "EVENTS: $($r.Content)"
        } elseif ($r -and $r.StatusCode -eq 204) {
            Write-Host "No events"
        } else {
            Write-Host "No response or error"
        }
    } catch { Write-Host "poll error: $_" }
    Start-Sleep -Milliseconds $DelayMs
}
