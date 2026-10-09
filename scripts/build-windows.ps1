# Silicon LogiX / SLX ResilienceLab
# Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
# Author: Marco Pezzullo
# License: Silicon LogiX Evaluation License 1.0. See LICENSE.
# Project build windows implementation.

param([string]$QtRoot='C:/Qt/6.12.0/mingw_64',[string]$ToolchainBin='C:/Qt/Tools/mingw1310_64/bin',[string]$BuildDirectory='build/internal')
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$projectRoot=Split-Path -Parent $PSScriptRoot
$previousPath=$env:PATH
try {
    $env:PATH="$QtRoot/bin;$ToolchainBin;$previousPath"
    & cmake -S $projectRoot -B "$projectRoot/$BuildDirectory" -G Ninja "-DCMAKE_PREFIX_PATH=$QtRoot" "-DCMAKE_CXX_COMPILER=$ToolchainBin/g++.exe" -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
    if($LASTEXITCODE -ne 0){throw 'CMake configuration failed'}
    & cmake --build "$projectRoot/$BuildDirectory" --parallel 4
    if($LASTEXITCODE -ne 0){throw 'Build failed'}
    & ctest --test-dir "$projectRoot/$BuildDirectory" --output-on-failure
    if($LASTEXITCODE -ne 0){throw 'Required tests failed'}
} finally {$env:PATH=$previousPath}
