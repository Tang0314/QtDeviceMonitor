- # 更新日志

  ## v1.1.0 (2026-05-13)

  ### 新增
  - 冷链仓储监控场景：扩展数据通道至5个（温度/湿度/压力/CO₂/门状态）
  - ChartWidget：三轴独立Y轴曲线图（温度/湿度/CO₂）
  - AlarmChecker：5通道独立报警检测（含门状态报警）
  - SettingsDialog：4通道报警阈值可视化配置（2×2网格布局）
  - HistoryDialog：历史查询加入湿度/CO₂/门状态字段
  - ConfigManager：配置持久化，重启保留阈值设置（INI文件）
  - SerialComm：串口通信模块（支持波特率/数据位/校验位配置）
  - SerialConfigDialog：串口参数配置对话框
  - VirtualTcpDevice：内置TCP虚拟设备服务端
  - TcpComm：TCP客户端通信模块，支持断线自动重连
  - virtual_serial_device.py：Python虚拟串口设备脚本

  ### 优化
  - 数据库表结构更新，支持新增字段
  - CSV导出加入UTF-8 BOM，解决Excel中文乱码
  - 报警阈值修改后自动重置报警状态

  ---

  ## v1.0.0 (2026-05-10)

  ### 新增
  - MockDataGenerator：内置正弦波虚拟数据生成器
  - ChartWidget：实时双轴曲线图（Qt Charts）
  - AlarmChecker：温度/压力超限报警检测
  - DatabaseManager：SQLite 历史数据存储（批量写入缓冲）
  - HistoryDialog：历史数据查询与 CSV 导出
  - SettingsDialog：报警阈值可视化配置
  - 菜单栏：文件/设置/操作三级菜单
  - CSV导出：支持时间范围筛选，UTF-8编码