# 简洁版信号接线图生成提示

生成方式：内置 image_gen。根据用户“有点太花了，简单点”的反馈，将接线图改为白底黑字。

最终提示词：

```text
Redesign this reference into an extremely simple BLACK-AND-WHITE wiring sheet. User feedback: "有点太花了，简单点" — too colorful and ornate, make it simple. Preserve correct wiring, but REMOVE ALL colors, gradients, shaded fills, capsule buttons, icons, thick borders, decorative headers, ground bus drawing, and large banners. Absolutely opaque WHITE paper background, solid BLACK typography and thin straight black arrows only. Calm technical document, plenty of whitespace, legible large Chinese type, balanced two columns, four simple groups separated by whitespace or very thin gray rules. A modest plain title, not huge. No illustrations. 1600x1200 or similar 4:3 landscape. Ensure all text fits inside generous white margins.

Title "STM32 信号接线图"
Subtitle in small black type "建议分配，尚待实物核对｜所有引脚都在同一块 STM32F103C8T6 上"

UPPER LEFT group title "电机驱动板 ①（马达 1、2）"
Column headers "STM32" and "DRV8833 ①"
Four plain text rows, one separate right-facing arrow per row:
PA0 → AIN1
PA1 → AIN2
PA2 → BIN1
PA3 → BIN2
Small line "A 通道：马达 1　B 通道：马达 2"

UPPER RIGHT group title "电机驱动板 ②（马达 3、4）"
Column headers "STM32" and "DRV8833 ②"
Four plain text rows:
PA6 → AIN1
PA7 → AIN2
PB0 → BIN1
PB1 → BIN2
Small line "A 通道：马达 3　B 通道：马达 4"

LOWER LEFT group title "两只舵机"
Two plain signal rows:
PB6 → MG995 的 S（翻转臂）
PB7 → SG90 的 S（夹爪）
Then three smaller simple explanatory lines:
"S：白色或橙色，接上面的信号引脚"
"V：红色，接 B 路 5V"
"G：黑色或棕色，接公共 GND"
Final small line "舵机电源从降压模块分支，不经过 STM32 板。"

LOWER RIGHT group title "PS2（通过转接板连接）"
Column headers "STM32" and "信号 / 接收器针号"
Four plain text rows with exactly one arrow each:
PB14 ← DI/DAT ①
PB15 → DO/CMD ②
PB12 → CS/SEL ⑥
PB13 → CLK ⑦
VERY IMPORTANT PB14 arrow MUST face LEFT; other three arrows face RIGHT.
Below rows, small plain text:
"④ 接公共 GND；③ ⑧ ⑨ 不接"
"B 路 5V → 转接板 → 板载稳压 → ⑤ VDD 3.3V"
"针号按用户照片朝向；转接板排针按丝印对应。"

Bottom of canvas, after thin gray horizontal rule, three short plain notes:
"所有模块 GND 相连；信号线不能代替电源线。"
"DRV8833 的 nSLEEP 运行时须为高电平，接法按实际板型确认。"
"按引脚名称接线；图中位置不是实物排针顺序。PA13 / PA14 留给 SWD 下载。"

Do not add any new text or hardware. All signal rows must be separate, no crossings, no joined arrows, no decorative colors. No gray shaded cards. White margins, opaque white background. Make it feel like an exceptionally clear simple instruction manual, not an infographic.
```

