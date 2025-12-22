# AAudio Demo (Android)

这是一个用于验证 AAudio 在设备变化/流断开时自动重建流的示例应用。点击 Start 会持续播放固定频率的正弦波，点击 Stop 停止。

## 功能
- 使用 AAudio 回调方式播放音频
- 注册 `AAudioStream_errorCallback`，在断开/错误时重建流
- 两个按钮控制播放/停止

## 运行环境
- Android Studio (带 NDK)
- Android 8.0+ (minSdk 26)

## 关键实现
- Java UI 与 JNI：`app/src/main/java/com/zxx/aaudio/MainActivity.java`
- AAudio 播放与重建逻辑：`app/src/main/cpp/native-lib.cpp`
- CMake 配置：`app/src/main/cpp/CMakeLists.txt`

## 构建与运行
1. 用 Android Studio 打开项目并 Sync
2. 连接设备或启动模拟器
3. 运行应用，点击 Start 开始播放

## 验证设备变化重建
可以尝试以下操作触发流断开：
- 插拔有线耳机
- 连接/断开蓝牙音频设备

日志中会出现类似提示（表示触发了重建）：
```
Restarting AAudio stream after device change/error
```

## 备注
- 目前没有处理音频焦点（Audio Focus）事件，如有需要可在 Java 层添加。
# AAudio
