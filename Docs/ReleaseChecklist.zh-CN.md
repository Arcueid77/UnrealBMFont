# 发布清单

[English](ReleaseChecklist.md)

## 范围与元数据

- 选择版本，更新 `UnrealBMFont.uplugin`。正式发布时，用实际发布日期把更新日志的 `[Unreleased]` 标题替换为 `[version] - YYYY-MM-DD`。
- 确认 `IsBetaVersion` 与版本成熟度一致。
- 仓库文档和支持的最终公开 URL 确实存在后，再填写相关 URL。
- 确认所有新增源码、样例、图片和文档来源明确，许可证兼容。

## 验证

- 完成下列所有必需门槛。本地验收记录、审查报告和生成的验证材料保留在仓库外。
- 对每个声明支持的引擎构建，针对确切的干净发布提交运行 `Scripts/TestRelease.ps1`；至少需要一个完整构建。其证据必须满足 `Completed=true` 和 `ReleaseReady=true`。
- 写明确切引擎构建和 Win64 支持范围。其他 UE 小版本和 Launcher 发行版必须先独立验证，才可声明支持。
- 审查标志、材质和基准图变化。只有预期行为变更时才重新生成；不得仅为了让失败比较通过而更新基准 PNG。
- 对每个声明支持的目标平台运行 `Scripts/BuildPlugin.ps1`。
- 确认 UnrealEditor Development、UnrealGame Development 和 UnrealGame Shipping 全部成功，且没有插件新增的警告。
- 运行 `Scripts/TestPlugin.ps1`，归档 `index.json` 和编辑器日志。
- 在 GPU 上运行 `UnrealBMFont.Render.*`，不使用 `-NullRHI`，并归档报告。
- 按默认两种配置运行 `Scripts/TestPackagedRuntime.ps1`，归档其 `Evidence` 目录。
- 在干净项目中导入 `Samples/Minimal/Minimal.fnt`，检查 `AB`、回退、换行、染色、阴影和重导入。
- 打开已导入字体的资产检查器，确认摘要、图集字形框和字形表正常。
- 确认两种打包配置都报告真实 GPU 成功，包含四类像素案例及 PNG 证据，且没有强制 Cook 插件内容目录。
- 打开生成的展示项目，点击 Play，检查多语言行和普通文本、packed、多页渲染。
- 接受升级策略之前，在一次性副本中加载并重新保存有代表性的 beta 资产。

## 插件包审计

- 将插件包解压到干净目录。
- 确认 Runtime 不依赖 UnrealEd 或 AssetDefinition。
- 确认测试模块不进入 UnrealGame 目标。
- 确认打包描述为 `Installed=true`、`EnabledByDefault=false`。
- 确认插件包包含 `LICENSE`、供参考的中文许可证译文、第三方声明和全部文档的中英文版本、`Content/M_BMFontPacked.uasset`、展示模板，以及且仅有两个生成辅助脚本。
- 确认 `BuildPlugin.ps1` 报告自动发布文件检查和个人、源码路径审计通过。
- 扫描凭据、私有 URL、过期供应商名称及生成的宿主项目文件。

## 发布

- 对确切测试提交打标签。
- 必须有独立审查验收和最终发布工作流证据；仅元数据 CI 成功不能授权发布。
- 为发布归档附上不可变的校验值。
- 在发布说明中公布兼容性矩阵和已知限制。
- 不得声明本次发布未实际验证的平台、引擎版本、Cook 或运行时结果。
