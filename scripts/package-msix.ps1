<#
.SYNOPSIS
    Stage, pack and (optionally) sign an MSIX package for Bolt C++ ML.

.DESCRIPTION
    Takes the CMake install output (or a CPack ZIP artifact), lays out the
    MSIX package root (bin\, share\, Assets\, AppxManifest.xml), patches the
    manifest (adds the optional GUI Application entry when bin\gui_main.exe
    is present, optional version override), validates the layout, and then:

      * if the Windows SDK's makeappx.exe is available, packs a real .msix
        (and signs it with a self-signed development certificate when -Sign
        is given, creating the certificate on demand);
      * otherwise leaves the fully-staged, validated layout directory and
        prints the makeappx command to run on a machine with the SDK.

    Relative paths passed to this script are resolved against the repository
    root (the parent of the scripts\ directory).

.PARAMETER BuildDir
    CMake build directory to stage from via
    `cmake --install <BuildDir> --component app`. Default: build

.PARAMETER ZipPath
    Path to a CPack ZIP artifact (bolt-cppml-<ver>-<sys>.zip) to stage from
    instead of running `cmake --install`.

.PARAMETER OutputDir
    Directory for the staged layout and the produced .msix.
    Default: build\msix

.PARAMETER Config
    Build configuration passed to `cmake --install` (multi-config generators
    only; ignored by Ninja/Makefiles). Default: Release

.PARAMETER Version
    Optional manifest version override, must be four dotted numbers
    (e.g. 1.0.0.0).

.PARAMETER Sign
    Sign the .msix with a self-signed development certificate matching the
    manifest Publisher (created on demand in Cert:\CurrentUser\My, private
    key never leaves the store). Requires makeappx/signtool (Windows SDK).

.PARAMETER CertThumbprint
    Thumbprint of an existing certificate in Cert:\CurrentUser\My to sign
    with (implies -Sign). Its subject must match the manifest Publisher.

.PARAMETER Help
    Show this help text and exit.

.EXAMPLE
    .\scripts\package-msix.ps1
    Stage from the default build\ directory and pack (or lay out) the MSIX.

.EXAMPLE
    .\scripts\package-msix.ps1 -ZipPath build\bolt-cppml-1.0.0-win64.zip -Sign
    Stage from a CPack ZIP, pack and sign with a self-signed dev certificate.
#>
[CmdletBinding()]
param(
    [string]$BuildDir = 'build',
    [string]$ZipPath,
    [string]$OutputDir = 'build\msix',
    [string]$Config = 'Release',
    [string]$Version,
    [switch]$Sign,
    [string]$CertThumbprint,
    [switch]$Help
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$FoundationNs = 'http://schemas.microsoft.com/appx/manifest/foundation/windows10'
$UapNs = 'http://schemas.microsoft.com/appx/manifest/uap/windows10'

$RepoRoot = Split-Path -Parent $PSScriptRoot

function Resolve-RepoPath([string]$Path) {
    if ([System.IO.Path]::IsPathRooted($Path)) { return $Path }
    return (Join-Path $RepoRoot $Path)
}

function Write-Step([string]$Message) {
    Write-Host "==> $Message" -ForegroundColor Cyan
}

function Find-SdkTool([string]$Name) {
    $cmd = Get-Command $Name -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }
    $roots = @("${env:ProgramFiles(x86)}\Windows Kits\10\bin",
               "$env:ProgramFiles\Windows Kits\10\bin")
    foreach ($root in $roots) {
        if (-not (Test-Path $root)) { continue }
        $hit = Get-ChildItem -Path $root -Directory -ErrorAction SilentlyContinue |
            Where-Object { $_.Name -match '^10\.\d+' } |
            Sort-Object { [version]$_.Name } -Descending |
            ForEach-Object { Join-Path $_.FullName "x64\$Name" } |
            Where-Object { Test-Path $_ } |
            Select-Object -First 1
        if ($hit) { return $hit }
    }
    return $null
}

function New-ManifestNamespaceManager([xml]$Xml) {
    $nsm = New-Object System.Xml.XmlNamespaceManager($Xml.NameTable)
    $nsm.AddNamespace('m', $FoundationNs)
    $nsm.AddNamespace('uap', $UapNs)
    # comma prevents PowerShell from enumerating the IEnumerable manager
    return ,$nsm
}

