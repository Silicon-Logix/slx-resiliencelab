# Silicon LogiX / SLX ResilienceLab
# Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
# Author: Marco Pezzullo
# License: Silicon LogiX Evaluation License 1.0. See LICENSE.
# Project package windows implementation.

param(
    [string]$QtRoot='C:/Qt/6.12.0/mingw_64',
    [string]$ToolchainBin='C:/Qt/Tools/mingw1310_64/bin',
    [string]$BuildDirectory='build/internal',
    [string]$OutputDirectory='dist/SLX-ResilienceLab-0.4.0',
    [string]$RuntimeSourceDirectory='dist/runtime-sources-0.1.0',
    [switch]$CreateArchive
)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$projectRoot=[IO.Path]::GetFullPath((Split-Path -Parent $PSScriptRoot))
function ProjectPath([string]$Path){if([IO.Path]::IsPathRooted($Path)){return [IO.Path]::GetFullPath($Path)};return [IO.Path]::GetFullPath((Join-Path $projectRoot $Path))}
$outputPath=ProjectPath $OutputDirectory
$prefix=$projectRoot.TrimEnd('\','/')+[IO.Path]::DirectorySeparatorChar
if(-not $outputPath.StartsWith($prefix,[StringComparison]::OrdinalIgnoreCase)){throw 'Package must stay inside the workspace'}
if(Test-Path -LiteralPath $outputPath){throw 'Output already exists; choose a new directory'}
$buildPath=ProjectPath $BuildDirectory
$sourcePath=ProjectPath $RuntimeSourceDirectory
if(-not(Test-Path -LiteralPath "$sourcePath/manifest.json")){throw 'Prepare pinned corresponding sources first: tools/prepare-runtime-sources.py'}
$previousPath=$env:PATH
try {
    $env:PATH="$QtRoot/bin;$ToolchainBin;$previousPath"
    New-Item -ItemType Directory -Path $outputPath -Force | Out-Null
    foreach($name in @('slx-resiliencelab.exe','SiliconLogiX.ResilienceLab.Core.dll','SLX.CustomerController.Example.dll')){
        Copy-Item -LiteralPath "$buildPath/$name" -Destination $outputPath
        & "$ToolchainBin/strip.exe" --strip-unneeded "$outputPath/$name"
        if($LASTEXITCODE -ne 0){throw 'Binary stripping failed'}
    }
    & "$QtRoot/bin/windeployqt.exe" --release --compiler-runtime --no-translations --no-system-d3d-compiler --no-opengl-sw --skip-plugin-types qmltooling,generic --include-plugins qsvg,qsvgicon --verbose 0 --qmldir "$projectRoot/qml" --dir $outputPath "$outputPath/slx-resiliencelab.exe"
    if($LASTEXITCODE -ne 0){throw 'Qt deployment failed'}
    foreach($name in @('README.md','CHANGELOG.md','SECURITY.md','LICENSE','THIRD_PARTY_NOTICES.md','THIRD_PARTY_SOURCE.md')){Copy-Item -LiteralPath "$projectRoot/$name" -Destination $outputPath}
    foreach($name in @('licenses','docs','scenarios')){Copy-Item -LiteralPath "$projectRoot/$name" -Destination "$outputPath/$name" -Recurse}
    New-Item -ItemType Directory -Path "$outputPath/assets" -Force | Out-Null
    Copy-Item -LiteralPath "$projectRoot/assets/screenshots" -Destination "$outputPath/assets/screenshots" -Recurse
    New-Item -ItemType Directory -Path "$outputPath/examples","$outputPath/sdk" -Force | Out-Null
    foreach($example in @('customer-controller','bench-client','board-client')){Copy-Item -LiteralPath "$projectRoot/examples/$example" -Destination "$outputPath/examples/$example" -Recurse}
    Copy-Item -LiteralPath "$projectRoot/core/include/slx" -Destination "$outputPath/sdk/slx" -Recurse
    $toolchainRoot=Split-Path -Parent $ToolchainBin
    $runtimeLicenses=@{'GCC-GPLv3.txt'='gcc/COPYING3';'GCC-runtime-exception.txt'='gcc/COPYING.RUNTIME';'Winpthreads-LICENSE.txt'='winpthreads/COPYING';'MinGW-w64-runtime-LICENSE.txt'='mingw-w64/COPYING.MinGW-w64-runtime.txt'}
    foreach($name in $runtimeLicenses.Keys){Copy-Item -LiteralPath "$toolchainRoot/licenses/$($runtimeLicenses[$name])" -Destination "$outputPath/licenses/$name"}
    foreach($file in Get-ChildItem "$QtRoot/sbom" -Filter '*.spdx' | Where-Object Name -Match '^(qtbase|qtdeclarative|qtsvg|qtshadertools|qtserialport)-'){
        $spdx=[IO.File]::ReadAllText($file.FullName).Replace($projectRoot.Replace('\','/'),'/project').Replace($projectRoot,'/project')
        [IO.File]::WriteAllText("$outputPath/licenses/$($file.Name)",$spdx,[Text.UTF8Encoding]::new($false))
    }
    $manifest=Get-Content -LiteralPath "$sourcePath/manifest.json" -Raw | ConvertFrom-Json
    foreach($record in $manifest.source_archives){$archive=Get-Item -LiteralPath "$sourcePath/$($record.file)";if($archive.Length -ne $record.bytes -or (Get-FileHash -LiteralPath $archive.FullName -Algorithm SHA256).Hash.ToLowerInvariant() -ne $record.sha256){throw 'Corresponding source archive changed'}}
    foreach($record in $manifest.runtime_files.PSObject.Properties){if((Get-FileHash -LiteralPath "$outputPath/$($record.Name)" -Algorithm SHA256).Hash.ToLowerInvariant() -ne $record.Value){throw "Runtime does not match source manifest: $($record.Name)"}}
    Copy-Item -LiteralPath $sourcePath -Destination "$outputPath/runtime-sources" -Recurse
    Set-Content -LiteralPath "$outputPath/START-HERE.txt" -Encoding UTF8 -Value @'
SLX ResilienceLab / Silicon LogiX
Copyright (c) 2026 Marco Pezzullo / Silicon LogiX. License: see LICENSE.
Open slx-resiliencelab.exe. Getting started explains the purpose and guided tests.
Run a test and compare expected behavior with the observed response.
Guidance evaluates executed data, including failed criteria.
Use Laboratory > New live bench for a nominal five-minute manual session.
Change requested opening, Close/Open, freeze/drop/restore one or all sensors.
Three agreeing frozen sensors can keep quorum; motion reveals missing progress.
API and boards: connect USB serial or enable the local TCP API and read position.
Board firmware must implement the documented JSON-lines contract.
Host/DLL code computes motor drive; full physical firmware control is a custom integration.
1x follows PC time, 0.5x slows observation and 4x accelerates. No hard realtime guarantee.
Campaign: run three tests with PASS/FAIL and complete reports.
Batch per CI: slx-resiliencelab.exe --suite --report campaign.html
Custom test bench: commissioned integrations, local technical request and contact page.
About includes version, evaluation license and third-party notices.
Cutaway shows the rotor following actual plant position.
Drag to rotate, wheel to zoom, Center to restore the camera.
Scenarios: parameters, timeline, seed, JSON save and customer DLL.
Comparison: replay two controls and export HTML/JSON/CSV.
Batch report: slx-resiliencelab.exe --run --scenario scenarios/sensor-stuck.json --report report.html
Compiled proprietary core. Software demonstrator with no safety certification.
Runtime licenses and corresponding sources are in runtime-sources.
'@
    $env:PATH="$env:SystemRoot/System32;$env:SystemRoot"
    $probe=Start-Process -FilePath "$outputPath/slx-resiliencelab.exe" -ArgumentList '--smoke-test' -WorkingDirectory $outputPath -WindowStyle Hidden -Wait -PassThru
    if($probe.ExitCode -ne 0){throw "Portable startup failed: $($probe.ExitCode)"}
    $release=[ordered]@{schema='siliconlogix.portable-release.v1';version='0.4.0';copyright='Copyright (c) 2026 Marco Pezzullo / Silicon LogiX';core_abi=1;files=[ordered]@{}}
    foreach($file in Get-ChildItem -LiteralPath $outputPath -Recurse -File | Sort-Object FullName){$relative=$file.FullName.Substring($outputPath.TrimEnd('\','/').Length+1).Replace('\','/');$release.files[$relative]=(Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash.ToLowerInvariant()}
    [IO.File]::WriteAllText("$outputPath/PORTABLE-MANIFEST.json",($release|ConvertTo-Json -Depth 5)+"`n",[Text.UTF8Encoding]::new($false))
    if($CreateArchive){$zipPath=$outputPath+'.zip';if(Test-Path -LiteralPath $zipPath){throw 'Archive already exists'};Compress-Archive -LiteralPath $outputPath -DestinationPath $zipPath -CompressionLevel Optimal}
    Write-Output "Portable application: $outputPath/slx-resiliencelab.exe"
} finally {$env:PATH=$previousPath}
