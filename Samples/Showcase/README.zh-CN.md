# 多语言展示样例

[English](README.md)

`Showcase.fnt` 和 `Showcase.png` 组成较完整的人工检查样例。只需导入 `Showcase.fnt`，导入器会自动解析旁边的 PNG。

图集包含 135 个不同码点：

- 空格、ASCII 数字 `0`–`9` 和大写拉丁字母 `A`–`Z`。
- `U+3041` 到 `U+3096` 的全部码点，覆盖标准平假名、小字形、浊音和历史形式。
- 日文长音符号 `ー`（`U+30FC`）。
- 中文数字 `零一二三四五六七八九`。
- 用于回退测试的替换字符 `U+FFFD`。

建议测试文本：

```text
0123456789
ABCDEFGHIJKLMNOPQRSTUVWXYZ
あいうえお かきくけこ さしすせそ
がぎぐげご ざじずぜぞ ぱぴぷぺぽ
零一二三四五六七八九
Missing: ?
```

问号 `?` 有意不包含在图集中，默认回退设置下应显示为 `U+FFFD`。

字形图片使用官方 [Noto CJK 项目](https://github.com/notofonts/noto-cjk)中的 Noto Sans SC 生成，字体使用 [SIL Open Font License 1.1](https://github.com/notofonts/noto-cjk/blob/main/Sans/LICENSE)。仓库不附带字体文件。重新生成时，从该项目下载 `NotoSansSC[wght].ttf`，然后运行：

```powershell
pwsh ./Scripts/GenerateBrandAssets.ps1 -ShowcaseFontFile "C:\Fonts\NotoSansSC[wght].ttf"
```