function Update-Manifest {
    # Patches the layout manifest in place; returns the Identity Publisher.
    param(
        [string]$ManifestPath,
        [bool]$IncludeGui,
        [string]$NewVersion
    )
    $xml = New-Object System.Xml.XmlDocument
    $xml.Load($ManifestPath)
    $nsm = New-ManifestNamespaceManager $xml

    $identity = $xml.SelectSingleNode('/m:Package/m:Identity', $nsm)
    if (-not $identity) { throw "AppxManifest.xml has no <Identity> element." }
    if ($NewVersion) {
        $identity.SetAttribute('Version', $NewVersion)
        Write-Host "    Manifest version set to $NewVersion"
    }

    if ($IncludeGui) {
        $apps = $xml.SelectSingleNode('/m:Package/m:Applications', $nsm)
        $existing = $xml.SelectSingleNode(
            "/m:Package/m:Applications/m:Application[@Executable='bin\gui_main.exe']", $nsm)
        if (-not $existing) {
            # The GUI becomes the headline entry; relabel the console app to
            # match the NSIS shortcut names (Bolt C++ ML / Bolt Console).
            $consoleVisual = $xml.SelectSingleNode(
                "/m:Package/m:Applications/m:Application[@Executable='bin\bolt.exe']/uap:VisualElements", $nsm)
            if ($consoleVisual) {
                $consoleVisual.SetAttribute('DisplayName', 'Bolt Console')
            }

            $guiApp = $xml.CreateElement('Application', $FoundationNs)
            $guiApp.SetAttribute('Id', 'BoltGui')
            $guiApp.SetAttribute('Executable', 'bin\gui_main.exe')
            $guiApp.SetAttribute('EntryPoint', 'Windows.FullTrustApplication')

            $visual = $xml.CreateElement('uap', 'VisualElements', $UapNs)
            $visual.SetAttribute('DisplayName', 'Bolt C++ ML')
            $visual.SetAttribute('Description', 'Bolt C++ ML desktop GUI')
            $visual.SetAttribute('BackgroundColor', 'transparent')
            $visual.SetAttribute('Square150x150Logo', 'Assets\Square150x150Logo.png')
            $visual.SetAttribute('Square44x44Logo', 'Assets\Square44x44Logo.png')

            $tile = $xml.CreateElement('uap', 'DefaultTile', $UapNs)
            $tile.SetAttribute('Wide310x150Logo', 'Assets\Wide310x150Logo.png')
            [void]$visual.AppendChild($tile)
            [void]$guiApp.AppendChild($visual)
            [void]$apps.PrependChild($guiApp)
            Write-Host '    Added Application entry for bin\gui_main.exe'
        }
    }

    $settings = New-Object System.Xml.XmlWriterSettings
    $settings.Indent = $true
    $settings.Encoding = New-Object System.Text.UTF8Encoding($false)
    $writer = [System.Xml.XmlWriter]::Create($ManifestPath, $settings)
    try { $xml.Save($writer) } finally { $writer.Dispose() }

    return $identity.GetAttribute('Publisher')
}

function Test-MsixLayout {
    # Validates the staged layout; throws on the first problem.
    param([string]$LayoutDir)

    $manifestPath = Join-Path $LayoutDir 'AppxManifest.xml'
    if (-not (Test-Path $manifestPath)) { throw "Layout is missing AppxManifest.xml." }

    $xml = New-Object System.Xml.XmlDocument
    $xml.Load($manifestPath)   # throws if not well-formed
    if ($xml.DocumentElement.LocalName -ne 'Package' -or
        $xml.DocumentElement.NamespaceURI -ne $FoundationNs) {
        throw "AppxManifest.xml root element must be <Package> in namespace $FoundationNs."
    }
    $nsm = New-ManifestNamespaceManager $xml

    $identity = $xml.SelectSingleNode('/m:Package/m:Identity', $nsm)
    foreach ($attr in 'Name', 'Publisher', 'Version') {
        if (-not $identity.GetAttribute($attr)) { throw "Identity is missing the $attr attribute." }
    }
    Write-Host ("    Identity: {0} {1} ({2})" -f $identity.GetAttribute('Name'),
        $identity.GetAttribute('Version'), $identity.GetAttribute('Publisher'))

    $applications = $xml.SelectNodes('/m:Package/m:Applications/m:Application', $nsm)
    if ($applications.Count -lt 1) { throw "Manifest declares no <Application> entries." }
    foreach ($app in $applications) {
        $exe = $app.GetAttribute('Executable')
        $exePath = Join-Path $LayoutDir $exe
        if (-not (Test-Path $exePath)) {
            throw "Application '$($app.GetAttribute('Id'))' references '$exe' which is not in the layout."
        }
        Write-Host "    Application '$($app.GetAttribute('Id'))' -> $exe (present)"
    }

    $images = New-Object System.Collections.Generic.List[string]
    $logo = $xml.SelectSingleNode('/m:Package/m:Properties/m:Logo', $nsm)
    if ($logo) { $images.Add($logo.InnerText.Trim()) }
    foreach ($visual in $xml.SelectNodes('//uap:VisualElements', $nsm)) {
        foreach ($attr in 'Square150x150Logo', 'Square44x44Logo') {
            $v = $visual.GetAttribute($attr)
            if ($v) { $images.Add($v) }
        }
    }
    foreach ($tile in $xml.SelectNodes('//uap:DefaultTile', $nsm)) {
        $v = $tile.GetAttribute('Wide310x150Logo')
        if ($v) { $images.Add($v) }
    }
    foreach ($img in $images | Sort-Object -Unique) {
        if (-not (Test-Path (Join-Path $LayoutDir $img))) {
            throw "Manifest references image '$img' which is not in the layout."
        }
    }
    Write-Host "    All $((($images | Sort-Object -Unique)).Count) referenced images present"
}

