# mod-emblem-transfer

> ======================================================================
> 
> **Important:**
> This is the first version of the module (old). If you want to work with the newest version, please visit AzerothCore's repo [https://github.com/azerothcore/mod-emblem-transfer](https://github.com/azerothcore/mod-emblem-transfer) where is currently being maintained.
> 
> ======================================================================

## Description

Allows a player to transfer emblems (frost, triump, conquest) from one character to another **within the same account**. A configurable penalty can be applied (default: 10%) to the transfered amount.


## How to use ingame

1. Walk to the npc and interact with it.
2. Select the type of emblem you want to transfer.
3. Select the character you want to transfer to.
4. Enter the amount of emblems.
5. Profit!

## Requirements

- AzerothCore v1.0.1+


## Installation

```
1) Simply place the module under the `modules` directory of your AzerothCore source. 
2) Import the SQL manually to the right Database (auth, world or characters).
3) Re-run cmake and launch a clean build of AzerothCore.
```

## Edit module configuration (optional)

If you need to change the module configuration, go to your server configuration folder (where your `worldserver` or `worldserver.exe` is), copy `emblem_transfer.conf.dist` to `emblem_transfer.conf` and edit that new file.


## Credits

- [ZeegCL](https://github.com/ZeegCL) (author)
- BarbzYHOOL and Talamortis (best guys)
- AzerothCore: [repository](https://github.com/azerothcore) - [website](http://azerothcore.org/) - [discord chat community](https://discord.gg/PaqQRkd)
