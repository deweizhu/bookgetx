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

## 软件运行效果图
![bookgetx](https://zhudw.cn/bookgetx/screenshot.png)

