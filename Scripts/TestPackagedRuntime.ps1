# Copyright (c) 2026 AvaloNero. Licensed under the MIT License.

[CmdletBinding()]
param(
	[Parameter(Mandatory = $true)][string]$EngineRoot,
	[Parameter(Mandatory = $true)][string]$PluginPackage,
	[string]$WorkDirectory,
	[ValidateNotNullOrEmpty()][ValidateSet('Development', 'Shipping')][string[]]$Configurations = @('Development', 'Shipping'),
	[ValidateSet('Win64')][string]$TargetPlatform = 'Win64',
	[ValidateRange(10, 600)][int]$LaunchTimeoutSeconds = 120
)

$ErrorActionPreference = 'Stop'
$engine = (Resolve-Path -LiteralPath $EngineRoot).Path
$package = (Resolve-Path -LiteralPath $PluginPackage).Path
$descriptor = Get-Content -LiteralPath (Join-Path $package 'UnrealBMFont.uplugin') -Raw | ConvertFrom-Json
if ($descriptor.Installed -ne $true -or $descriptor.EnabledByDefault -ne $false) {
	throw 'PluginPackage must be an installed, explicitly opt-in BuildPlugin package.'
}
if ([string]::IsNullOrWhiteSpace($WorkDirectory)) {
	$WorkDirectory = Join-Path ([IO.Path]::GetTempPath()) ('UnrealBMFont\Runtime-' + [Guid]::NewGuid().ToString('N'))
}
$work = [IO.Path]::GetFullPath($WorkDirectory)
$source = [IO.Path]::GetFullPath((Split-Path -Parent $PSScriptRoot))
foreach ($parent in @($source, $package, $engine)) {
	$prefix = [IO.Path]::TrimEndingDirectorySeparator($parent)
	if ($work.Equals($prefix, [StringComparison]::OrdinalIgnoreCase) -or
		$work.StartsWith($prefix + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) {
		throw "WorkDirectory must be outside source, package and engine directories: $work"
	}
}
if (Test-Path -LiteralPath $work) { throw "WorkDirectory must not already exist: $work" }
$evidence = Join-Path $work 'Evidence'
New-Item -ItemType Directory -Path $evidence -Force | Out-Null
$project = Join-Path $work 'Example\BMFontExample.uproject'
& (Join-Path $PSScriptRoot 'CreateExampleProject.ps1') -EngineRoot $engine -PluginPackage $package -OutputDirectory (Join-Path $work 'Example') |
	Tee-Object -FilePath (Join-Path $evidence 'CreateExample.log') | Out-Host
$uat = Join-Path $engine 'Engine\Build\BatchFiles\RunUAT.bat'
$results = @()
foreach ($configuration in ($Configurations | Select-Object -Unique)) {
	$archive = Join-Path $work "Archive-$configuration"
	$uatLog = Join-Path $evidence "BuildCookRun-$configuration.log"
	& $uat BuildCookRun "-project=$project" -noP4 -utf8output -unattended -WaitForUATMutex -build -cook `
		'-AdditionalCookerOptions=-SkipZenStore' -stage -package -pak -iostore -archive `
		"-archivedirectory=$archive" "-platform=$TargetPlatform" "-clientconfig=$configuration" 2>&1 | Tee-Object -FilePath $uatLog
	if ($LASTEXITCODE -ne 0) { throw "BuildCookRun failed ($configuration). See $uatLog" }
	$executables = @(Get-ChildItem -LiteralPath $archive -Filter 'BMFontExample*.exe' -File -Recurse)
	$executable = $executables | Where-Object { $_.DirectoryName -match '\\Binaries\\Win64$' } | Select-Object -First 1
	if ($null -eq $executable) { throw "Packaged executable missing: $archive" }
	$caseEvidence = Join-Path $evidence $configuration
	New-Item -ItemType Directory -Path $caseEvidence | Out-Null
	$arguments = @('-BMFontValidate', '-Unattended', '-NoSound', '-NoSplash', '-windowed', '-ResX=900', '-ResY=600',
		'-RenderOffscreen', "-BMFontEvidenceDir=`"$caseEvidence`"", "-AbsLog=`"$(Join-Path $caseEvidence 'Runtime.log')`"")
	$process = Start-Process -FilePath $executable.FullName -ArgumentList $arguments -PassThru -WindowStyle Hidden
	if (-not $process.WaitForExit($LaunchTimeoutSeconds * 1000)) {
		$process.Kill($true)
		throw "Packaged GPU validation timed out ($configuration). Evidence: $caseEvidence"
	}
	$process.WaitForExit()
	if ($process.ExitCode -ne 0) { throw "Packaged process failed with exit code $($process.ExitCode) ($configuration)" }
	$resultFile = Join-Path $caseEvidence 'result.json'
	if (-not (Test-Path -LiteralPath $resultFile)) { throw "Packaged renderer produced no result: $caseEvidence" }
	$result = Get-Content -LiteralPath $resultFile -Raw | ConvertFrom-Json
	$names = @($result.cases | ForEach-Object name | Sort-Object)
	if ($result.passed -ne $true -or $result.assetsLoaded -ne $true -or $result.nullRHI -ne $false -or
		($names -join ',') -ne 'MultiPage,Packed,Plain,RichText' -or
		@($result.cases | Where-Object { $_.passed -ne $true -or $_.mismatchedPixels -ne 0 }).Count -ne 0) {
		throw "Packaged pixel validation failed ($configuration). See $resultFile and screenshots."
	}
	foreach ($case in $result.cases) {
		if (-not (Test-Path -LiteralPath (Join-Path $caseEvidence ($case.name + '.png')))) {
			throw "Missing screenshot evidence: $($case.name)"
		}
	}
	if (-not (Test-Path -LiteralPath (Join-Path $caseEvidence 'Showcase.png'))) {
		throw "Missing full showcase screenshot: $caseEvidence"
	}
	$results += [ordered]@{ Configuration = $configuration; Result = $resultFile; BuildCookRunLog = $uatLog }
	Write-Host "Packaged GPU rendering passed: $configuration"
}
$version = Get-Content -LiteralPath (Join-Path $engine 'Engine\Build\Build.version') -Raw | ConvertFrom-Json
$summary = [ordered]@{
	PluginVersion = $descriptor.VersionName
	EngineVersion = "$($version.MajorVersion).$($version.MinorVersion).$($version.PatchVersion)"
	TargetPlatform = $TargetPlatform
	GPUVerified = $true
	Configurations = $results
}
$summary | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $evidence 'summary.json') -Encoding utf8NoBOM
$global:LASTEXITCODE = 0
Write-Host "Packaged real-asset validation passed. Evidence: $evidence"
