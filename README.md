## 小狼毫个人修改版


## 前言
新增的配置如果没有特别说明，都是在 `weasel.custom.yaml` 中进行配置，如下例子:
```yaml
# weasel.custom.yaml
patch:
    "新增配置项": 值
```

## 变化

1. 新增全局中英文模式
```yaml
"global/ascii_mode": true # 默认 false
```
2. 修改输入法开关时会重置中英文模式的逻辑
3. 优化切换中英文时图标显示逻辑。
  在开启全局中英文模式时，仅在主动切换中英文或者第一个会话才会显示图标
4. 新增类似macOS下的CapsLock按键切换中英文模式，原版会导致键盘灯亮起，在其他的程序中被识别成大写键打开
```yaml
"global/macos_capslock": true # 默认 false
```
5. 新增配置项隐藏切换中英文时的图标显示
```yaml
"global/hide_tip_icon": true # 默认 false
```