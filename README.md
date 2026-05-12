# Bookget X (Qt6)

Bookget X 使用 Qt6 重写，主要是简化操作难度。

- 支持 Windows / Linux / macOS 原生UI
- 通用图片批量下载器，支持真人验证码
- URL hash 下载记录持久化
- 下载失败后按“重试间隔(秒)”等待再重试（默认 3 秒）

## 目录结构

- `src/main.cpp` 程序入口
- `src/MainWindow.h/.cpp` 主界面与下载流程
- `src/HashStore.h/.cpp` 下载记录存储
- `CMakeLists.txt` 构建配置

## 构建

```bash
cmake -S . -B build
cmake --build build -j
```
## 运行

```bash
./build/bookget
```
## 一键构建
- `./build_mac.sh` macOS 构建
- `./build_linux.sh` Linux 构建
- `./build_win.ps1` Windows 构建

```bash
./build_mac.sh
./build_linux.sh
./build_win.ps1
```

## GitHub Actions 自动构建

已添加工作流：`.github/workflows/build-cross-platform.yml`

- 自动编译：Windows x64 / Linux x64 / macOS arm64
- macOS 产物：`bookget.app` + `bookget.dmg`
- 当配置签名证书后，macOS 构建会对 `.app` 和 `.dmg` 进行签名
- 工作流会安装完整的常用 Qt6 模块集（含 `qtwebengine` / `qtsvg` / `qtvirtualkeyboard` 等）
- 工作流仅安装本项目实际使用的 Qt6 模块：`Qt6::Core` / `Qt6::Widgets` / `Qt6::Network` / `Qt6::WebEngineWidgets` / `Qt6::Gui`（installer 模块：`qtbase` `qtwidgets` `qtnetwork` `qtwebengine` `qtgui`）

### 打包选项

- 打包产物：
  - Linux: tar.xz（`build/bookget-linux-x64.tar.xz`）
  - Windows: zip（`build/bookget-windows-x64.zip`）
  - macOS: dmg（`build/bookget-macos-arm.dmg`）

（CI 使用动态 Qt 库，打包时会包含构建输出和需要的共享库。）

注：Linux job 会使用 `linuxdeployqt`，Windows job 会使用 `windeployqt` 来把 Qt 运行时捆绑入最终包，使产物在目标机器上更易运行。

### 可选 Secrets（用于 macOS Developer ID 签名）

- `MACOS_CERTIFICATE_P12`：Base64 编码的 `.p12` 证书
- `MACOS_CERTIFICATE_PASSWORD`：`.p12` 证书密码
- `MACOS_DEVELOPER_IDENTITY`：签名身份，例如 `Developer ID Application: Your Name (TEAMID)`

未配置以上 Secrets 时，工作流仍会生成 DMG，但只会使用 ad-hoc 方式签名 `.app`。

## 软件运行效果图
![bookgetx](https://zhudw.cn/bookgetx/screenshot.png)

## 编译依赖
- Qt6 https://mirrors.ustc.edu.cn/qtproject/archive/online_installers/4.10/
- C++17
- CMake 3.16+
- Windows: Visual Studio 2022+ https://visualstudio.microsoft.com/zh-hans/downloads/
- Linux: g++ 9+
- macOS: Xcode 12+
- 第三方库：Qt6 Network, Qt6 Widgets, Qt6 Core, Qt6 WebEngineWidgets
  - Qt6 Network 用于 HTTP 请求和验证码处理
  - Qt6 Widgets 用于构建用户界面
  - Qt6 Core 用于文件操作和数据存储
  - Qt6 WebEngineWidgets 用于浏览器预览图片
## 许可证
本项目使用 MIT 许可证，详见 LICENSE 文件。
