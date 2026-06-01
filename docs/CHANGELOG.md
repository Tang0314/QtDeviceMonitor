# 更新日志

## v1.3.0 (2026-06-01)

### 新增
- DeviceMonitorController：新增核心控制器层，统一采集生命周期、数据源互斥切换、报警检测和数据库写入编排。
- DatabaseWorker：数据库写入线程增加定时 flush，降低异常退出时最近数据丢失风险。
- DataParser：新增 ParseResult，返回解析失败原因，便于通信层记录坏帧。
- Qt Test：新增 DataParserTest 和 AlarmCheckerTest，并通过 CTest 集成到根 CMake。
- 根 CMake 集成 virtual_device 子项目，打开根项目即可构建主程序、虚拟设备和测试。

### 优化
- MainWindow 瘦身为 UI 展示和用户交互层，不再直接持有 Mock/TCP/Serial/Alarm/Database 等核心模块。
- TCP/串口接收缓冲增加上限保护，坏帧会触发 frameDropped 信号并写入日志。
- SQLite 主线程查询连接和 worker 写入连接使用唯一连接名，补充 busy_timeout、WAL 和 alarm_log 时间索引。
- CSV 协议解析增加字段数量、数值范围、门状态、状态码合法性校验。
- 报警配置增加归一化防御，配置文件异常时回退到默认阈值。
- 日志系统增加重复安装保护、UTF-8 输出和 shutdown 清理。
- 独立 VirtualDevice GUI 复用 MockDataFormulas，避免与主程序内置虚拟设备数据公式漂移。

### 修复
- TcpComm::connectToDevice 重新启用自动重连，避免主动断开后再次连接不再重连。
- SerialComm 打开前清理旧连接和残留缓冲。
- 数据库批量写入改用稳定的 bindValue 复用 prepared query，补充事务失败日志。

## v1.2.0 (2026-05-29)

### 新增
- DataSource 状态机：Mock/TCP/串口三种数据源互斥切换，统一 UI 状态管理。
- AlarmHistoryDialog：报警历史查询对话框（按时间范围检索 alarm_log 表）。
- HistoryDialog 分页：每页 100 条，支持上一页/下一页导航。
- LogManager：文件日志系统，按日期写入 userdata/app.log。
- 状态栏：显示当前数据源、记录数、运行时长。

### 重构
- 提取 DataParser（TcpComm 和 SerialComm 共用 CSV 帧解析）。
- 提取 MockDataFormulas（MockDataGenerator 和 VirtualTcpDevice 共用正弦波生成公式）。
- 集中 AlarmDefaults 默认阈值常量，消除 ConfigManager/SettingsDialog 中的硬编码。

### 修复
- ChartWidget 温度 Y 轴范围 -30~-10 → -30~10，避免数据裁剪。
- SettingsDialog 补充 CO₂ 上限>下限校验。
- HistoryDialog 删除重复的表格属性设置。
- VirtualTcpDevice::start() 补上 m_time 重置。
- CMakeLists.txt 删除 SerialComm/SerialConfigDialog 重复声明。
- CSV 帧精度提升，减少 TCP 模式与 Mock 模式的数据偏差。

---

## v1.1.0 (2026-05-13)

### 新增
- 冷链仓储监控场景：扩展数据通道至5个（温度/湿度/压力/CO₂/门状态）。
- ChartWidget：三轴独立Y轴曲线图（温度/湿度/CO₂）。
- AlarmChecker：5通道独立报警检测（含门状态报警）。
- SettingsDialog：4通道报警阈值可视化配置（2×2网格布局）。
- HistoryDialog：历史查询加入湿度/CO₂/门状态字段。
- ConfigManager：配置持久化，重启保留阈值设置（INI文件）。
- SerialComm：串口通信模块（支持波特率/数据位/校验位配置）。
- SerialConfigDialog：串口参数配置对话框。
- VirtualTcpDevice：内置TCP虚拟设备服务端。
- TcpComm：TCP客户端通信模块，支持断线自动重连。
- virtual_serial_device.py：Python虚拟串口设备脚本。

### 优化
- 数据库表结构更新，支持新增字段。
- CSV导出加入UTF-8 BOM，解决Excel中文乱码。
- 报警阈值修改后自动重置报警状态。

---

## v1.0.0 (2026-05-10)

### 新增
- MockDataGenerator：内置正弦波虚拟数据生成器。
- ChartWidget：实时双轴曲线图（Qt Charts）。
- AlarmChecker：温度/压力超限报警检测。
- DatabaseManager：SQLite 历史数据存储（批量写入缓冲）。
- HistoryDialog：历史数据查询与 CSV 导出。
- SettingsDialog：报警阈值可视化配置。
- 菜单栏：文件/设置/操作三级菜单。
- CSV导出：支持时间范围筛选，UTF-8编码。
