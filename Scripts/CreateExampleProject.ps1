# Copyright (c) 2026 AvaloNero. Licensed under the MIT License.

[CmdletBinding()]
param(
	[Parameter(Mandatory = $true)][string]$EngineRoot,
	[Parameter(Mandatory = $true)][string]$PluginPackage,
	[Parameter(Mandatory = $true)][string]$OutputDirectory
)

$ErrorActionPreference = 'Stop'
$pluginRoot = Split-Path -Parent $PSScriptRoot
$engine = (Resolve-Path -LiteralPath $EngineRoot).Path
$package = (Resolve-Path -LiteralPath $PluginPackage).Path
$destination = [IO.Path]::GetFullPath($OutputDirectory)
foreach ($parent in @($pluginRoot, $package, $engine)) {
	$prefix = [IO.Path]::TrimEndingDirectorySeparator([IO.Path]::GetFullPath($parent))
	if ($destination.Equals($prefix, [StringComparison]::OrdinalIgnoreCase) -or
		$destination.StartsWith($prefix + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) {
		throw "Example output must be outside source, package and engine directories: $destination"
	}
}
if (Test-Path -LiteralPath $destination) { throw "OutputDirectory must not already exist: $destination" }
$editor = Join-Path $engine 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$build = Join-Path $engine 'Engine\Build\BatchFiles\Build.bat'
if (-not (Test-Path -LiteralPath $editor) -or -not (Test-Path -LiteralPath $build)) {
	throw 'EngineRoot must contain a runnable Win64 Editor and build tools.'
}
if (-not (Test-Path -LiteralPath (Join-Path $package 'Samples\MultiPage\MultiPage.fnt'))) {
	throw 'The plugin package must include the multi-page fixture.'
}
New-Item -ItemType Directory -Path $destination | Out-Null
New-Item -ItemType Directory -Path (Join-Path $destination 'Content') | Out-Null
Get-ChildItem -LiteralPath (Join-Path $pluginRoot 'Examples\Showcase') -Force | ForEach-Object {
	Copy-Item -LiteralPath $_.FullName -Destination $destination -Recurse
}
$pluginsDirectory = Join-Path $destination 'Plugins'
New-Item -ItemType Directory -Path $pluginsDirectory | Out-Null
Copy-Item -LiteralPath $package -Destination (Join-Path $pluginsDirectory 'UnrealBMFont') -Recurse
$project = Join-Path $destination 'BMFontExample.uproject'
& $build BMFontExampleEditor Win64 Development "-Project=$project" -WaitMutex -NoHotReloadFromIDE
if ($LASTEXITCODE -ne 0) { throw "Example editor build failed: $LASTEXITCODE" }
$prepareLog = Join-Path $destination 'PrepareExampleAssets.log'
$prepareScript = Join-Path $PSScriptRoot 'PrepareExampleAssets.py'
& $editor $project "-ExecutePythonScript=$prepareScript" -Unattended -NoSplash -NoSound -NullRHI -stdout -FullStdOutLogOutput "-AbsLog=$prepareLog"
if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath $prepareLog) -or
	-not (Select-String -LiteralPath $prepareLog -SimpleMatch 'BMFONT_EXAMPLE_ASSETS=PASS' -Quiet) -or
	-not (Test-Path -LiteralPath (Join-Path $destination 'Content\BMFont\ShowcaseMap.umap'))) {
	throw "Example asset preparation failed. See $prepareLog"
}
$global:LASTEXITCODE = 0
Write-Host "Example ready: $project"
Write-Output $project
