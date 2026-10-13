# 可运行展示项目与打包渲染探针

[English](README.md)

这里是源码模板，不是已经生成的项目。从仓库构建插件包后运行：

```powershell
./Scripts/CreateExampleProject.ps1 -EngineRoot '<UE root>' -PluginPackage '<BuildPlugin output>' -OutputDirectory '<new external directory>'
```

命令会构建示例，导入仓库样例，设置最近邻过滤，保存所有资产，并创建 `Content/BMFont/ShowcaseMap.umap`。打开生成的 `BMFontExample.uproject`，点击 Play，可以看到数字、大写拉丁字母、平假名、中文数字、缺字回退、packed 通道和多页图集。

地图中的 actor 持有导入资产的硬引用。项目仅 Cook 地图及其依赖，不会强制 Cook 插件内容目录。

`Scripts/TestPackagedRuntime.ps1` 生成同一项目，打包 Development 和 Shipping，并通过 `-BMFontValidate` 启动 GPU 验证。四个探针逐像素与原创 Minimal、Packed 图案的独立掩码比较。即使像素不匹配，也会保存截图和机器可读结果。渲染探针明确拒绝 NullRHI，普通 Play 不会自动退出。

生成的资产、二进制和引擎专用项目文件应放在仓库外，不要把生成项目复制回插件根目录。
