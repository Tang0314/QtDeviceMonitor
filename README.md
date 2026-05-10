# QtDeviceMonitor

> 基于 Qt6/C++17 的工业设备上位机监控软件

![License](https://img.shields.io/badge/license-MIT-blue)
![Qt](https://img.shields.io/badge/Qt-6.7.3-green)
![Platform](https://img.shields.io/badge/platform-Windows-lightgrey)

## 功能特性

- 📊 实时数据采集与显示（温度、压力、状态）
- 📈 实时曲线图（双Y轴，温度/压力独立显示）
- ⚠️ 报警检测（超限/低限报警，颜色提示）
- 🗄️ 历史数据存储（SQLite 数据库）
- 📄 历史数据查询与导出（CSV 格式）
- ⚙️ 报警阈值可调节（设置页面）
- 🌐 TCP 通信模块（含内置虚拟设备）
- 🔧 内置 Mock 数据生成器（无需硬件即可测试）

## 技术栈

| 类别     | 技术                       |
| -------- | -------------------------- |
| 开发语言 | C++17                      |
| UI 框架  | Qt 6.7.3 Widgets           |
| 绘图     | Qt Charts                  |
| 通信     | Qt Network / Qt SerialPort |
| 数据库   | SQLite (Qt SQL)            |
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

**编译步骤：**
```bash
git clone https://github.com/Tang0314/QtDeviceMonitor.git
cd QtDeviceMonitor
# 用 Qt Creator 打开 CMakeLists.txt 编译运行
```

## 使用说明

### Mock 模式（内置虚拟数据）
1. 启动程序
2. 点击「▶ 开始采集」
3. 实时查看温度、压力曲线和报警记录

### TCP 模式（虚拟设备）
1. 启动程序
2. 点击「连接虚拟设备(TCP)」
3. 程序自动启动本地 TCP 服务器并连接

### 设置报警阈值
菜单 → 设置 → 报警阈值

### 查询历史数据
菜单 → 文件 → 历史查询

### 导出 CSV
菜单 → 文件 → 导出 CSV

## 项目结构

```
QtDeviceMonitor/
├── src/
│   ├── ui/          # 界面模块
│   ├── comm/        # 通信模块
│   ├── data/        # 数据模块
│   ├── alarm/       # 报警模块
│   ├── mock/        # 虚拟设备
│   └── config/      # 配置模块
├── resources/       # 资源文件
├── docs/            # 开发文档
└── CMakeLists.txt
```

## 开源协议

[MIT License](LICENSE)

## 作者

[@Tang0314](https://github.com/Tang0314)