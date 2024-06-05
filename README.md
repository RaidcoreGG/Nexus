[![](https://discordapp.com/api/guilds/410828272679518241/widget.png?style=banner2)](https://discord.gg/Mvk7W7gjE4)
[![](https://raidcore.gg/Resources/Images/Patreon.png)](https://www.patreon.com/bePatron?u=46163080)

![](https://img.shields.io/github/license/RaidcoreGG/Nexus?style=for-the-badge&labelColor=%23131519&color=%230F79AA)
![](https://img.shields.io/github/v/release/RaidcoreGG/Nexus?style=for-the-badge&labelColor=%23131519&color=%230F79AA)
![](https://img.shields.io/github/downloads/RaidcoreGG/Nexus/total?style=for-the-badge&labelColor=%23131519&color=%230F79AA)

# Nexus Addon Loader &amp; Manager

Nexus is a next-gen Addon Manager & Loader for Guild Wars 2. It provides a powerful API for addons and keeps your plugins always up-to-date. No fidgeting with DLL files, crashing games after updates or inconsistent behaviour across addons. A single installation to manage all your extensions.
### *This is as close to an official Addon API as it gets.*

## Features
### Overview
- **Loading & unloading** addons **without restarting the game**.
- **Automatic updates**.
- **Addon Library** for quick discovery & installation of new addons.
- **Automatically disable breaking addons** when the game updates.
- **Consistent behaviour across addons**.
- **Multiboxing** compatibile.
- **100% ToS compliant.** Doesn't read game memory at all.

### Overview for addon developers
- **Hot-Loading**, enabling fast feature iteration.
- **Event Publishing / Subscribing**
- **Managed Keybinds** No need to fiddle with WndProc. *Unless you want to*.
- **Logging**
- **Resource Registry** to share resources & functions between Addons.
- **Texture/Image Loader**
- Shared &amp; parsed **[Mumble](https://github.com/RaidcoreGG/RCGG-lib-mumble-api)**
- Shared **local combat log**
- **Integrates with ArcDPS**
- Shared world &amp; **map completion progress**
- Shared **character stats**
- Managed **[GW2 Web API](https://api.guildwars2.com/v2)**

## Installation
1. Download `d3d11.dll` found in the [latest release](https://github.com/RaidcoreGG/Nexus/releases).
2. Place the file in your game installation directory (e.g. `C:\Program Files\Guild Wars 2`). In that same folder you should see `Gw2-64.exe`.
3. Start the game, you should have a popup to accept the Legal Agreement.

After accepting you can open the menu using `CTRL+O` by default.

### Chainloading
If you want another proxy d3d11.dll, for example ArcDPS, you can chainload it by naming it `d3d11_chainload.dll` and also putting it in the game directory.

## Third Party Notices
### [Dear ImGui](https://github.com/ocornut/imgui)
Licensed under the MIT license.

### [cpp-httplib](https://github.com/yhirose/cpp-httplib)
Licensed under the MIT license.

### [nlohmann/json](https://github.com/nlohmann/json)
Licensed under the MIT license.

### [nothings/stb_image](https://github.com/nothings/stb)
Licensed under the MIT license.

### [openssl](https://github.com/openssl/openssl)
Licensed under the Apache-2.0 license.

### [TsudaKageyu/minhook](https://github.com/TsudaKageyu/minhook)
Licensed under the BSD 2-Clause license.

## Special Thanks
### [GGDM](https://nkga.github.io/post/ggdm---combat-analysis-mod-for-guild-wars-2/)
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