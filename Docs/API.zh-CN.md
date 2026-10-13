# 公开 API 与资产兼容性

[English](API.md)

## 1.x 稳定性约定

受支持的接口是 `Source/UnrealBMFont/Public` 下的导出声明：`UBMFontAsset`、`UBMFontText`、`UBMFontRichTextBlock`、`SBMFontText`、解析和布局的数据与设置，以及 `BMFontRendering`。私有 Slate run、编辑器实现细节、测试钩子和构建脚本内部实现不属于稳定的扩展 API。

在 1.x 中保留已有序列化属性名称和 Blueprint 类、函数路径。破坏性的公开 API 变更需要新的主版本，或明确记录弃用并提供受支持的替代接口。不承诺不同 Unreal 构建之间的 C++ ABI 稳定性：二进制必须针对确切引擎构建和目标配置重新编译。

## 修改方式与所有权

- 用 `SetFontData` 替换资产数据，用 `SetPackedRenderMaterial` 修改材质。这些接口会重建派生缓存并通知使用者。公开反射字段仍用于序列化和检查；直接在 C++ 中赋值不是受支持的缓存一致性修改方式。
- 控件构建后使用 setter。直接写反射 C++ 属性，不会自动同步已经存在的 Slate 控件。
- UObject 和控件缓存只能在游戏线程访问、修改。不涉及 UObject 的独立解析器，以及接受不可变 `FBMFontData` 的布局重载，可以在调用者持有输入且防止并发修改的条件下运行。
- `FindGlyph` 和 `FindPage` 返回借用指针。任何数据替换、重导入或属性编辑都会使其失效。需要跨越修改保留数据时，请复制数据。
- UMG 通过反射硬引用持有字体。直接使用 Slate 的调用者必须用 GC 可见的引用维持字体生命周期；`SBMFontText` 有意只保存弱引用，不拥有资产。
- `OnChanged()` 是原生通知。度量变化使布局失效；渲染资源变化使绘制失效。不再需要时移除订阅，或使用弱绑定。回调不得递归修改同一资产。

## Packed 材质约定

自定义材质必须是 UI domain 材质，并使用 `FontAtlas`（纹理）、`ChannelWeights`（向量）和 `ChannelBias`（标量）参数。通道提取得到覆盖率，Slate 顶点颜色提供文本染色。资产持有缓存的动态材质实例。资源版本变化后，不得继续保留或编辑旧实例。

材质加载在资产生命周期阶段尝试，并在渲染资源请求时重试一次。尚未解析成功的资产只以 4 Hz 查询已经加载的软引用对象，轮询本身绝不执行同步 IO。恢复成功后推进资源版本，并在之后的 tick 通知使用者，避免绘制过程中重入 Rich Text 布局。外部加载器可以调用 `RefreshRenderResources()` 立即采用已经加载的资源。

## 序列化 schema

`FBMFontCustomVersion::GUID` 标识 schema，版本 0 记录初始的 1.0 布局。没有版本标记的 beta 资产保留相同反射属性，无须破坏性转换。未来布局变更必须新增 schema 版本，必要时提供迁移代码，并测试旧版本样本。不得把同一版本号用于不同的序列化含义。

插件 schema 兼容性不能覆盖 Unreal 包兼容性：不保证较新引擎保存的资产能在较旧引擎打开。请在目标引擎中导入原始 `.fnt` 和图集，不要直接复制较新版本的 `.uasset`。
