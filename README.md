<img src="readme/splash.svg" width="100%" />

---
<p align="center">
<a href="https://travis-ci.org/vicr123/theTerminal"><img src="https://img.shields.io/travis/vicr123/theterminal/blueprint?label=Linux%2C%20macOS&style=for-the-badge" alt="Travis CI Build Status" /></a>
<img src="https://img.shields.io/github/license/vicr123/theterminal?style=for-the-badge" />
</p>

theTerminal is a terminal emulator designed to integrate well with theShell and come with many features.

---

# Dependencies
- Qt 5
  - Qt Core
  - Qt GUI
  - Qt Widgets
  - Qt WebEngine
  - Qt SVG
- [the-libs](https://github.com/vicr123/the-libs)
- [tttermwidget](https://github.com/vicr123/tttermwidget)

# Get
If you're using a supported operating system, we may have binaries available:

| System | Package |
|-------------------|---------------------------------------------------------------------------------------------------------|
| macOS | ~~[Application Bundle on GitHub Releases](https://github.com/vicr123/theTerminal/releases)~~ Coming soon! |
| Arch Linux | `theterminal` on the AUR |
| Other Linux | [AppImage on GitHub Releases](https://github.com/vicr123/theTerminal/releases) |

## Build
Run the following commands in your terminal. 
```
qmake
make
```

## Install
On Linux, run the following command in your terminal (with superuser permissions)
```
make install
```

On macOS, drag the resulting application bundle (`theTerminal.app` or `theTerminal Blueprint.app`, depending on which branch you're building) into your Applications folder

# Contributing
Thanks for your interest in theTerminal! Check out the [CONTRIBUTING.md](CONTRIBUTING.md) file to get started and see how you can help!

---

> © Victor Tran, 2019. This project is licensed under the GNU General Public License, version 3, or at your option, any later version.
> 
> Check the [LICENSE](LICENSE) file for more information.