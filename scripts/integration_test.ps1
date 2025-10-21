param(
    [string]$ExePath = "C:\Users\ASUS\BuildTools_build\matching_engine_cpp_build\Release\me_server.exe",
    [int]$Port = 8080
)

Write-Host "Integration test: starting server from $ExePath"
$proc = Start-Process -FilePath $ExePath -PassThru
Start-Sleep -Milliseconds 300

function PostJson($path, $obj){
    $json = ($obj | ConvertTo-Json -Compress)
    $uri = "http://localhost:$Port$path"
    Write-Host "POST $uri -> $json"
    $r = Invoke-WebRequest -Uri $uri -Method Post -Body $json -ContentType 'application/json' -UseBasicParsing
    return $r.Content
}

function GetJson($path){
    $uri = "http://localhost:$Port$path"
    Write-Host "GET $uri"
    $r = Invoke-WebRequest -Uri $uri -Method Get -UseBasicParsing
    return $r.Content
}

# submit buy order
$buy = @{ side = 'BUY'; symbol = 'BTCUSD'; quantity = 1; price = 100.0 }
$bresp = PostJson '/order' $buy
Write-Host "buy response: $bresp"

# submit crossing sell order
$sell = @{ side = 'SELL'; symbol = 'BTCUSD'; quantity = 1; price = 100.0 }
$sresp = PostJson '/order' $sell
Write-Host "sell response: $sresp"

Start-Sleep -Milliseconds 200

$trades = GetJson '/trades'
Write-Host "trades: $trades"

$md = GetJson '/md'
Write-Host "md: $md"

# shutdown server
$sr = PostJson '/shutdown' @{}
Write-Host "shutdown resp: $sr"

# ensure process exited
Start-Sleep -Milliseconds 200
if(!$proc.HasExited){
    Write-Host "Killing server process"
    $proc | Stop-Process -Force
}

Write-Host "Integration test complete"
