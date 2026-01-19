---

layout: page

title: Installation Guide

---



This guide describes the supported ways to install \*\*CLI Calculator\*\* on Linux and Windows.



---



\## Option 1: Official Install Script (Debian-based Linux only)



This is the recommended and simplest installation method for Debian-based distributions (Ubuntu, Linux Mint, etc.).



```bash

curl -fsSL https://clicalculator.blob.core.windows.net/main/quick-install.sh -o install.sh

chmod +x install.sh

sudo ./install.sh

```



After installation, run:



```bash

calculator        # Launches the CLI menu

calculator\_gui    # Launches the GUI

```



---



\## Option 2: Manual Installation (Latest Release)



Use this method if you prefer manual control or are installing on non-Debian systems.



\### 1. Download



Download the latest prebuilt release from:



\[https://github.com/Benedek553/cli-calculator/releases](https://github.com/Benedek553/cli-calculator/releases)



---



\### 2. Linux



1\. Navigate to the extracted release directory:



```bash

cd cli-calculator-ubuntu

```



2\. Make the binaries executable:



```bash

chmod +x calculator calculator\\\_gui

```



3\. Move the binaries into your PATH:



```bash

sudo mv calculator calculator\\\_gui /usr/local/bin

```



4\. Run the applications:



```bash

calculator        # Launches the CLI application

calculator\\\_gui    # Launches the GUI application

```



---



\### 3. Windows



1\. Open \*\*File Explorer\*\*

2\. Navigate to your \*\*Downloads\*\* folder (or the location of the downloaded ZIP file)

3\. Extract the archive

4\. Locate the `.exe` files

5\. Double-click the desired executable to run it



> \\\*\\\*Note\\\*\\\*

> If Windows SmartScreen blocks the application, click \\\*\\\*More info\\\*\\\* → \\\*\\\*Run anyway\\\*\\\*.

