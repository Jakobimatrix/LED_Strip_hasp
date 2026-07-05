# Decode trace script
# Place this script next to trace.txt and double-click to run.
# It searches for the ELF under .pio/build (prefers .pio/build/debug/firmware.elf),
# finds an addr2line binary under PlatformIO packages or PATH, extracts hex addresses
# from trace.txt and writes a decoded, human-readable stack trace to trace_decoded.txt.

param()

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$root = $scriptDir

$traceFile = Join-Path $root 'trace.txt'
if (-not (Test-Path $traceFile)) {
  Write-Host "trace.txt not found at $traceFile"
  exit 1
}

# Find ELF: prefer .pio/build/debug/firmware.elf
$elfDefault = Join-Path $root '.pio\build\debug\firmware.elf'
if (Test-Path $elfDefault) {
  $elfPath = $elfDefault
}
else {
  $buildDir = Join-Path $root '.pio\build'
  if (Test-Path $buildDir) {
    $elfs = Get-ChildItem -Path $buildDir -Recurse -Include *.elf -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending
    if ($elfs -and $elfs.Count -gt 0) {
      $elfPath = $elfs[0].FullName
    }
  }
}

if (-not $elfPath) {
  Write-Host "ELF not found under .pio/build. Please build the project first." -ForegroundColor Red
  exit 1
}

# Find addr2line: search PlatformIO packages then PATH
$addr2line = $null
$pioPackages = Join-Path $env:USERPROFILE ".platformio\packages"
if (Test-Path $pioPackages) {
  try {
    $found = Get-ChildItem -Path $pioPackages -Recurse -Include '*addr2line*' -File -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($found) { $addr2line = $found.FullName }
  }
  catch {
    # ignore
  }
}

if (-not $addr2line) {
  $which = Get-Command addr2line -ErrorAction SilentlyContinue
  if ($which) { $addr2line = $which.Path }
}

if (-not $addr2line) {
  Write-Host "addr2line tool not found. Ensure PlatformIO toolchain is installed or add addr2line to PATH." -ForegroundColor Red
  exit 1
}

# Read trace file and extract hex addresses like 0x420019ba
$traceText = Get-Content $traceFile -Raw
$matches = [regex]::Matches($traceText, '0x[0-9A-Fa-f]{6,16}')
$addrs = @()
foreach ($m in $matches) {
  if (-not ($addrs -contains $m.Value)) { $addrs += $m.Value }
}

if ($addrs.Count -eq 0) {
  Write-Host "No hex addresses found in trace.txt" -ForegroundColor Yellow
  exit 1
}

Write-Host "Using ELF: $elfPath"
Write-Host "Using addr2line: $addr2line"
Write-Host "Decoding $($addrs.Count) addresses..."

# Call addr2line. Pass addresses as separate arguments.
$procArgs = @('-pfia', '-e', $elfPath) + $addrs
try {
  $decoded = & $addr2line @procArgs 2>&1
}
catch {
  Write-Host "Failed to invoke addr2line: $_" -ForegroundColor Red
  exit 1
}

$outFile = Join-Path $root 'trace_decoded.txt'
$decoded | Set-Content -Path $outFile -Encoding UTF8

Write-Host "Decoded trace written to: $outFile`n"
Write-Host $decoded

