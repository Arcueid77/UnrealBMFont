# 测试

[English](Testing.md)

## 自动化覆盖

`UnrealBMFont.*` 套件覆盖：

- 带 BOM、引号名称、任意字段顺序、多页和增补 Unicode 码点的文本描述。
- XML 声明和文档位于同一行的紧凑 XML。
- 生成的二进制 v3 块、有符号偏移和 kerning。
- 无效度量和图集校验。
- 可配置的描述大小、文本/XML 结构、记录数、页面名称、图集尺寸及多页总像素上限。
- 截断二进制前缀、不可能的块大小、固定种子的畸形字节语料、诊断洪泛，以及 XML `DOCTYPE` 拒绝。
- Kerning、tracking、字形偏移、换行、显式行、制表符、回退、缺字和行高语义。
- 通过 `UBMFontFactory` 导入真实生成的 PNG 和 `.fnt`。
- 随包提供的多语言 Showcase 样例，包括代表性拉丁字母、平假名、中文和回退码点。
- UI 纹理默认值、源路径跟踪、多页重导入事务与回滚，以及保留用户修改的纹理过滤方式。
- Slate 期望尺寸计算，以及文本变化后的失效处理。
- 对生成的 20,000 字形资产进行布局范围内的画刷创建检查。
- 资产度量通知、Slate 重建、packed 材质恢复，以及直接使用 Slate 时的弱引用垃圾回收。
- 既有 Rich Text run 中仅资源变化的画刷失效，以及已保存包中的 schema GUID。
- `UBMFontRichTextBlock` 的 run 测量，包括 kerning、回退、带标签与空 run、运行时指定字体、继承前景颜色、阴影边界，以及父级裁剪下省略号绘制的截图覆盖。
- Packed 通道映射、经 `UBMFontFactory` 的 packed 导入与重导入、实际保存中的打包材质依赖，以及按页面和字形通道分离渲染资源。

## 截图渲染测试

`UnrealBMFont.Render.*` 通过 `FWidgetRenderer` 离屏绘制 `SBMFontText`，并与 `Samples/GroundTruth` 中的基准 PNG 比较像素。它们需要真实 RHI：在 `-NullRHI` 下只记录信息级跳过并通过，所以默认无 GPU 的运行可以保持成功。

在 GPU 上运行：

```powershell
& "C:\Path\To\Engine\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "D:\Projects\BMFontHost\BMFontHost.uproject" `
  -Unattended -NoSplash -NoSound -NoP4 -NoPause -stdout -FullStdOutLogOutput `
  -ExecCmds="Automation RunTests UnrealBMFont.Render" `
  -TestExit="Automation Test Queue Empty" `
  -ReportExportPath="$env:TEMP\UnrealBMFont-RenderReports"
```

基准图保存在仓库中，视为源码。只有预期的渲染行为发生变化后，才在参考 GPU、驱动上主动重新生成，并审查 PNG 差异：

```powershell
# Same command as above, plus:
  -UpdateBMFontGroundTruth
```

比较允许少量通道误差和少量不同像素，避免轻微驱动、采样器差异造成不稳定；结构性渲染变化仍会失败。`UnrealBMFont.Render.PackedAtlas` 检查不同通道中的重叠字形矩形；`UnrealBMFont.Render.RichTextEllipsis` 检查自定义 Rich Text 绘制是否遵循溢出策略。

## 对宿主项目运行

宿主必须安装并启用当前插件。

```powershell
pwsh ./Scripts/TestPlugin.ps1 `
  -EngineRoot "C:\Program Files\Epic Games\UE_5.8" `
  -Project "D:\Projects\BMFontHost\BMFontHost.uproject" `
  -ReportDirectory "$env:TEMP\UnrealBMFont-Reports"
```

脚本使用 NullRHI 启动 `UnrealEditor-Cmd`，等待自动化队列清空；进程或 JSON 报告存在失败时，脚本失败。

## 插件包验证

```powershell
pwsh ./Scripts/BuildPlugin.ps1 `
  -EngineRoot "C:\Program Files\Epic Games\UE_5.8" `
  -OutputDirectory "$env:TEMP\UnrealBMFont-Package" `
  -TargetPlatforms Win64
```

包装脚本先在仓库外创建干净源码暂存副本，排除本地引擎树、构建产物、报告和之前的插件包。内部宿主和插件包位于共享的中性构建目录，可通过 `-BuildRoot` 覆盖，避免编译器路径暴露开发者个人目录或源码检出路径。这很重要，因为 UE 5.8 的 `BuildPlugin` 会先复制完整输入插件到临时宿主，再应用 `FilterPlugin.ini`。