function Get-DevCert([string]$Subject, [string]$Thumbprint) {
    if ($Thumbprint) {
        $cert = Get-Item "Cert:\CurrentUser\My\$Thumbprint" -ErrorAction SilentlyContinue
        if (-not $cert) { throw "Certificate $Thumbprint not found in Cert:\CurrentUser\My." }
        if ($cert.Subject -ne $Subject) {
            throw "Certificate subject '$($cert.Subject)' does not match the manifest Publisher '$Subject'; MSIX signing requires an exact match."
        }
        return $cert
    }
    $existing = Get-ChildItem Cert:\CurrentUser\My |
        Where-Object { $_.Subject -eq $Subject -and $_.HasPrivateKey -and $_.NotAfter -gt (Get-Date) } |
        Sort-Object NotAfter -Descending | Select-Object -First 1
    if ($existing) {
        Write-Host "    Reusing certificate $($existing.Thumbprint) ($Subject)"
        return $existing
    }
    Write-Host "    Creating self-signed development certificate '$Subject' in Cert:\CurrentUser\My"
    return New-SelfSignedCertificate -Type Custom -Subject $Subject `
        -KeyUsage DigitalSignature `
        -FriendlyName 'Bolt C++ ML MSIX development certificate' `
        -CertStoreLocation 'Cert:\CurrentUser\My' `
        -TextExtension @('2.5.29.37={text}1.3.6.1.5.5.7.3.3', '2.5.29.19={text}')
}

if ($Help) {
    Get-Help $MyInvocation.MyCommand.Path -Detailed
    return
}

# ---------------------------------------------------------------------------
# Parameter validation
# ---------------------------------------------------------------------------
if ($Version -and $Version -notmatch '^\d+\.\d+\.\d+\.\d+$') {
    throw "-Version must be four dotted numbers (e.g. 1.0.0.0), got '$Version'."
}
if ($CertThumbprint) { $Sign = $true }

$manifestSrc = Join-Path $RepoRoot 'packaging\msix\AppxManifest.xml'
$assetsSrc = Join-Path $RepoRoot 'packaging\msix\Assets'
if (-not (Test-Path $manifestSrc)) { throw "Missing $manifestSrc." }
if (-not (Test-Path $assetsSrc)) { throw "Missing $assetsSrc (run scripts/generate_msix_assets.py)." }

if ($ZipPath) {
    $ZipPath = Resolve-RepoPath $ZipPath
    if (-not (Test-Path $ZipPath)) { throw "ZIP not found: $ZipPath" }
    if ([System.IO.Path]::GetExtension($ZipPath) -ne '.zip') { throw "-ZipPath must point to a .zip file." }
} else {
    $BuildDir = Resolve-RepoPath $BuildDir
    if (-not (Test-Path (Join-Path $BuildDir 'CMakeCache.txt'))) {
        throw "'$BuildDir' is not a configured CMake build directory (no CMakeCache.txt). Build first or pass -ZipPath."
    }
    if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
        throw "cmake was not found on PATH (required to stage from -BuildDir)."
    }
}

$OutputDir = Resolve-RepoPath $OutputDir
$layoutDir = Join-Path $OutputDir 'layout'

# ---------------------------------------------------------------------------
# Stage the application files into the MSIX layout root
# ---------------------------------------------------------------------------
Write-Step 'Staging application files'
if (Test-Path $layoutDir) { Remove-Item -Recurse -Force $layoutDir }
New-Item -ItemType Directory -Force -Path $layoutDir | Out-Null

