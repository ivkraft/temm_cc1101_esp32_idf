# ESP-IDF Project T-Embed CC1101 - Board: [LilyGO T-Embed CC1101](https://lilygo.cc/en-us/products/t-embed-cc1101)

This is an attempt to build a complete, production-ready product entirely on the ESP-IDF SDK—avoiding the legacy issues that come with Arduino-based components and eliminating the dependency bloat and “noise” they often introduce.

This repository contains an ESP32 project built with **Espressif ESP-IDF**.  
Below are step-by-step instructions to install and initialize ESP-IDF, configure/build/flash the project, monitor logs over a COM/serial port, and push the project to GitHub from a local machine.

---

## Prerequisites

- Git
- Python 3 (recommended: the one installed by ESP-IDF Tools Installer)
- ESP-IDF (v4.4+ or v5.x recommended)
- USB-to-UART drivers for your board (CP210x / CH340 / FTDI), if required

---

## 1) Install and initialize ESP-IDF

### Windows (recommended: ESP-IDF Tools Installer)
1. Install **ESP-IDF Tools Installer** from Espressif.
2. Open **ESP-IDF Command Prompt** (it sets environment variables automatically).

Verify:
```bash
idf.py --version
python --version

Linux/macOS (ESP-IDF installed from source)

If you installed ESP-IDF by cloning the esp-idf repository:

cd ~/esp/esp-idf
./install.sh
. ./export.sh

Verify:

idf.py --version

2) Clone this repository and set the target

git clone <YOUR_REPO_URL>
cd <YOUR_PROJECT_FOLDER>

Set the target once (choose the correct chip):

idf.py set-target esp32
# alternatives: esp32s3 / esp32c3 / esp32c6 etc.

3) Configure the project

Open menuconfig:

idf.py menuconfig

Common settings:

    Serial flasher config → default baud rate

    Component config → logging level

    Wi-Fi / Bluetooth / partitions depending on your project

4) Build

idf.py build

Build artifacts are generated in the build/ folder.
5) Flash and monitor over COM/serial port
Find your serial port

Windows: Device Manager → Ports (COM & LPT) → COMx
Linux: typically /dev/ttyUSB0 or /dev/ttyACM0
macOS: typically /dev/cu.usbserial-* or /dev/cu.SLAB_USBtoUART
Flash + monitor (recommended)

Windows example:

idf.py -p COM5 -b 921600 flash monitor

Linux/macOS example:

idf.py -p /dev/ttyUSB0 -b 921600 flash monitor

Exit monitor:

    Ctrl + ]

6) Monitor only (no flashing)

Windows:

idf.py -p COM5 monitor

Linux/macOS:

idf.py -p /dev/ttyUSB0 monitor

7) Push the project to GitHub from a local machine
Option A: New repo (initialize Git locally and push)

From the project root:

git init
git add .
git commit -m "Initial commit"

Add a remote and push:

git remote add origin https://github.com/<user>/<repo>.git
git branch -M main
git push -u origin main

SSH variant:

git remote add origin git@github.com:<user>/<repo>.git
git branch -M main
git push -u origin main

Option B: Repo already connected (push updates)

git add .
git commit -m "Update"
git push

Common issues
Linux: permission denied on serial port

sudo usermod -aG dialout $USER

Log out and log back in after running the command.
Port is busy / monitor cannot open

    Close any other serial terminal using the same port

    Unplug/replug USB

    Double-check the selected COM/serial port

Flashing fails ("Failed to connect")

    Hold the BOOT button during flashing (board-dependent)

    Reduce baud rate:

idf.py -p COM5 -b 115200 flash monitor

Useful ESP-IDF commands

idf.py fullclean
idf.py reconfigure
idf.py size
idf.py partition-table
idf.py app
