<#
File: build_win.ps1
用法:
  powershell -ExecutionPolicy Bypass -File .\build_win.ps1 -BuildDir build -BuildType Release -Generator "Visual Studio 17 2022"
#>
param(
  [string]$BuildDir = "build",
  [string]$BuildType = "Release",
  [string]$Generator = "",
  [switch]$DryRun
)

Write-Host "Platform: Windows (PowerShell)"
Write-Host "BuildDir: $BuildDir  BuildType: $BuildType  Generator: $Generator"
$jobs = [Environment]::ProcessorCount

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
  Write-Error "cmake not found in PATH"
  exit 1
}

if (-not (Test-Path $BuildDir)) {
  New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

$cmakeArgs = @("-S", ".", "-B", $BuildDir, "-DCMAKE_BUILD_TYPE=$BuildType")
if ($Generator -ne "") {
  $cmakeArgs += "-G"
  $cmakeArgs += $Generator
}

cmake @cmakeArgs
# 使用并行构建，MSBuild 语法为 /m:##
cmake --build $BuildDir -- /m:$jobs
Write-Host "Build finished: $BuildDir"

# --------------------- Qt deployment for Windows (windeployqt enforced) ---------------------
if ($env:AUTO_DEPLOY -eq "0") {
  Write-Host "AUTO_DEPLOY=0, skipping Qt deployment"
  exit 0
}

$windeployCmd = Get-Command windeployqt -ErrorAction SilentlyContinue
if ($null -eq $windeployCmd) {
  Write-Error "Error: windeployqt not found in PATH. Install Qt and ensure windeployqt is available."
  exit 1
}

Write-Host "Starting Qt6 dependency deployment using windeployqt..."

# prefer Release subdir
$searchDir = $BuildDir
if (Test-Path (Join-Path $BuildDir 'Release')) { $searchDir = Join-Path $BuildDir 'Release' }

$exes = Get-ChildItem -Path $searchDir -Recurse -Filter *.exe -ErrorAction SilentlyContinue | Where-Object { $_.FullName -notmatch "CMakeFiles" }
if ($exes.Count -eq 0) {
  Write-Host "No executables found in $searchDir to run windeployqt on. Exiting deploy step."
  exit 0
}

foreach ($exe in $exes) {
  Write-Host "Running windeployqt on $($exe.FullName)"
  if ($DryRun) { Write-Host "DRY RUN: windeployqt $($exe.FullName) -verbose=1" } else { windeployqt $exe.FullName -verbose=1 }
}

Write-Host "windeployqt deployment finished."
