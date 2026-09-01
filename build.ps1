# EchoTrace Windows build script
# Usage: .\build.ps1 [target]
# Targets: cli (default), lib, tests, bench, clean

param(
    [string]$Target = "cli"
)

$ErrorActionPreference = "Stop"

# Find g++ - check PATH first, then common install locations
$gpp = Get-Command g++ -ErrorAction SilentlyContinue
if (-not $gpp) {
    $searchPaths = @(
        "$env:LOCALAPPDATA\Programs\mingw64\bin\g++.exe",
        "C:\mingw64\bin\g++.exe",
        "C:\msys64\mingw64\bin\g++.exe",
        "C:\Program Files\mingw64\bin\g++.exe"
    )
    foreach ($p in $searchPaths) {
        if (Test-Path $p) {
            $gpp = Get-Item $p
            break
        }
    }
}

if (-not $gpp) {
    Write-Error "g++ not found. Install WinLibs/MinGW and add to PATH."
    exit 1
}

$GXX = if ($gpp -is [System.Management.Automation.ApplicationInfo]) { $gpp.Source } else { $gpp.FullName }
$AR = ($GXX -replace 'g\+\+', 'llvm-ar')
if (-not (Test-Path $AR)) { $AR = "llvm-ar" }

Write-Host "Using: $GXX" -ForegroundColor Cyan
& $GXX --version | Select-Object -First 1

$CXXSTD = "-std=c++20"
$WARN = "-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Wnon-virtual-dtor -Wold-style-cast -Wcast-align"
$OPT = "-O2"
$INC = "-Iinclude"

$BUILD = "build"
$BINDIR = "$BUILD\bin"
$OBJDIR = "$BUILD\obj"
$TESTDIR = "$BUILD\tests"

function Ensure-Dir($path) {
    if (-not (Test-Path $path)) {
        New-Item -ItemType Directory -Path $path -Force | Out-Null
    }
}

function Compile-File($src, $obj) {
    $dir = Split-Path $obj -Parent
    Ensure-Dir $dir
    $cmd = "$GXX $CXXSTD $WARN $OPT $INC -MMD -MP -c `"$src`" -o `"$obj`""
    Write-Host "[CXX] $src" -ForegroundColor Green
    Invoke-Expression $cmd
    if ($LASTEXITCODE -ne 0) { throw "Compilation failed: $src" }
}

function Build-Lib {
    Ensure-Dir $BUILD
    Ensure-Dir $OBJDIR

    $libSrcs = Get-ChildItem "src" -Filter "*.cpp" -Recurse -File | Where-Object { $_.FullName -notmatch "\\cli\\" }
    $objs = @()
    foreach ($src in $libSrcs) {
        # Keep relative path under src/ to avoid name collisions
        $rel = (Resolve-Path -Relative $src.FullName).TrimStart('.\').Substring(4)
        $obj = "$OBJDIR\$($rel -replace '\.cpp$','.o')"
        $objs += $obj
        Compile-File $src.FullName $obj
    }

    $lib = "$BUILD\libechotrace.a"
    Write-Host "[AR] $lib" -ForegroundColor Yellow
    & $AR rcs $lib @objs
    if ($LASTEXITCODE -ne 0) { throw "Archive failed" }
    return $lib
}

function Build-Cli {
    $lib = Build-Lib
    Ensure-Dir $BINDIR
    Ensure-Dir "$OBJDIR\cli"

    $cliSrc = "src\cli\main.cpp"
    $cliObj = "$OBJDIR\cli\main.o"
    Compile-File $cliSrc $cliObj

    $cliBin = "$BINDIR\echotrace.exe"
    Write-Host "[LINK] $cliBin" -ForegroundColor Yellow
    & $GXX $CXXSTD $OPT $cliObj -L"$BUILD" -lechotrace -static -o $cliBin
    if ($LASTEXITCODE -ne 0) { throw "Link failed" }
    Write-Host "Built: $cliBin" -ForegroundColor Cyan
    return $cliBin
}

function Build-Tests {
    $lib = Build-Lib
    Ensure-Dir $BINDIR
    Ensure-Dir $TESTDIR

    $testSrcs = Get-ChildItem "tests\*.cpp" -File
    $objs = @()
    foreach ($src in $testSrcs) {
        $obj = "$TESTDIR\$($src.BaseName).o"
        $objs += $obj
        $dir = Split-Path $obj -Parent
        Ensure-Dir $dir
        $cmd = "$GXX $CXXSTD $WARN $OPT $INC -Itests -MMD -MP -c `"$($src.FullName)`" -o `"$obj`""
        Write-Host "[CXX] $($src.Name)" -ForegroundColor Green
        Invoke-Expression $cmd
        if ($LASTEXITCODE -ne 0) { throw "Test compilation failed: $($src.Name)" }
    }

    $testBin = "$BINDIR\echotrace_tests.exe"
    $objStr = $objs -join " "
    Write-Host "[LINK] $testBin" -ForegroundColor Yellow
    & $GXX $CXXSTD $OPT @objs -L"$BUILD" -lechotrace -o $testBin
    if ($LASTEXITCODE -ne 0) { throw "Test link failed" }

    Write-Host "`n[RUN] Unit Tests" -ForegroundColor Cyan
    & ".\$testBin"
    if ($LASTEXITCODE -ne 0) { throw "Tests failed!" }
}

function Clean {
    if (Test-Path $BUILD) {
        Remove-Item -Recurse -Force $BUILD
        Write-Host "Cleaned build/" -ForegroundColor Yellow
    }
}

switch ($Target) {
    "cli"   { Build-Cli }
    "lib"   { Build-Lib }
    "tests" { Build-Tests }
    "clean" { Clean }
    default { Write-Error "Unknown target: $Target" }
}
