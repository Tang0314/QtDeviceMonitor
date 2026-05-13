# QtDeviceMonitor

> 基于 Qt6/C++17 的冷链仓储工业设备上位机监控软件

![License](https://img.shields.io/badge/license-MIT-blue)
![Qt](https://img.shields.io/badge/Qt-6.7.3-green)
![Platform](https://img.shields.io/badge/platform-Windows-lightgrey)
![Version](https://img.shields.io/badge/version-1.1.0-orange)

## 功能特性

- 📊 实时采集显示（温度/湿度/压力/CO₂/门状态）
- 📈 实时三轴曲线图（温度/湿度/CO₂独立Y轴）
- ⚠️ 5通道报警检测（超限/低限/门开报警，颜色提示）
- 🗄️ 历史数据存储（SQLite，批量写入）
- 📄 历史查询与CSV导出（UTF-8，Excel直接打开）
- ⚙️ 报警阈值可视化配置（重启保留）
- 🌐 TCP通信（含内置虚拟TCP设备，支持断线重连）
- 🔌 串口通信（支持波特率/数据位/校验位配置）
- 🔧 内置Mock数据生成器（无需硬件即可运行）
- 🐍 Python虚拟串口脚本（配合虚拟串口工具测试）

## 技术栈

| 类别     | 技术                       |
| -------- | -------------------------- |
| 开发语言 | C++17                      |
| UI 框架  | Qt 6.7.3 Widgets           |
| 绘图     | Qt Charts                  |
| 通信     | Qt Network / Qt SerialPort |
| 数据库   | SQLite (Qt SQL)            |
| 配置     | QSettings (INI格式)        |
| 构建     | CMake 3.21+                |

## 快速开始

### 直接运行（无需编译）

1. 下载 [最新 Release](https://github.com/Tang0314/QtDeviceMonitor/releases)
2. 解压 zip 文件
3. 双击 `QtDeviceMonitor.exe` 运行

### 从源码编译

**环境要求：**
- Qt 6.7.3（含 Qt Charts、Qt SerialPort）
- CMake 3.21+
- MinGW 11.2.0 64-bit

```bash
git clone https://github.com/Tang0314/QtDeviceMonitor.git
cd QtDeviceMonitor
# 用 Qt Creator 打开 CMakeLists.txt 编译运行
```

## 使用说明

### Mock 模式（内置虚拟数据）
1. 启动程序
2. 点击「▶ 开始采集」
3. 实时查看温度、湿度、压力、CO₂曲线和报警记录

### TCP 模式
1. 点击「连接虚拟设备(TCP)」
2. 程序自动启动本地 TCP 服务器（127.0.0.1:8888）并连接

### 串口模式
1. 安装虚拟串口工具（HHD Virtual Serial Port Tools）
2. 创建 COM5 ↔ COM6 虚拟串口对
3. 运行虚拟设备脚本：
```bash
py docs/virtual_serial_device.py
```
4. 点击「连接串口」，选择 COM6，波特率 9600

### 设置报警阈值
菜单 → 设置 → 报警阈值（重启后自动恢复）

### 查询历史数据
菜单 → 文件 → 历史查询

### 导出 CSV
菜单 → 文件 → 导出 CSV

## 数据协议

CSV 文本帧格式（每帧以 `\r\n` 结尾）：

温度,湿度,压力,CO₂,门状态,状态码 -17.0,85.3,0.1012,856,0,OK

| 字段   | 类型   | 单位 | 说明                  |
| ------ | ------ | ---- | --------------------- |
| 温度   | float  | ℃    | 冷库温度，范围 -30~10 |
| 湿度   | float  | %    | 相对湿度，范围 0~100  |
| 压力   | float  | MPa  | 气压，范围 0.08~0.12  |
| CO₂    | float  | ppm  | 二氧化碳浓度          |
| 门状态 | int    | -    | 0=关闭，1=开启        |
| 状态码 | string | -    | OK/WARN/ALARM/ERROR   |

## 项目结构

### 项目结构

```text
QtDeviceMonitor/
├── src/
│   ├── ui/      # 界面模块（MainWindow/ChartWidget/SettingsDialog等）
│   ├── comm/    # 通信模块（TcpComm/SerialComm）
│   ├── data/    # 数据模块（DeviceData/DatabaseManager）
│   ├── alarm/   # 报警模块（AlarmChecker）
│   ├── mock/    # 虚拟设备（MockDataGenerator/VirtualTcpDevice）
│   └── config/  # 配置模块（ConfigManager）
├── docs/
│   ├── CHANGELOG.md
│   └── virtual_serial_device.py
├── resources/
└── CMakeLists.txt
```

## 开源协议

[MIT License](LICENSE)

## 作者

[@Tang0314](https://github.com/Tang0314)