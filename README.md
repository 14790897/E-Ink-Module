# E-Ink 小说阅读器

基于ESP32-C3和2.13英寸E-Ink显示屏的小说阅读器项目。

## 功能特点

- ✅ **双WiFi模式** - 支持STA模式连接路由器，失败时自动回退到AP模式
- ✅ **mDNS支持** - 使用 `http://littlebook.local` 轻松访问（STA模式）
- ✅ **Web管理界面** - 通过浏览器上传和管理小说
- ✅ **LittleFS文件系统** - 支持存储多本小说文件
- ✅ **文本自动分页** - 根据屏幕尺寸智能分页显示
- ✅ **E-Ink显示** - 支持全刷新和局部刷新
- ✅ **中文支持** - 支持显示中文文本
- ✅ **页码显示** - 实时显示当前页码和总页数
- ✅ **物理按钮控制** - 通过GPIO按钮控制翻页和浏览书籍列表
- ✅ **阅读进度保存** - 自动保存阅读记录到NVS，断电不丢失

## 硬件要求

- ESP32-C3开发板（合宙AIR-M2M Core ESP32-C3）
- 2.13英寸E-Ink显示屏（DEPG0213BN，SSD1680控制器）
- 分辨率：250x122像素

## 引脚连接

根据 `platformio.ini` 配置：

### E-Ink显示屏连接

| 功能 | GPIO |
|------|------|
| SPI CLK | 2 |
| SPI MOSI | 3 |
| SPI MISO | -1 (不使用) |
| CS | 7 |
| DC | 6 |
| RST | 10 |
| BUSY | 5 |

### 物理按钮连接

| 按钮 | GPIO | 功能 |
|------|------|------|
| Boot按钮 | 9 | 单击: 下一页/选择<br>双击: 上一页<br>长按: 进入/退出书籍列表 |
| UP按钮 | 12 | 在书籍列表中向上选择 |
| DOWN按钮 | 18 | 在书籍列表中向下选择 |

**注意**: GPIO 12和18需要连接物理按钮，按钮另一端接地(GND)。按钮应配置为下拉模式，按下时为LOW。

## 软件依赖

项目自动安装以下库（在 `platformio.ini` 中定义）：

- Adafruit GFX Library - 图形绘制
- GxEPD2 - E-Ink显示驱动
- ESP Async WebServer - 异步Web服务器
- ArduinoJson - JSON解析

## 使用步骤

### 1. 配置WiFi（可选）

编辑 `include/secrets.h` 文件，设置您的WiFi信息：

```cpp
#define WIFI_SSID "YOUR_WIFI_NAME"       // 您的WiFi名称
#define WIFI_PASSWORD "YOUR_PASSWORD"     // 您的WiFi密码
#define MDNS_HOSTNAME "littlebook"        // mDNS主机名
```

**WiFi模式说明：**
- 如果能连接到配置的WiFi，将使用 **STA模式**（推荐）
- 如果连接失败，会自动回退到 **AP模式**
  - AP SSID: `EInk-Reader`
  - AP 密码: `12345678`

### 2. 编译并上传固件

```bash
# 使用PlatformIO编译并上传
pio run --target upload

# 或在VS Code中按下 Upload 按钮
```

### 2. 上传文件系统数据

data目录包含Web界面文件，需要上传到ESP32的LittleFS：

```bash
# 上传文件系统
pio run --target uploadfs

# 或在VS Code中使用 "Upload File System image"
```

### 3. 连接设备

#### 方式A：STA模式（推荐）

如果ESP32成功连接到您的WiFi：

1. 确保您的电脑/手机与ESP32在**同一WiFi网络**
2. 可以使用以下方式访问：
   - **mDNS域名**：`http://littlebook.local` （推荐）
   - **IP地址**：查看串口输出获取IP地址

#### 方式B：AP模式（备用）

如果WiFi连接失败，ESP32会自动创建热点：

1. 连接到WiFi热点：
   - SSID: `EInk-Reader`
   - 密码: `12345678`
