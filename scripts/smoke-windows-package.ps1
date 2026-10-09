# Silicon LogiX / SLX ResilienceLab
# Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
# Author: Marco Pezzullo
# License: Silicon LogiX Evaluation License 1.0. See LICENSE.
# Project smoke windows package implementation.

param([string]$BundleDirectory='dist/SLX-ResilienceLab-0.4.0',[string]$EvidenceDirectory='build/portable-evidence-0.4.0')
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$projectRoot=[IO.Path]::GetFullPath((Split-Path -Parent $PSScriptRoot))
$bundlePath=[IO.Path]::GetFullPath((Join-Path $projectRoot $BundleDirectory))
$evidencePath=[IO.Path]::GetFullPath((Join-Path $projectRoot $EvidenceDirectory))
$prefix=$projectRoot.TrimEnd('\','/')+[IO.Path]::DirectorySeparatorChar
if(-not $evidencePath.StartsWith($prefix,[StringComparison]::OrdinalIgnoreCase)){throw 'Evidence must stay inside the workspace'}
New-Item -ItemType Directory -Path $evidencePath -Force | Out-Null
$manifest=Get-Content -LiteralPath "$bundlePath/PORTABLE-MANIFEST.json" -Raw | ConvertFrom-Json
foreach($item in $manifest.files.PSObject.Properties){if((Get-FileHash -LiteralPath "$bundlePath/$($item.Name)" -Algorithm SHA256).Hash.ToLowerInvariant() -ne $item.Value){throw "Bundle hash mismatch: $($item.Name)"}}
$previousPath=$env:PATH
try {
    $env:PATH="$env:SystemRoot/System32;$env:SystemRoot"
    $env:QT_FORCE_STDERR_LOGGING='1'
    $env:QT_PLUGIN_PATH=$null; $env:QML_IMPORT_PATH=$null; $env:QML2_IMPORT_PATH=$null
    foreach($name in @('sensor-stuck','main-loss','measurement-loss')){
        $arguments="--run --scenario `"$bundlePath/scenarios/$name.json`" --report `"$evidencePath/$name.html`""
        $probe=Start-Process -FilePath "$bundlePath/slx-resiliencelab.exe" -ArgumentList $arguments -WindowStyle Hidden -Wait -PassThru -RedirectStandardError "$evidencePath/$name-errors.txt"
        if($probe.ExitCode -ne 0){throw "Portable scenario failed: $name / $($probe.ExitCode)"}
        $result=Get-Content -LiteralPath "$evidencePath/$name.json" -Raw | ConvertFrom-Json
        if($result.samples.Count -ne 600){throw 'Incorrect number of executed samples'}
        if($result.bus_trace.Count -lt 1000){throw 'Executed transport trace missing'}
        if($name -eq 'sensor-stuck' -and ($result.metrics.final_error_pct -gt 0.5 -or $result.metrics.stops -ne 0)){throw 'Sensor redundancy regression'}
        if($name -eq 'main-loss' -and ($result.final.owner -ne 'backup' -or $result.metrics.takeovers -ne 1)){throw 'Failover regression'}
        if($name -eq 'measurement-loss' -and ($result.final.motor -ne 0 -or $result.final.quorum)){throw 'No-quorum stop regression'}
        Write-Output "PASS portable $name"
    }
    foreach($control in @('slx','customer')){
        $pluginArgument=if($control -eq 'customer'){" --plugin `"$bundlePath/SLX.CustomerController.Example.dll`""}else{''}
        $arguments="--suite$pluginArgument --report `"$evidencePath/campaign-$control.html`""
        $probe=Start-Process -FilePath "$bundlePath/slx-resiliencelab.exe" -ArgumentList $arguments -WindowStyle Hidden -Wait -PassThru -RedirectStandardError "$evidencePath/campaign-$control-errors.txt"
        if($probe.ExitCode -ne 0){throw "Portable campaign failed: $control / $($probe.ExitCode)"}
        $result=Get-Content -LiteralPath "$evidencePath/campaign-$control.json" -Raw | ConvertFrom-Json
        if(-not $result.all_passed -or $result.passed -ne 3 -or $result.runs.Count -ne 3){throw 'Campaign acceptance results missing'}
        if($control -eq 'customer' -and $result.customer_plugin_sha256.Length -ne 64){throw 'Customer DLL provenance missing'}
        foreach($run in $result.runs){if($run.result.samples.Count -ne 600){throw 'Campaign samples missing'}}
        Write-Output "PASS portable campaign $control"
    }
    $failedScenarios="$evidencePath/failed-scenarios"
    New-Item -ItemType Directory -Path $failedScenarios -Force | Out-Null
    foreach($name in @('sensor-stuck','main-loss','measurement-loss')){
        $testScenario=Get-Content -LiteralPath "$bundlePath/scenarios/$name.json" -Encoding UTF8 -Raw | ConvertFrom-Json
        if($name -eq 'sensor-stuck'){$testScenario.faults+=@([pscustomobject]@{at_s=0.5;kind='actuator_jam'})}
        [IO.File]::WriteAllText("$failedScenarios/$name.json",($testScenario|ConvertTo-Json -Depth 20),[Text.UTF8Encoding]::new($false))
    }
    $probe=Start-Process -FilePath "$bundlePath/slx-resiliencelab.exe" -ArgumentList "--suite --scenario-dir `"$failedScenarios`" --report `"$evidencePath/campaign-fail.html`"" -WindowStyle Hidden -Wait -PassThru -RedirectStandardError "$evidencePath/campaign-fail-errors.txt"
    if($probe.ExitCode -ne 1){throw "Campaign failure must return CI exit code 1, received $($probe.ExitCode)"}
    $result=Get-Content -LiteralPath "$evidencePath/campaign-fail.json" -Raw | ConvertFrom-Json
    if($result.all_passed -or $result.passed -ne 2){throw 'Failed criterion not reflected in campaign'}
    Write-Output 'PASS portable failing criterion / CI exit code 1'
    # Use the deployed transport and GUI session without the Qt development kit.
    $reservation=[Net.Sockets.TcpListener]::new([Net.IPAddress]::Loopback,0)
    $reservation.Start()
    $benchPort=$reservation.LocalEndpoint.Port
    $reservation.Stop()
    $benchProcess=Start-Process -FilePath "$bundlePath/slx-resiliencelab.exe" -ArgumentList "--api --api-port $benchPort" -WindowStyle Hidden -PassThru -RedirectStandardError "$evidencePath/live-api-errors.txt"
    $benchClient=$null
    try {
        for($attempt=0;$attempt -lt 60;$attempt++){
            $benchClient=[Net.Sockets.TcpClient]::new()
            try {$benchClient.Connect('127.0.0.1',$benchPort);break}
            catch {$benchClient.Dispose();$benchClient=$null;Start-Sleep -Milliseconds 50}
        }
        if($null -eq $benchClient){throw 'Portable local API did not start'}
        $benchClient.ReceiveTimeout=3000
        $benchClient.SendTimeout=3000
        $benchReader=[IO.StreamReader]::new($benchClient.GetStream(),[Text.UTF8Encoding]::new($false))
        $benchWriter=[IO.StreamWriter]::new($benchClient.GetStream(),[Text.UTF8Encoding]::new($false))
        $benchWriter.AutoFlush=$true
        $script:benchRequestId=0
        function Invoke-BenchRequest([hashtable]$Request){
            $script:benchRequestId++
            $Request.id=$script:benchRequestId
            $benchWriter.WriteLine(($Request|ConvertTo-Json -Compress -Depth 10))
            $reply=$benchReader.ReadLine()|ConvertFrom-Json
            if(-not $reply.ok -or $reply.id -ne $script:benchRequestId){throw 'Portable API rejected a request or returned an unmatched reply'}
            return $reply.data
        }
        Invoke-BenchRequest @{op='live'} | Out-Null
        Invoke-BenchRequest @{op='pause'} | Out-Null
        Invoke-BenchRequest @{op='target';value=80} | Out-Null
        for($step=0;$step -lt 50;$step++){Invoke-BenchRequest @{op='step'} | Out-Null}
        Invoke-BenchRequest @{op='fault';kind='sensor_stuck';sensors=@(0,1,2)} | Out-Null
        for($step=0;$step -lt 110;$step++){Invoke-BenchRequest @{op='step'} | Out-Null}
        $observed=Invoke-BenchRequest @{op='read'}
        $inputs=Invoke-BenchRequest @{op='measurements'}
        if($observed.motor -ne 0 -or $observed.target -ne 80 -or $inputs.diagnostic -ne 'no_measured_progress' -or -not $inputs.quorum){throw 'Portable common sensor fault did not produce the executed progress stop'}
        if($inputs.PSObject.Properties.Name -contains 'position' -or $inputs.PSObject.Properties.Name -contains 'jammed'){throw 'Controller-facing API exposes plant truth'}
        if($observed.position -le ($inputs.fused_percent+5)){throw 'Plant truth and frozen measurements were not kept separate'}
        [IO.File]::WriteAllText("$evidencePath/live-api.json",(@{copyright='Copyright (c) 2026 Marco Pezzullo / Silicon LogiX';operator=$observed;controller=$inputs}|ConvertTo-Json -Depth 30),[Text.UTF8Encoding]::new($false))
        Write-Output 'PASS portable live API / manual target / three frozen sensors / truth-free controller reads'
    } finally {
        if($null -ne $benchClient){$benchClient.Dispose()}
        $benchProcess.Refresh()
        if(-not $benchProcess.HasExited){Stop-Process -Id $benchProcess.Id}
    }
    $probe=Start-Process -FilePath "$bundlePath/slx-resiliencelab.exe" -ArgumentList "--scenario 0 --capture-time 3 --capture `"$evidencePath/desktop.png`"" -WindowStyle Hidden -Wait -PassThru -RedirectStandardError "$evidencePath/desktop-errors.txt"
    if($probe.ExitCode -ne 0){throw "Portable renderer failed: $($probe.ExitCode)"}
    Write-Output 'PASS portable Qt Quick + OpenGL desktop (development kit excluded from PATH)'
    foreach($view in @('about','license','notices')){
        $probe=Start-Process -FilePath "$bundlePath/slx-resiliencelab.exe" -ArgumentList "--$view --capture `"$evidencePath/$view.png`"" -WindowStyle Hidden -Wait -PassThru -RedirectStandardError "$evidencePath/$view-errors.txt"
        if($probe.ExitCode -ne 0 -or -not(Test-Path -LiteralPath "$evidencePath/$view.png")){throw "Portable dialog failed: $view"}
        Write-Output "PASS portable $view"
    }
} finally {$env:PATH=$previousPath}