if ($ZipPath) {
    $unzipDir = Join-Path $OutputDir 'unzip'
    if (Test-Path $unzipDir) { Remove-Item -Recurse -Force $unzipDir }
    Expand-Archive -Path $ZipPath -DestinationPath $unzipDir -Force
    $binExe = Get-ChildItem -Path $unzipDir -Recurse -Filter 'bolt.exe' |
        Where-Object { $_.Directory.Name -eq 'bin' } | Select-Object -First 1
    if (-not $binExe) { throw "Could not find bin\bolt.exe inside $ZipPath." }
    $appRoot = $binExe.Directory.Parent.FullName
    Copy-Item -Path (Join-Path $appRoot '*') -Destination $layoutDir -Recurse
    Write-Host "    Staged from ZIP ($appRoot)"
} else {
    & cmake --install $BuildDir --prefix $layoutDir --component app --config $Config
    if ($LASTEXITCODE -ne 0) { throw "cmake --install failed with exit code $LASTEXITCODE." }
}

$boltExe = Join-Path $layoutDir 'bin\bolt.exe'
if (-not (Test-Path $boltExe)) { throw "Staging produced no bin\bolt.exe under $layoutDir." }
$hasGui = Test-Path (Join-Path $layoutDir 'bin\gui_main.exe')
Write-Host "    bin\bolt.exe present; gui_main.exe $(if ($hasGui) { 'present' } else { 'absent (console-only package)' })"

# ---------------------------------------------------------------------------
# Add manifest + assets, patch the manifest
# ---------------------------------------------------------------------------
Write-Step 'Writing AppxManifest.xml and Assets'
Copy-Item $manifestSrc (Join-Path $layoutDir 'AppxManifest.xml')
Copy-Item $assetsSrc (Join-Path $layoutDir 'Assets') -Recurse
$publisher = Update-Manifest -ManifestPath (Join-Path $layoutDir 'AppxManifest.xml') `
    -IncludeGui $hasGui -NewVersion $Version

Write-Step 'Validating MSIX layout'
Test-MsixLayout -LayoutDir $layoutDir

# ---------------------------------------------------------------------------
# Pack (and sign) with the Windows SDK, or stop at the validated layout
# ---------------------------------------------------------------------------
$manifestXml = [xml](Get-Content (Join-Path $layoutDir 'AppxManifest.xml') -Raw)
$pkgVersion = $manifestXml.Package.Identity.Version
$msixPath = Join-Path $OutputDir "bolt-cppml-$pkgVersion-x64.msix"

$makeappx = Find-SdkTool 'makeappx.exe'
if (-not $makeappx) {
    Write-Step 'makeappx.exe not found - stopping at the staged layout'
    Write-Host ''
    Write-Host 'The Windows 10/11 SDK is not installed, so no .msix was produced.' -ForegroundColor Yellow
    Write-Host 'A fully staged, validated MSIX layout is ready at:'
    Write-Host "    $layoutDir"
    Write-Host ''
    Write-Host 'On a machine with the Windows SDK, produce the package with:'
    Write-Host "    makeappx pack /o /d `"$layoutDir`" /p `"$msixPath`""
    if ($Sign) {
        Write-Host ''
        Write-Warning 'Signing was requested (-Sign) but also requires the Windows SDK (signtool.exe).'
    }
    return
}

Write-Step "Packing with $makeappx"
& $makeappx pack /o /d $layoutDir /p $msixPath
if ($LASTEXITCODE -ne 0) { throw "makeappx pack failed with exit code $LASTEXITCODE." }
Write-Host "    Created $msixPath"

if ($Sign) {
    Write-Step 'Signing'
    $signtool = Find-SdkTool 'signtool.exe'
    if (-not $signtool) { throw 'signtool.exe not found (install the Windows SDK).' }
    $cert = Get-DevCert -Subject $publisher -Thumbprint $CertThumbprint
    & $signtool sign /fd SHA256 /sha1 $cert.Thumbprint $msixPath
    if ($LASTEXITCODE -ne 0) { throw "signtool sign failed with exit code $LASTEXITCODE." }
    Write-Host "    Signed with certificate $($cert.Thumbprint)"
    Write-Host ''
    Write-Host 'To install on this or another machine, first trust the certificate' -ForegroundColor Yellow
    Write-Host '(export the .cer WITHOUT the private key and import it into'
    Write-Host 'LocalMachine\TrustedPeople - see docs/WINDOWS_PACKAGING.md), then run:'
    Write-Host "    Add-AppxPackage `"$msixPath`""
} else {
    Write-Host ''
    Write-Host 'Package is unsigned; sign it with -Sign (self-signed dev cert) before installing,' -ForegroundColor Yellow
    Write-Host 'or install unsigned in Developer Mode with: Add-AppxPackage -Register on the layout.'
}
