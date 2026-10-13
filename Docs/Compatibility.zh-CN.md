# 兼容性

[English](Compatibility.md)

初始支持范围为 **UE 5.8.0 源码构建、CL 0 / Win64**。下表记录参考环境及该范围使用的验证项目。发布证据应绑定确切测试提交，保留在仓库外；它不能替代人工交互验收或发布授权。

| 项目 | 已验证 | 尚未声明支持 |
| --- | --- | --- |
| Unreal Engine | 5.8.0 源码构建、CL 0 | Launcher 发行版；5.7 及更早版本；未来小版本 |
| 宿主平台 | Windows 11、Win64 | Linux、macOS |
| 目标 | UnrealEditor Development、UnrealGame Development、UnrealGame Shipping | 主机和移动平台 |
| 打包渲染 | Development 和 Shipping 中使用真实导入资产的 GPU 像素探针，覆盖普通文本、packed、多页和 Rich Text | 其他引擎与平台组合 |
| 控件渲染 | 在参考 GPU、驱动上与仓库基准图比较的 GPU 截图测试 | 超过比较容差的跨厂商 GPU 差异 |
| 描述文件 | 文本、紧凑 XML、二进制 v3 | 二进制 v1/v2 |
| 页面图片 | PNG | TextureFactory 接受的全部格式 |
| Packed 图集 | 通过内置 UI 材质提取字形覆盖率 | 单独合成 outline 通道 |

Runtime 模块使用平台无关的 Engine、Slate、SlateCore、UMG 和 XmlParser API，但只有在目标平台编译并运行 CI 验证之后，才能确认可移植性。

1.0 发布范围是 UE 5.8.0 源码构建 / Win64。最终验证至少需要一个完整引擎构建，并测试所有传入构建。其他 UE 小版本和 Launcher 发行版属于后续兼容性工作，缺少它们不会阻止这个限定范围的发布。现有证据没有确立任何早于 5.8 的最低支持版本。

预编译归档只适用于发布记录中的引擎构建和目标配置。已验证的源码引擎二进制，不代表适用于所有 UE 5.8 构建或 Launcher 发行版。目标引擎构建不匹配时，必须在目标引擎中重新编译插件。

源码分发有意不在插件描述中写入 `EngineVersion`，方便开发者尝试较新的兼容引擎；省略该字段不是向前兼容承诺。发布应注明确切测试的引擎 CL、版本，并按引擎版本重新生成打包二进制。