2. 访问：`http://192.168.4.1`

### 4. 打开Web界面

- 根据您的连接模式访问相应地址
- 界面会显示设备状态、书籍列表等信息

### 5. 上传小说

1. 点击"选择TXT文件"按钮
2. 选择一个TXT格式的小说文件（建议小于2MB）
3. 点击"上传"按钮
4. 等待上传完成

### 6. 开始阅读

有两种方式开始阅读：

#### 方式A: 通过Web界面

1. 在书籍列表中点击"阅读"按钮
2. 小说第一页会显示在E-Ink屏幕上
3. 可以通过Web界面的翻页按钮控制

#### 方式B: 通过物理按钮

1. **长按Boot按钮** (GPIO 9) 进入书籍列表界面
2. 使用 **UP按钮** (GPIO 12) 和 **DOWN按钮** (GPIO 18) 上下选择书籍
3. **单击Boot按钮**打开选中的书籍
4. 在阅读模式下：
   - **单击Boot按钮**: 下一页
   - **双击Boot按钮**: 上一页
   - **长按Boot按钮**: 返回书籍列表

## 文件结构

```
E-Ink Module/
├── data/                      # LittleFS文件系统数据
│   ├── index.html            # Web管理界面
│   ├── app.js                # 前端JavaScript逻辑
│   └── books/                # 小说存储目录
│       └── 三体节选.txt      # 示例小说
├── src/
│   └── main.cpp              # 主程序
├── platformio.ini            # PlatformIO配置
└── README_NOVEL.md           # 本文档
```

## API接口

Web界面通过REST API与ESP32通信：

| 接口 | 方法 | 说明 |
|------|------|------|
| `/api/status` | GET | 获取设备状态（当前页、总页数、当前书籍、存储空间） |
| `/api/books` | GET | 获取书籍列表 |
| `/api/upload` | POST | 上传书籍文件 |
| `/api/read` | POST | 打开书籍开始阅读 |
| `/api/delete` | POST | 删除书籍 |
| `/api/next` | POST | 下一页 |
| `/api/prev` | POST | 上一页 |

## 文本显示参数

在 `main.cpp` 中可以调整显示参数：

```cpp
const int CHAR_WIDTH = 12;           // 字符宽度（像素）
const int LINE_HEIGHT = 16;          // 行高（像素）
const int MARGIN_LEFT = 8;           // 左边距
const int MARGIN_TOP = 8;            // 上边距
const int MARGIN_RIGHT = 8;          // 右边距
const int MARGIN_BOTTOM = 20;        // 下边距
```

根据这些参数计算：
- 每行字符数：约 19 字符
- 每页行数：约 6 行
- 每页字符数：约 114 字符

## 注意事项

1. **WiFi配置**：
   - STA模式：修改 `include/secrets.h` 配置WiFi
   - AP模式：自动启用（SSID: `EInk-Reader`, 密码: `12345678`）
   - mDNS域名：`littlebook.local`（仅STA模式可用）
2. **文件大小限制**：建议单个TXT文件不超过2MB
3. **文件编码**：TXT文件应使用UTF-8编码
4. **E-Ink刷新**：
   - 全刷新约需4秒
   - 局部刷新约需0.75秒
   - 建议每10次局部刷新后进行一次全刷新以防止残影
5. **WiFi范围**：
   - STA模式：取决于您的路由器
   - AP模式：ESP32热点有效范围约10-20米

## 未来改进

- [x] 添加物理按钮控制翻页
- [x] 支持阅读进度保存
- [ ] 支持书签功能
- [ ] 优化中文字符显示
- [ ] 添加字体大小调节
- [ ] 支持更多文件格式（EPUB等）
- [ ] 电池电量显示
- [ ] 深度睡眠模式以节省电量

## 许可证

MIT License

## 作者

Created with ❤️ for E-Ink reading enthusiasts


C:\Program Files\VirtViewer v11.0-256\bin>remote-viewer --full-screen C:\Users\13963\Downloads\pve-spice.vv
