# Packed 样例

[English](README.md)

这是最小的 packed-channel（`packed=1`）样例：字形覆盖率存放在绿色通道，其余通道均声明为零，用于人工检查通道提取材质路径。

- 将 `Packed.fnt` 导入为 BMFont 资产。
- 导入的页面纹理禁用 sRGB。
- 每个字形的 `chnl=2` 掩码选取绿色通道覆盖率，再应用控件颜色染色。

样例为本仓库原创生成内容，使用 MIT License。
