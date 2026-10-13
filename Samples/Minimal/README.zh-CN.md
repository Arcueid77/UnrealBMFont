# 最小样例

[English](README.md)

`Minimal.fnt` 和 `Minimal.png` 是原创测试样例，包含空格、`A`、`B` 和替换字符 `U+FFFD`。导入 `Minimal.fnt` 即可，页面文件会自动解析。

建议测试文本：

```text
AB BA
A missing glyph: C
```

`C` 会解析为替换字形。在 Windows 上，可用 `Scripts/GenerateBrandAssets.ps1` 重新生成图集和插件标志。