两个发布脚本都会等待已有的 AutomationTool 进程，避免其他 Unreal 构建暂时占用引擎级 UAT 互斥锁时直接导致发布失败。

复制到 `OutputDirectory` 前，包装脚本确认打包描述同时为 installed 和显式 opt-in，检查必需的许可证、README 和兼容性文件，默认移除 PDB，并扫描每个打包文件中的个人目录和源码路径。原始 `BuildPlugin` 会移除 `EnabledByDefault`；把这种未修正的项目本地插件描述直接发布，可能阻止纯内容项目生成包含插件的游戏目标。

不要把静态源码扫描描述为成功的运行时测试。发布流程应保留 BuildPlugin 日志、自动化 `index.json`、Cook 日志和打包运行时验证材料。

## 打包运行时验证

`BuildPlugin.ps1` 成功后运行：

```powershell
pwsh ./Scripts/TestPackagedRuntime.ps1 `
  -EngineRoot "C:\Program Files\Epic Games\UE_5.8" `
  -PluginPackage "$env:TEMP\UnrealBMFont-Package"
```

脚本在仓库外生成[可运行展示项目](../Examples/Showcase/README.zh-CN.md)，导入并保存真实样例，再 Cook 展示地图的资产依赖，不强制 Cook 插件目录。Development 和 Shipping 在真实 GPU 上启动，并将普通文本、packed、多页和 Rich Text 的像素与独立定义的样例掩码比较。每个配置输出四张探针 PNG、一张完整 `Showcase.png` 和 `result.json`；缺少案例、像素不符、资产缺失或使用 NullRHI 都会失败。显式传入的 `-WorkDirectory` 必须尚不存在，防止旧结果通过新一轮运行。应保留完整 `Evidence` 目录，包括导入、构建和 Cook 日志。

默认无 GPU 自动化仍会跳过 GPU 截图案例。另用 `-DisableNullRHI -Strict` 重跑完整套件，要求真实渲染，包括验证 packed 图集预览和缩略图的 `UnrealBMFont.Editor.AtlasPreviewPackedChannels`。`-Strict` 也会拒绝带警告的测试结果；任何模式都会拒绝不完整报告。最终发布验证采用这一完整 GPU 阶段，而非仅筛选 `UnrealBMFont.Render.*`。

## 最终发布验证

`Scripts/TestRelease.ps1` 为每个传入引擎运行插件包审计、两种打包 GPU 配置、完整自动化和 GPU 自动化。最终模式要求干净提交，至少一个完整的 Win64 引擎构建即可，源码构建也满足条件。所有传入构建都必须通过。脚本记录各引擎版本和 installed-build 标记，检查源码提交与输入指纹始终不变，并输出 `release-evidence.json`。使用 `-CandidateOnly` 的开发运行、预发布版本和 beta 描述，即使 `AutomatedGatesPassed` 为真，也明确不能作为正式发布证据。详见[发布清单](ReleaseChecklist.zh-CN.md)。生成材料和本地验收记录保留在仓库外；自动化成功不代表交互验收已完成，也不授权发布。

`Scripts/TestScriptGuards.ps1` 无须启动构建即可检查源码树内输出、已有目录和空配置的拒绝行为。需要提供参考引擎和已构建插件包。`Scripts/TestAssetUpgrade.py` 在两个编辑器进程之间检查复制的 beta 资产，详见[升级说明](Upgrading.zh-CN.md)。

## GitHub Actions

不依赖引擎的仓库验证任务在每次 push 和 pull request 运行。开发用途的 UE 5.8 任务需要显式启用，pull request 永远不会在持久化自托管 runner 上执行。元数据检查成功不代表最终发布获准。

启用 Unreal 任务时，注册带 `ue-5.8` 标签的 Windows x64 自托管 runner，再设置仓库变量：

- `UE_CI_ENABLED=true`
- `UE_ROOT` 指向 runner 上的 Unreal Engine 5.8 安装目录或源码树根目录。

Runner 需要可用 GPU，以执行打包像素验证。`UE_GPU_CI_ENABLED=true` 为开发任务添加编辑器截图套件。单独手动触发的 `Release validation` 工作流至少需要一个完整引擎构建，并强制所有 GPU 和插件包门槛；它不提供可选的 GPU 跳过，也不会自动发布 release。
