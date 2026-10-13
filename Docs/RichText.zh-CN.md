# Rich Text

[English](RichText.md)

`UBMFontRichTextBlock` 继承 `URichTextBlock`，使用导入的 BMFont 资产渲染文本 run，而不是字体面。它是普通文本布局原语之上的适配层，不改变普通文本核心。

## 使用

1. 创建 **BMFont Rich Text** 控件，位于 Text 面板分类。
2. 将 **Font Asset** 指定为导入的 BMFont 资产。
3. 设置文本。标记按通常方式工作；带 decorator 标签的 run 通过 BMFont 渲染：

```html
<bmfont>Score: 1250</>
```

默认启用 **Decorate Plain Text Runs**，因此没有标签的 run 也使用 BMFont，普通字符串无须标记即可使用。不匹配 decorator 标签、也不属于普通文本处理范围的命名 run，仍使用 Rich Text 样式集和常规字体，从而在同一文本块中混合 BMFont 与常规字体 run。

默认标签名称为 `bmfont`，可以通过 **Decorator Tag** 修改。

## 行为与限制

- 每个 run 独立布局。Kerning 只在 run 内解析，跨标签边界的字形对不应用字偶距。
- Run 度量来自资产：行高使用 `common.lineHeight`，并按 **Font Scale** 缩放。Run 以行顶为基准；与更高的常规字体 run 共用一行时，BMFont run 在行内底部对齐。
- 染色和阴影来自控件自身外观属性，不来自 Rich Text 样式集。BMFont run 不查询样式表中的命名样式。
- Packed-channel 资产使用与普通文本控件相同的通道提取材质。
- 省略号、多行省略号和中间省略号策略优先使用资产中的 `U+2026`，否则使用三个句点字形，再回退到配置的缺字字形。末尾省略号遵循该行溢出方向，但渲染器整体仍受双向布局限制约束。
- 空的成对标签和匹配的自闭合标签产生空 BMFont run，不会把标记本身绘制成文本。
- 控件 setter 修改字体资产、缩放、间距和颜色时，通过重建受影响的 run 立即生效；调用者无须重新分词标记。
- 重导入已指定资产会推进数据版本。Run 在下一次布局失效时观察到变化，并在文本不变的情况下刷新测量位置。
- 适配器继承核心渲染器的范围：从左到右布局，不提供双向重排、OpenType shaping、连字或组合附加符定位。
