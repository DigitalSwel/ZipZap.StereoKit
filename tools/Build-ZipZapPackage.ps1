param(
    [string]$NdkPath = $env:NDK,
    [string]$JavaHome = $env:JAVA_HOME,
    [string]$CMakePath = '',
    [string]$NinjaPath = '',
    [ValidateRange(1, 999999)][int]$Revision = 1,
    [ValidateRange(1, 64)][int]$Jobs = 8,
    [string]$LogDirectory = '.tmp/stereokit-package',
    [switch]$PackOnly
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$packageId = 'ZipZap.StereoKit'
$feed = Join-Path $repoRoot 'bin/nuget-feed'
$packageWork = Join-Path $repoRoot 'bin/zipzap-package'
$logs = [IO.Path]::GetFullPath((Join-Path $repoRoot $LogDirectory))
$scratchRoot = [IO.Path]::GetFullPath((Join-Path $repoRoot '.tmp')) + [IO.Path]::DirectorySeparatorChar
if (-not $logs.StartsWith($scratchRoot, [StringComparison]::OrdinalIgnoreCase)) {
    throw 'LogDirectory must stay inside repository-root .tmp/.'
}

function Invoke-BuildCommand {
    param([string]$Executable, [string[]]$Arguments, [string]$LogName)

    $quotedArguments = @($Arguments | ForEach-Object { '"' + ($_ -replace '"', '\"') + '"' })
    $stdout = Join-Path $logs "$LogName.out.log"
    $stderr = Join-Path $logs "$LogName.err.log"
    Write-Host "$LogName..."
    $process = Start-Process -FilePath $Executable -ArgumentList $quotedArguments -WorkingDirectory $repoRoot `
        -NoNewWindow -PassThru -RedirectStandardOutput $stdout -RedirectStandardError $stderr
    try {
        # Start-Process -Wait also waits for persistent compiler-server descendants.
        $null = $process.Handle
        $process.WaitForExit()
        if ($process.ExitCode -ne 0) {
            throw "$LogName failed (exit $($process.ExitCode)). See $stdout and $stderr."
        }
    }
    finally { $process.Dispose() }
}

# Logs are scratch; the distributable package/feed use the existing ignored bin layout.
if (-not ((Get-Content (Join-Path $repoRoot '.gitignore')) -contains '/.tmp/')) {
    throw 'Add /.tmp/ to repository-root .gitignore before creating build logs.'
}
New-Item -ItemType Directory -Force -Path $logs, $feed, $packageWork | Out-Null

$git = (Get-Command git -ErrorAction Stop).Source
$dotnet = (Get-Command dotnet -ErrorAction Stop).Source
$commit = (& $git -C $repoRoot rev-parse HEAD).Trim()
if ($LASTEXITCODE -ne 0 -or $commit -notmatch '^[a-f0-9]{40}$') {
    throw 'Cannot identify source commit.'
}
# Packaging edits may be uncommitted; runtime changes must have committed provenance.
& $git -C $repoRoot diff --quiet HEAD -- StereoKitC 'StereoKit/*.cs' 'StereoKit/**/*.cs' CMakeLists.txt cmake CMakePresets.json
if ($LASTEXITCODE -ne 0) {
    throw 'Runtime/native build source differs from HEAD. Commit it before creating a release package.'
}
$untrackedRuntimeSource = @(& $git -C $repoRoot ls-files --others --exclude-standard -- StereoKitC 'StereoKit/*.cs' 'StereoKit/**/*.cs' CMakeLists.txt cmake CMakePresets.json)
if ($LASTEXITCODE -ne 0 -or $untrackedRuntimeSource.Count -ne 0) {
    throw 'Runtime/native build source contains untracked files. Commit them before creating a release package.'
}
$nativeVersion = & (Join-Path $PSScriptRoot 'Get-Version.ps1')
$version = "$($nativeVersion.major).$($nativeVersion.minor).$($nativeVersion.patch)-zipzap.$Revision.g$($commit.Substring(0, 12))"
$finalPackage = Join-Path $feed "$packageId.$version.nupkg"
if (Test-Path $finalPackage) {
    throw "Package version already exists: $finalPackage. Use a new -Revision; do not overwrite released packages."
}

$environmentNames = @('NDK', 'JAVA_HOME', 'PATH', 'CPM_SOURCE_CACHE')
$previousEnvironment = @{}
foreach ($name in $environmentNames) {
    $previousEnvironment[$name] = [Environment]::GetEnvironmentVariable($name, 'Process')
}

try {
    if (-not $PackOnly) {
        $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
        $vsPath = ''
        if (Test-Path $vswhere) {
            $vsPath = (& $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath).Trim()
        }
        if ($CMakePath -eq '' -and $vsPath -ne '') {
            $CMakePath = Join-Path $vsPath 'Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe'
        }
        if ($CMakePath -eq '') { $CMakePath = (Get-Command cmake -ErrorAction Stop).Source }
        if ($NinjaPath -eq '' -and $vsPath -ne '') {
            $NinjaPath = Join-Path $vsPath 'Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe'
        }
        if ($NinjaPath -eq '') { $NinjaPath = (Get-Command ninja -ErrorAction Stop).Source }
        if (-not (Test-Path $CMakePath) -or -not (Test-Path $NinjaPath)) {
            throw 'CMake and Ninja must be installed; no toolchain is installed or upgraded by this script.'
        }
        if ($NdkPath -eq '') {
            $sdk = $env:ANDROID_SDK_ROOT
            if ($sdk -eq '') { $sdk = $env:ANDROID_HOME }
            if ($sdk -eq '' -or -not (Test-Path (Join-Path $sdk 'ndk'))) { throw 'Set -NdkPath or NDK.' }
            $NdkPath = (Get-ChildItem (Join-Path $sdk 'ndk') -Directory | Sort-Object { [version]$_.Name } -Descending | Select-Object -First 1).FullName
        }
        if (-not (Test-Path (Join-Path $NdkPath 'build/cmake/android.toolchain.cmake'))) {
            throw 'NdkPath is not a valid installed Android NDK.'
        }
        if ($JavaHome -eq '' -or -not (Test-Path (Join-Path $JavaHome 'bin/javac.exe'))) {
            throw 'Set -JavaHome to an installed JDK containing javac and jar.'
        }
        $env:NDK = $NdkPath.Replace('\', '/')
        $env:JAVA_HOME = $JavaHome
        $env:PATH = "$(Split-Path $CMakePath);$(Split-Path $NinjaPath);$(Join-Path $JavaHome 'bin');$env:PATH"
        $env:CPM_SOURCE_CACHE = (Join-Path $repoRoot '.deps_cache').Replace('\', '/')

        Invoke-BuildCommand $CMakePath @('--preset', 'Win32_x64_Release', '-DSK_BUILD_TESTS=OFF') 'configure-win'
        Invoke-BuildCommand $CMakePath @('--build', '--preset', 'Win32_x64_Release', '--target', 'StereoKitC', '--parallel', "$Jobs") 'build-win'
        $hostShaderCompiler = (Join-Path $repoRoot 'bin/tools/win32_x64/skshaderc.exe').Replace('\', '/')
        Invoke-BuildCommand $CMakePath @('--preset', 'Android_Arm64_Release', "-DCMAKE_MAKE_PROGRAM=$($NinjaPath.Replace('\', '/'))",
            "-DSKSHADERC_HOST_EXE=$hostShaderCompiler", '-DSKSHADERC_BUILD_HOST_TOOL=OFF', '-DSK_BUILD_TESTS=OFF') 'configure-android'
        # Upstream's distribute copy expects the Java target to have been built explicitly.
        Invoke-BuildCommand $CMakePath @('--build', '--preset', 'Android_Arm64_Release', '--target', 'sk_app_jar', '--parallel', "$Jobs") 'build-android-java'
        Invoke-BuildCommand $CMakePath @('--build', '--preset', 'Android_Arm64_Release', '--target', 'StereoKitC', '--parallel', "$Jobs") 'build-android'
    }

    $requiredFiles = [ordered]@{
        'runtimes/win-x64/native/StereoKitC.dll' = 'bin/distribute/bin/Win32/x64/Release/StereoKitC.dll'
        'runtimes/android-arm64/native/libStereoKitC.so' = 'bin/distribute/bin/Android/arm64-v8a/Release/libStereoKitC.so'
        'android_openxr_loaders/standard/arm64/libopenxr_loader.so' = 'bin/distribute/bin/Android/arm64-v8a/Release/standard/libopenxr_loader.so'
        'build/sk_app.jar' = 'bin/distribute/bin/Android/sk_app.jar'
        'tools/skshaderc/win32_x64/skshaderc.exe' = 'bin/tools/win32_x64/skshaderc.exe'
    }
    $binaryHashes = [ordered]@{}
    foreach ($entry in $requiredFiles.GetEnumerator()) {
        $file = Join-Path $repoRoot $entry.Value
        if (-not (Test-Path $file)) { throw "Required build output missing: $file" }
        $binaryHashes[$entry.Key] = (Get-FileHash $file -Algorithm SHA256).Hash.ToLowerInvariant()
    }
    $manifest = [ordered]@{
        packageId = $packageId
        packageVersion = $version
        sourceRepository = 'https://github.com/DigitalSwel/ZipZap.StereoKit'
        sourceCommit = $commit
        nativeVersion = $nativeVersion.str
        runtimeIdentifiers = @('win-x64', 'android-arm64')
        shaderHost = 'win-x64'
        nativeTestsIncluded = $false
        binarySha256 = $binaryHashes
    }
    [IO.File]::WriteAllText((Join-Path $packageWork 'provenance.json'), ($manifest | ConvertTo-Json -Depth 8), [Text.UTF8Encoding]::new($false))

    $staging = Join-Path $packageWork 'staging'
    New-Item -ItemType Directory -Force -Path $staging | Out-Null
    Invoke-BuildCommand $dotnet @('pack', 'StereoKit/StereoKit.csproj', '-c', 'Release', '-o', $staging,
        "-p:PackageId=$packageId", "-p:Version=$version", "-p:PackageVersion=$version", "-p:Title=$packageId",
        '-p:PackageProjectUrl=https://github.com/DigitalSwel/ZipZap.StereoKit',
        '-p:RepositoryUrl=https://github.com/DigitalSwel/ZipZap.StereoKit', "-p:RepositoryCommit=$commit",
        '-p:ContinuousIntegrationBuild=true') 'pack'

    Add-Type -AssemblyName System.IO.Compression.FileSystem
    $stagedPackage = Join-Path $staging "$packageId.$version.nupkg"
    $archive = [IO.Compression.ZipFile]::OpenRead($stagedPackage)
    try {
        $names = @($archive.Entries | ForEach-Object FullName)
        $expectedFiles = @($requiredFiles.Keys) + @('lib/net8.0/StereoKit.dll', 'lib/net8.0/StereoKit.pdb',
            "build/$packageId.props", "build/$packageId.targets", 'build/SKAssets.targets', 'build/SKShaders.targets',
            'tools/include/stereokit.hlsli', 'LICENSE', 'zipzap/provenance.json')
        foreach ($name in $expectedFiles) {
            if ($names -notcontains $name) { throw "Package missing $name." }
        }
        foreach ($name in $names) {
            if ($name.StartsWith('runtimes/') -and $name -notmatch '^runtimes/(win-x64|android-arm64)/') {
                throw "Unexpected platform payload: $name. Do not ship stale build outputs."
            }
        }
        foreach ($entry in $requiredFiles.GetEnumerator()) {
            $stream = $archive.GetEntry($entry.Key).Open()
            $sha = [Security.Cryptography.SHA256]::Create()
            try { $hash = ([BitConverter]::ToString($sha.ComputeHash($stream))).Replace('-', '').ToLowerInvariant() }
            finally { $stream.Dispose(); $sha.Dispose() }
            if ($hash -ne $binaryHashes[$entry.Key]) { throw "Package payload hash mismatch: $($entry.Key)" }
        }
        $reader = [IO.StreamReader]::new($archive.GetEntry("$packageId.nuspec").Open())
        try { [xml]$nuspec = $reader.ReadToEnd() } finally { $reader.Dispose() }
        if ($nuspec.package.metadata.id -ne $packageId -or $nuspec.package.metadata.version -ne $version -or
            $nuspec.package.metadata.repository.commit -ne $commit) { throw 'Package identity/provenance mismatch.' }
    }
    finally { $archive.Dispose() }

    Copy-Item $stagedPackage $finalPackage
    Write-Host "Validated package: $finalPackage"
    Write-Host "SHA256: $((Get-FileHash $finalPackage -Algorithm SHA256).Hash.ToLowerInvariant())"
    Write-Host 'No registry upload performed. Consumer and headset validation are separate steps.'
}
finally {
    foreach ($name in $environmentNames) {
        [Environment]::SetEnvironmentVariable($name, $previousEnvironment[$name], 'Process')
    }
}
