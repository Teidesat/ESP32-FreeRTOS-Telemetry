# 🛰️ TEIDESAT - ESP32 Development Setup Guide

## 🎯 Requirements

### Hardware
- ESP32 development board (ESP32-DOIT-DEVKIT-V1 recommended)
- USB cable (data capable)
- Computer with Linux (Gentoo tested)

### Software
- Visual Studio Code
- PlatformIO IDE extension
- Git

## 🔧 ESP32 Setup

### 1. Install PlatformIO in VS Code
1. Open VS Code
2. Go to Extensions
3. Search for "PlatformIO IDE"
4. Install the extension

### 2. Configure USB Permissions (Linux/Gentoo)

#### Add user to required groups:
```bash
sudo usermod -a -G dialout,uucp,tty,plugdev $USER
```

#### Create udev rules for ESP32:
```bash
sudo nano /etc/udev/rules.d/49-esp32.rules
```

#### Add the following content:
https://raw.githubusercontent.com/platformio/platformio-core/develop/platformio/assets/system/99-platformio-udev.rules

#### Reload udev rules:
```bash
sudo udevadm control --reload-rules
sudo udevadm trigger
```

#### Verify Setup
```bash
#  Connect ESP32 and check permissions
ls -la /dev/ttyUSB0
# Should show: crw-rw-rw- 1 root dialout
```
