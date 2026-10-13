# 贡献指南

[English](CONTRIBUTING.md)

欢迎改进 Unreal BMFont。请优先提交范围小、便于审查，并附有可复现验证结果的改动。

## 提交改动之前

- 先检索已有问题，避免重复提交。
- 行为变更需要说明受影响的 `.fnt` 格式、Unreal 版本和目标平台。
- 不要加入专有字体、许可证不明确的生成图集，或复制来的引擎、插件源码。
- Runtime 模块不得依赖仅编辑器可用的模块。

## 构建

先运行不依赖 Unreal Engine 的元数据和仓库检查：

```powershell
python ./Scripts/ValidateRepository.py
```

使用空的输出目录，并指定完整的 UE 5.8 安装目录或源码树：

```powershell
pwsh ./Scripts/BuildPlugin.ps1 `
  -EngineRoot "C:\Program Files\Epic Games\UE_5.8" `
  -OutputDirectory "$env:TEMP\UnrealBMFont-Package"
```

对于声明支持的平台，`BuildPlugin` 必须通过 UnrealEditor、UnrealGame Development 和 UnrealGame Shipping 三个目标。

Unreal Automation Tool 会先把整个插件目录复制到临时宿主，再应用 `FilterPlugin.ini`。引擎源码、本地构建缓存、插件包和自动化报告都应放在插件根目录之外。包装脚本在共享的中性构建目录中建立干净暂存副本，拒绝使用源码树内的输出目录，并在交付前扫描个人目录和源码路径泄漏。只有默认的共享 Documents 位置不可用时才指定 `-BuildRoot`；它仍须位于检出目录之外，且不应包含用户名。

## 测试

把打包后的插件安装到轻量宿主项目，再运行：

```powershell
pwsh ./Scripts/TestPlugin.ps1 `
  -EngineRoot "C:\Program Files\Epic Games\UE_5.8" `
  -Project "D:\Projects\BMFontHost\BMFontHost.uproject" `
  -ReportDirectory "$env:TEMP\UnrealBMFont-Reports"
```

解析、布局、导入或渲染行为变化时，需要新增或更新自动化用例。测试必须先确认查询结果有效，再解引用，避免回归问题直接导致编辑器崩溃而无法报告失败。

可能影响游戏目标、Cook 内容或模块启动的改动，必须用通过审计的插件包验证真实 Development 和 Shipping 宿主：

```powershell
pwsh ./Scripts/TestPackagedRuntime.ps1 `
  -EngineRoot "C:\Program Files\Epic Games\UE_5.8" `
  -PluginPackage "$env:TEMP\UnrealBMFont-Package"
```

此步骤比编辑器自动化更重。生成的宿主和验证材料必须保留在仓库外；声明打包运行时已验证时，不能用源码扫描或仅启动编辑器替代。

## Pull Request 清单

- 改动理由明确，没有无关的生成文件。
- 对外行为变更已同步更新相关文档的中英文版本，语言切换链接保持一致。
- 格式和兼容性声明与实际测试相符。
- Runtime 代码不依赖 Editor 模块即可构建。
- 自动化测试通过，并附上或概述报告。
- 需要版权头的新文件包含仓库要求的版权声明。
- 面向用户的改动已更新 `CHANGELOG.md`。

生成项目、本地人工验收清单、审查报告和发布验证材料不进入仓库。仓库文档保留公开的使用、API、开发和发布流程，不记录本地执行流水。

提交贡献即表示同意按本仓库的 MIT License 授权该贡献。
