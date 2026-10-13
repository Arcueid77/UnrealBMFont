# Copyright (c) 2026 AvaloNero. Licensed under the MIT License.

[CmdletBinding()]
param(
	[Parameter(Mandatory = $true)][ValidateNotNullOrEmpty()][string[]]$EngineRoots,
	[Parameter(Mandatory = $true)][string]$OutputDirectory,
	[switch]$CandidateOnly
)

$ErrorActionPreference = 'Stop'
$source = Split-Path -Parent $PSScriptRoot
$output = [IO.Path]::GetFullPath($OutputDirectory)
$sourcePrefix = [IO.Path]::TrimEndingDirectorySeparator([IO.Path]::GetFullPath($source))
if ($output.Equals($sourcePrefix, [StringComparison]::OrdinalIgnoreCase) -or
	$output.StartsWith($sourcePrefix + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) {
	throw 'Release validation output must be outside the repository.'
}
if (Test-Path -LiteralPath $output) { throw 'OutputDirectory must not exist; release evidence cannot be reused.' }

function Get-SourceFingerprint {
	$files = @(& git -C $source -c core.quotepath=false ls-files --cached --others --exclude-standard | Sort-Object -Unique)
	if ($LASTEXITCODE -ne 0 -or $files.Count -eq 0) { throw 'Cannot enumerate source inputs.' }
	$entries = foreach ($relative in $files) {
		$path = Join-Path $source $relative
		$hash = if (Test-Path -LiteralPath $path -PathType Leaf) { (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash } else { 'DELETED' }
		"${relative}:$hash"
	}
	[Convert]::ToHexString([Security.Cryptography.SHA256]::HashData([Text.Encoding]::UTF8.GetBytes(($entries -join "`n"))))
}

$head = & git -C $source rev-parse HEAD
if ($LASTEXITCODE -ne 0) { throw 'Cannot identify source revision.' }
$status = @(& git -C $source status --porcelain)
if ($LASTEXITCODE -ne 0) { throw 'Cannot inspect source status.' }
$dirty = $status.Count -gt 0
if ($dirty -and -not $CandidateOnly) { throw 'Final release validation requires a clean, committed source tree. Use -CandidateOnly for development.' }
$fingerprint = Get-SourceFingerprint
$engines = @(foreach ($root in $EngineRoots) {
	$resolved = (Resolve-Path -LiteralPath $root).Path
	$version = Get-Content -LiteralPath (Join-Path $resolved 'Engine\Build\Build.version') -Raw | ConvertFrom-Json
	if (-not (Test-Path -LiteralPath (Join-Path $resolved 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'))) {
		throw "No runnable Editor: $resolved"
	}
	[pscustomobject]@{
		Root = $resolved; Minor = "$($version.MajorVersion).$($version.MinorVersion)"; Version = $version
		InstalledBuild = Test-Path -LiteralPath (Join-Path $resolved 'Engine\Build\InstalledBuild.txt')
	}
})
if ($engines.Count -eq 0) { throw 'Release validation requires at least one complete Win64 engine build.' }
New-Item -ItemType Directory -Path $output | Out-Null
$descriptor = Get-Content -LiteralPath (Join-Path $source 'UnrealBMFont.uplugin') -Raw | ConvertFrom-Json
$summary = [ordered]@{
	SchemaVersion = 1; SourceCommit = [string]$head; SourceFingerprint = $fingerprint
	DirtySource = $dirty; PluginVersion = $descriptor.VersionName; CandidateOnly = [bool]$CandidateOnly
	AutomatedGatesPassed = $false; ReleaseReady = $false; Completed = $false; Engines = @()
}
try {
	& python (Join-Path $PSScriptRoot 'ValidateRepository.py')
	if ($LASTEXITCODE -ne 0) { throw 'Repository validation failed.' }
	$index = 0
	foreach ($engine in $engines) {
		$run = Join-Path $output ("UE-$($engine.Minor)-$index")
		$index++
		New-Item -ItemType Directory -Path $run | Out-Null
		$package = Join-Path $run 'Package'
		& (Join-Path $PSScriptRoot 'BuildPlugin.ps1') -EngineRoot $engine.Root -OutputDirectory $package
		& (Join-Path $PSScriptRoot 'TestPackagedRuntime.ps1') -EngineRoot $engine.Root -PluginPackage $package -WorkDirectory (Join-Path $run 'Runtime')
		$project = Join-Path $run 'Runtime\Example\BMFontExample.uproject'
		& (Join-Path $PSScriptRoot 'TestPlugin.ps1') -EngineRoot $engine.Root -Project $project -ReportDirectory (Join-Path $run 'Automation') -Strict
		& (Join-Path $PSScriptRoot 'TestPlugin.ps1') -EngineRoot $engine.Root -Project $project -ReportDirectory (Join-Path $run 'GPU') -DisableNullRHI -Strict
		$summary.Engines += [ordered]@{
			Version = $engine.Version; InstalledBuild = $engine.InstalledBuild
			Directory = [IO.Path]::GetRelativePath($output, $run); Passed = $true
		}
	}
	$finalHead = & git -C $source rev-parse HEAD
	if ($LASTEXITCODE -ne 0 -or [string]$finalHead -ne [string]$head) {
		throw 'Source commit changed during validation; these results cannot authorize a release.'
	}
	if ((Get-SourceFingerprint) -ne $fingerprint) { throw 'Source changed during validation; these results cannot authorize a release.' }
	$summary.Completed = $true
	$summary.AutomatedGatesPassed = $true
	$stableVersion = $descriptor.VersionName -match '^\d+\.\d+\.\d+$' -and $descriptor.IsBetaVersion -ne $true
	$summary.ReleaseReady = -not $CandidateOnly -and -not $dirty -and $stableVersion
} finally {
	$summary | ConvertTo-Json -Depth 10 | Set-Content -LiteralPath (Join-Path $output 'release-evidence.json') -Encoding utf8NoBOM
}
Write-Host "Validation complete. ReleaseReady=$($summary.ReleaseReady). Evidence: $output"
