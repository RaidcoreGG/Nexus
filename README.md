[![](https://discordapp.com/api/guilds/410828272679518241/widget.png?style=banner2)](https://discord.gg/Mvk7W7gjE4)
[![](https://raidcore.gg/Resources/Images/Patreon.png)](https://www.patreon.com/bePatron?u=46163080)

![](https://img.shields.io/github/v/release/RaidcoreGG/Nexus?style=for-the-badge&labelColor=%23131519&color=%230F79AA)

![](https://img.shields.io/github/downloads/RaidcoreGG/Nexus/total?style=for-the-badge&labelColor=%23131519&color=%230F79AA&label=Direct%20Downloads)
![](https://img.shields.io/github/downloads/RaidcoreGG/NexusInstaller/total?style=for-the-badge&labelColor=%23131519&color=%230F79AA&label=Installer%20Downloads)

# Nexus - An addon engine for Guild Wars 2

*Host, Loader, Manager, Framework* and *Platform* are all terms describing what Nexus does.

### For Users
Nexus means **easy setup**, **updating** and **management**. Maximum **convenience**. **Directly in the game.** No fiddling with any DLL files.

### For Developers
Nexus means focus on developing actual features. **Hot-Loading. Easy Integration. Powerful API.**

## Features
- **Hot-Loading**
- **Automatic Updates**
- **Event PubSub**
- **Managed Keybinds**
- **Logging**
- **Resource Registry** to share resources & functions between Addons.
- **Texture/Image Loader**
- **Addon Library** for quick discovery & installation.
- **Auto-Disable on Game Update**
- **Extended Real-Time APIs**
- **ArcDPS Integration**

## Installation
1. Download `d3d11.dll` found in the [latest release](https://github.com/RaidcoreGG/Nexus/releases).
2. Place the file in your game installation directory (e.g. `C:\Program Files\Guild Wars 2`). In that same folder you should see `Gw2-64.exe`.
3. Start the game, you should have a popup to accept the Legal Agreement.

After accepting you can open the menu using `CTRL+O` by default.

### Chainloading
If you want another proxy d3d11.dll, for example ArcDPS, you can chainload it by naming it `d3d11_chainload.dll` and also putting it in the game directory.

## [Third Party Notices](https://github.com/RaidcoreGG/Nexus/blob/main/THIRDPARTYSOFTWAREREADME.TXT)

## Special Thanks
### GGDM
For the idea of a proxy dll & hot-loading.

### [Deltaconnected / ArcDPS](https://www.deltaconnected.com/arcdps/)
For the idea of an addon loading system.

### [Thomas McBoyle](https://github.com/TMcBoyle)
For general guidance & programming help.

### [Jakub Vitek](https://github.com/Sognus)
Invaluable help with debugging & general guidance.

### [ArenaNet](https://arena.net/)
For creating my favourite game!

### The Guild Wars 2 Community
For supporting my work!
