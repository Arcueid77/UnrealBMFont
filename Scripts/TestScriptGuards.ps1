# Copyright (c) 2026 AvaloNero. Licensed under the MIT License.

[CmdletBinding()]
param(
	[Parameter(Mandatory = $true)][string]$EngineRoot,
	[Parameter(Mandatory = $true)][string]$PluginPackage
)

$ErrorActionPreference = 'Stop'
$source = Split-Path -Parent $PSScriptRoot
function Assert-Rejected {
	param([scriptblock]$Action, [string]$Expected)
	try { & $Action; throw 'Guard unexpectedly accepted the request.' }
	catch { if ($_.Exception.Message -notlike "*$Expected*") { throw } }
}

Assert-Rejected { & "$PSScriptRoot/CreateExampleProject.ps1" -EngineRoot $EngineRoot -PluginPackage $PluginPackage -OutputDirectory $source } 'outside source'
Assert-Rejected { & "$PSScriptRoot/TestPackagedRuntime.ps1" -EngineRoot $EngineRoot -PluginPackage $PluginPackage -WorkDirectory $source } 'outside source'
Assert-Rejected { & "$PSScriptRoot/TestRelease.ps1" -EngineRoots @($EngineRoot) -OutputDirectory $source } 'outside the repository'
Assert-Rejected { & "$PSScriptRoot/TestRelease.ps1" -EngineRoots @($EngineRoot) -OutputDirectory (Split-Path -Parent $source) } 'must not exist'
Assert-Rejected { & "$PSScriptRoot/TestPackagedRuntime.ps1" -EngineRoot $EngineRoot -PluginPackage $PluginPackage -Configurations @() } 'empty'
Write-Host 'Release/example path and empty-configuration guards passed without building or modifying a host.'
