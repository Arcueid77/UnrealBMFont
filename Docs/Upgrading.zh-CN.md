# 升级到 1.0

[English](Upgrading.md)

1. 备份项目，并保留源 `.fnt` 和图集图片。
2. 关闭编辑器，换成针对目标引擎构建的插件包，或从源码重新编译。不要混用不同插件版本或引擎构建的 Editor DLL 和预编译游戏库。
3. 打开项目副本。现有 BMFont 资产和控件属性名称保持不变，无须类重定向或批量重命名。
4. 保存有代表性的资产，在副本中检查普通文本、Rich Text、换行、packed 通道、多页字体和重导入。
5. 打包 Development 和 Shipping，确认字体、页面纹理和 packed 材质通过资产引用进入 Cook。可运行示例演示了不使用 `DirectoriesToAlwaysCook` 的做法。

需要可重复的升级验证时，将 beta 资产和依赖复制到一次性生成项目中，保留原来的 `/Game` 路径。在两个独立编辑器进程中运行 `Scripts/TestAssetUpgrade.py`，将 `BMFONT_UPGRADE_ASSET` 设为对象路径：第一次设 `BMFONT_UPGRADE_PHASE=resave`，第二次设为 `verify`。测试在全新加载后比较每个字形的度量、码点集合、字体元数据、spacing、通道映射、kerning 和页面纹理引用。

这项测试不授权修改原始项目。

## C++ 调用者

控件创建后，使用 `SetFontData` 和 `SetPackedRenderMaterial`，不要直接给相应字段赋值。资产变化会自动通知已有控件。自定义订阅者可以使用 `OnChanged()`，但必须遵守游戏线程和非重入约定，详见[公开 API](API.zh-CN.md)。

## 脚本与验证材料

`TestPackagedRuntime.ps1` 需要 GPU，并检查真实导入资产和像素。其验证材料是 `Evidence/<Configuration>/result.json` 和 PNG，不再是仅包含文本的冒烟标记。示例在仓库外生成，测试后仍可用于人工检查。`TestPlugin.ps1` 默认保留 NullRHI；发布验证另外执行严格 GPU 测试套件。

## 更换引擎

选择引擎前阅读[兼容性说明](Compatibility.zh-CN.md)。支持一个引擎，不代表支持它与另一个已测试版本之间的所有小版本。Launcher 二进制和源码引擎需要分别验证。迁移到较旧引擎时，用目标引擎重新生成导入资产和 packed 材质，不要假定较新引擎的 `.uasset` 向后兼容。
