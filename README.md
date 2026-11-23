# itel i2163 — fpdoom Sideload Guide

> Complete walkthrough for running Doom, Wolfenstein 3D, Retris, and more on the **itel i2163** feature phone (Spreadtrum SC6531E) from a **CachyOS / Arch Linux** host.

# the games are in releases haha. get em from there, alternatively visit https://github.com/amorphousneon/fp_wads_i2163
# password of games.7z in releases: ``` itel-i2163-gamesideload ```

![Phone](https://img.shields.io/badge/Phone-itel%20i2163-blue)
![Chip](https://img.shields.io/badge/Chipset-SC6531E-green)
![LCD](https://img.shields.io/badge/LCD-128%C3%97160%20GC9106-orange)
![OS](https://img.shields.io/badge/Host-CachyOS%20%2F%20Arch-1793d1)
![Status](https://img.shields.io/badge/Status-Working-brightgreen)

---

## Table of Contents

1. [What This Is](#what-this-is)
2. [Phone Compatibility Check](#phone-compatibility-check)
3. [Hardware Specs](#hardware-specs)
4. [Prerequisites (CachyOS)](#prerequisites-cachyos)
5. [Phase 1 — Build the USB Tools](#phase-1--build-the-usb-tools)
6. [Phase 2 — Dump the Firmware](#phase-2--dump-the-firmware)
7. [Phase 3 — Scan the Firmware](#phase-3--scan-the-firmware)
8. [Phase 4 — Get the Prebuilt Game Package](#phase-4--get-the-prebuilt-game-package)
9. [Phase 5 — Prepare the SD Card](#phase-5--prepare-the-sd-card)
10. [Phase 6 — Install sdboot (Permanent)](#phase-6--install-sdboot-permanent)
11. [Phase 7 — Get Game Data Files](#phase-7--get-game-data-files)
12. [Phase 8 — Build Games from Source (Optional)](#phase-8--build-games-from-source-optional)
13. [Phase 9 — Boot and Play](#phase-9--boot-and-play)
14. [In-Game Controls](#in-game-controls)
15. [Troubleshooting](#troubleshooting)
16. [Recovery and Uninstall](#recovery-and-uninstall)
17. [Brick Risk Summary](#brick-risk-summary)
18. [Credits](#credits)
19. [License](#license)

---

## What This Is

This repository is a **phone-specific guide** for sideloading and running games on the **itel i2163** keypad feature phone using the [fpdoom](https://github.com/ilyakurdyukov/fpdoom) framework by Ilya Kurdyukov. It documents a complete, tested workflow from a factory-fresh phone to playing Doom on battery power — no USB cable needed after installation.

The itel i2163 runs bare-metal firmware on a Spreadtrum SC6531E chipset (ARM9EJ-S, 208 MHz). It has no Android, no app store, and no official way to run custom code. This guide uses the fpdoom `sdboot` loader to add an SD-card boot capability: hold one key at power-on, and the phone boots your code from a FAT32 micro-SD card instead of its internal firmware.

**What works on this phone (tested):**
- ✅ Doom 1 (shareware)
- ✅ Retris (Tetris clone)
- ✅ The fpmenu game launcher
- ⚠️ Wolfenstein 3D (works with correct build flags + data files)
- ⚠️ NES / GameBoy / SNES emulators (need ROM files)

**What may not work (4 MB RAM limit):**
- ❌ Build engine games (Duke 3D, Blood, Shadow Warrior) — too RAM-heavy

---

## Phone Compatibility Check

### CRITICAL: i2163 vs i2163S

Before starting, confirm you have the right phone. The **itel i2163** and **itel i2163S** look identical but use completely different chipsets:

| Model | Chipset | Supported? |
|-------|---------|------------|
| itel i2163 | Spreadtrum SC6531E | ✅ Yes — this guide |
| itel i2163A | Spreadtrum SC6531E | ✅ Yes — this guide |
| itel i2163S | MediaTek MT6261D | ❌ No — different chip, won't work |

**How to check:** Look at the label under the battery, or the phone's spec sheet. If it says Spreadtrum, Unisoc, or SC6531E, you're on the right path.

### Confirm with fphelper

After Phase 3, the `fphelper scan` output will print your chip ID. You should see:

```
0x20: found SC6531E firmware marker
```

and the CHIP ID reported by spd_dump during the dump:

```
BSL_REP_VER: "Custom FDL1: CHIP ID = 0x65620001"
```

`0x65620001` = SC6531E. If you see a different chip ID, stop — this guide won't apply.

---

## Hardware Specs

| Component | Value |
|-----------|-------|
| Phone | itel i2163 / i2163A |
| Chipset | Spreadtrum SC6531E |
| CPU | ARM9EJ-S, single-core, 208 MHz, ARMv5TE, Thumb |
| RAM | 4 MB (32 Mbit) external SDRAM |
| Flash | 4 MB (32 Mbit) NOR SPI flash |
| LCD | 128×160, RGB565, GC9106 controller, SPI1 |
| SD card slot | Yes (micro-SD) |
| USB | micro-USB (data + power) |
| Boot key (USB download) | Left Soft (LSOFT, 0x08) |
| SD boot keys | Up (0x04) OR Left (0x06) OR Center (0x0d) — hold ONE, not all |

---

## Prerequisites (CachyOS)

### System packages

```sh
sudo pacman -Syu
sudo pacman -S base-devel libusb p7zip git curl
```

| Package | Purpose |
|---------|---------|
| `base-devel` | gcc, make, build tools |
| `libusb` | USB communication library for spd_dump |
| `p7zip` | Extract the `.7z` prebuilt package |
| `git` | Clone source repos |
| `curl` | Download prebuilt binaries |

### ARM cross-compiler (for building games from source — optional)

If you only want to use prebuilt binaries (recommended), skip this. If you want to build games from source:

**Option A — Bootlin prebuilt toolchain (recommended, no root):**
```sh
mkdir -p ~/toolchain && cd ~/toolchain
curl -L -o armv5.tar.xz "https://toolchains.bootlin.com/downloads/releases/toolchains/armv5-eabi/tarballs/armv5-eabi--glibc--stable-2024.05-1.tar.xz"
tar xf armv5.tar.xz
TC=~/toolchain/armv5-eabi--glibc--stable-2024.05-1/bin/arm-buildroot-linux-gnueabi
```

**Option B — Arch repo bare-metal compiler:**
```sh
sudo pacman -S arm-none-eabi-gcc arm-none-eabi-binutils
TC=/usr/bin/arm-none-eabi
```

### udev rule (run spd_dump without sudo)

```sh
echo 'SUBSYSTEMS=="usb", ATTRS{idVendor}=="1782", ATTRS{idProduct}=="4d00", MODE="0666", TAG+="uaccess"' | sudo tee /etc/udev/rules.d/80-spd-mtk.rules
sudo udevadm control --reload-rules
sudo udevadm trigger
```

After this, re-plug the phone and you can run `spd_dump` without `sudo`.

### What else you need

| Item | Why |
|------|-----|
| USB **data** cable (micro-USB) | Connect phone to PC — must carry data, not charge-only |
| micro-SD card (any size, ≥ 128 MB) | Holds the game binaries + data files |
| Patience | The first dump + flash takes ~5 minutes |

---

## Phase 1 — Build the USB Tools

`spd_dump` is not in any Linux package repo — it must be built from source. It's the tool that talks to the phone's boot ROM over USB.

### Clone and build

```sh
mkdir -p ~/Documents/itel-i2163-sideload
cd ~/Documents/itel-i2163-sideload

git clone https://github.com/ilyakurdyukov/spreadtrum_flash.git
cd spreadtrum_flash
make
```

This produces:
- `spd_dump` — the USB tool (dump, write, RAM-load)
- `fphelper/fphelper` — firmware scanner and sdboot patcher (build separately, see below)

### Build fphelper

```sh
cd fphelper
make
cd ..
```

`fphelper` is a host-side utility that scans your firmware dump to find boot keys and generates the sdboot install commands.

### Get nor_fdl1.bin

`nor_fdl1.bin` is the custom download-mode loader that spd_dump sends to the phone. Two options:

**Option A — download prebuilt (fastest):**
Download from the [spreadtrum_flash Releases page](https://github.com/ilyakurdyukov/spreadtrum_flash/releases) and extract `nor_fdl1.bin`.

**Option B — build from source (needs ARM cross-compiler):**
```sh
cd custom_fdl
make TOOLCHAIN=$TC
# produces custom_fdl/nor_fdl1.bin
cd ..
```

### Verify

```sh
ls -la spd_dump fphelper/fphelper nor_fdl1.bin
```

All three must exist and be executable.

---

## Phase 2 — Dump the Firmware

This creates your **safety net** — a complete backup of the phone's firmware. Do this before any flashing.

### Enter boot mode

1. Remove the battery
2. Wait 5–10 seconds (phone fully powers down)
3. Hold the **Left Soft key** (LSOFT — top-left button under the screen)
4. While holding it, plug in the USB cable
5. Keep holding for 2–3 seconds, then release
6. The screen stays off — the phone is now in boot mode

### Verify the phone is detected

```sh
lsusb | grep 1782
```

You should see:
```
Bus 001 Device 004: ID 1782:4d00 Spreadtrum Communications Inc.
```

If you don't see it, repeat the boot-mode procedure or try a different cable.

### Dump the firmware

```sh
cd ~/Documents/itel-i2163-sideload/spreadtrum_flash
sudo ./spd_dump fdl nor_fdl1.bin 0x40004000 read_flash 0x80000003 0 16M flash.bin
```

Expected output:
```
BSL_REP_VER: "SPRD3\0"
BSL_REP_VER: "Custom FDL1: CHIP ID = 0x65620001\0"
dump_flash: 0x80000003+0x0, target: 0x1000000, read: 0x1000000
```

- `SPRD3` — phone answered in Spreadtrum BootROM protocol ✓
- `CHIP ID = 0x65620001` — confirms SC6531E ✓
- `read: 0x1000000` — 16 MiB dumped ✓

### Verify the dump

```sh
ls -l flash.bin
stat -c '%s' flash.bin
```

Must be **exactly 16777216 bytes** (16 MiB). The phone's actual flash is 4 MiB, so the dump contains the firmware repeated 4× — that's normal; fphelper handles it.

### BACK UP THE DUMP

```sh
cp flash.bin flash.bin.backup
cp flash.bin ~/Documents/firmware-backups/itel-i2163-$(date +%Y%m%d).bin
```

**This file is your recovery image.** If anything ever goes wrong, flashing this file back restores the phone to its exact original state. Do not lose it.

---

## Phase 3 — Scan the Firmware

```sh
cd ~/Documents/itel-i2163-sideload/spreadtrum_flash/fphelper
./fphelper ../flash.bin scan
```

### What to look for

The key line:
```
0x405429c: keymap, rows = 8, bootkey = 0x08 (LSOFT), sdboot keys = 0x04 0x06 0x0d (UP, LEFT, CENTER)
```

| Value | Meaning |
|-------|---------|
| `bootkey = 0x08 (LSOFT)` | Hold Left Soft + plug USB to enter boot mode |
| `sdboot keys = 0x04 0x06 0x0d` | These are **three alternative single-key triggers**, NOT a combo. Hold **ONE** of Up / Left / Center at power-on to boot from SD |

Other useful info from the scan:
- `cs0_size = 0x400000` — confirms 4 MiB flash
- `guess: RAM size = 4MB` — confirms 4 MB RAM
- `guess: with SD card slot` — confirms SD boot is possible

### Generate the sdboot install commands

```sh
./fphelper ../flash.bin sdboot
```

This prints three command blocks **tailored to your firmware**:

```
sdboot: entry = 0x549d0, chip = 1
sdboot: jump_buf = 0x3014, end = 0x36f150
sdboot: sdboot = 0x370000

# sdboot test run:
./spd_dump fdl sdboot1.bin 0x40004000

# sdboot install/update:
./spd_dump fdl nor_fdl1.bin 0x40004000 \
  write_word fw+0x24 0x370000 \
  erase_flash fw+0x370000 4K \
  write_data fw+0x370000 0 0 sdboot1.bin \
  write_word fw+0x370004 0x549d0

# sdboot remove:
./spd_dump fdl nor_fdl1.bin 0x40004000 \
  write_word fw+0x24 0x549d0 \
  erase_flash fw+0x370000 4K
```

**Save this output.** The exact addresses (`0x370000`, `0x549d0`, etc.) are specific to your firmware revision. The values above are from a tested itel i2163 — yours may differ slightly.

---

## Phase 4 — Get the Prebuilt Game Package

The fpdoom Releases include a prebuilt package with all game binaries + sdboot, already compiled for SC6531E.

### Download and extract

```sh
cd ~/Documents/itel-i2163-sideload
curl -L -o prebuilt.7z https://github.com/ilyakurdyukov/fpdoom/releases/latest/download/prebuilt.7z
7z x prebuilt.7z -ofpdoom_prebuilt
```

### What's inside

```
fpdoom_prebuilt/
├── sdboot1.bin          ← sdboot for SC6531E (chip = 1)
├── sdboot2.bin          ← for SC6531DA (not your phone)
├── sdboot3.bin          ← for SC6530 (not your phone)
├── sdboot_t117.bin      ← for UMS9117 4G phones (not your phone)
├── jump4m.bin
├── sdcard/
│   └── fpbin/
│       ├── fpmain.bin           ← fpmenu launcher (this is what loads first)
│       ├── config.txt           ← menu configuration
│       ├── fpdoom.bin           ← Doom engine
│       ├── wolf3d.bin           ← Wolfenstein 3D
│       ├── wolf3d_sw.bin        ← Wolf3D shareware variant
│       ├── retris.bin           ← Tetris clone
│       ├── infones.bin          ← NES emulator
│       ├── gnuboy.bin           ← GameBoy emulator
│       ├── snes9x.bin           ← SNES emulator
│       ├── snes9x_16bit.bin     ← SNES (16-bit color variant)
│       ├── fpduke3d.bin         ← Duke Nukem 3D (may not fit in 4 MB RAM)
│       ├── fpsw.bin             ← Shadow Warrior (may not fit)
│       ├── fpblood.bin          ← Blood (may not fit)
│       ├── chocolate-doom.bin   ← Chocolate Doom (Heretic/Hexen engine)
│       ├── chocolate-heretic.bin
│       └── chocolate-hexen.bin
├── usb/                 ← binaries for USB-mode (LIBC_SDIO=0), not for SD boot
└── usb_t117/
```

**You only need `sdboot1.bin` and `sdcard/fpbin/`.** The `usb/` directory contains binaries compiled for USB file I/O (needs libc_server running on PC) — do NOT use those for SD card boot.

---

## Phase 5 — Prepare the SD Card

### Identify the SD card

```sh
lsblk
```

Insert the card, run `lsblk` again, and identify the new device (e.g. `/dev/sdb`). **Double-check — picking the wrong device will wipe a disk.**

### Format as FAT32

sdboot only supports FAT32 (not FAT16, not exFAT):

```sh
# replace /dev/sdX with your SD card device
sudo umount /dev/sdX* 2>/dev/null
sudo sgdisk --zap-all /dev/sdX
sudo parted -s /dev/sdX mklabel msdos
sudo parted -s /dev/sdX mkpart primary fat32 1MiB 100%
sudo mkfs.vfat -F 32 -n ITELBOOT /dev/sdX1
```

If `sgdisk` or `parted` are missing: `sudo pacman -S gptfdisk parted`.

### Copy the game files

```sh
sudo mkdir -p /mnt/itelboot
sudo mount /dev/sdX1 /mnt/itelboot

# copy the entire fpbin directory (preserves directory structure)
sudo cp -r ~/Documents/itel-i2163-sideload/fpdoom_prebuilt/sdcard/fpbin /mnt/itelboot/

# create the games directory structure
sudo mkdir -p /mnt/itelboot/games/{doom1,wolf3d,retris,nes,gameboy,snes}

# verify
ls /mnt/itelboot/fpbin/
ls /mnt/itelboot/games/

sudo umount /mnt/itelboot
```

### Expected SD card layout

```
<SD card root>/
├── fpbin/
│   ├── fpmain.bin          ← fpmenu launcher (loaded by sdboot)
│   ├── config.txt          ← menu items + arguments
│   ├── fpdoom.bin
│   ├── wolf3d.bin
│   ├── retris.bin
│   ├── infones.bin
│   ├── gnuboy.bin
│   ├── snes9x.bin
│   └── ... (other game binaries)
└── games/
    ├── doom1/
    │   └── doom1.wad       ← YOU ADD THIS (Phase 7)
    ├── wolf3d/
    │   └── *.wl1           ← YOU ADD THIS (Phase 7)
    ├── retris/             ← may be empty (Retris is self-contained)
    ├── nes/
    │   └── *.nes           ← YOU ADD ROMs
    ├── gameboy/
    │   └── *.gbc           ← YOU ADD ROMs
    └── snes/
        └── *.smc           ← YOU ADD ROMs
```

---

## Phase 6 — Install sdboot (Permanent)

This writes `sdboot1.bin` to a free space region in your phone's flash and patches the boot pointer so holding an SD-boot key at power-on launches sdboot instead of the normal firmware.

### The install command

Use the exact commands from your `fphelper flash.bin sdboot` output (Phase 3). For the tested itel i2163:

```sh
cd ~/Documents/itel-i2163-sideload/spreadtrum_flash
sudo ./spd_dump fdl nor_fdl1.bin 0x40004000 \
  write_word fw+0x24 0x370000 \
  erase_flash fw+0x370000 4K \
  write_data fw+0x370000 0 0 ../fpdoom_prebuilt/sdboot1.bin \
  write_word fw+0x370004 0x549d0
```

### What each line does

| Command | Effect |
|---------|--------|
| `write_word fw+0x24 0x370000` | Patches boot pointer: SD-boot key → jump to 0x370000 |
| `erase_flash fw+0x370000 4K` | Erases the 4 KiB sector at 0x370000 (free space after firmware end 0x36f150) |
| `write_data fw+0x370000 0 0 sdboot1.bin` | Writes sdboot1.bin to that sector |
| `write_word fw+0x370004 0x549d0` | Stores original entry point (0x549d0) so sdboot can chain to normal boot |

### Procedure

1. Phone powered off (battery out, wait 5s, reinsert)
2. Hold **LSOFT** + plug in USB
3. Run the install command above
4. Wait for all four operations to report success
5. Unplug USB

**Do not unplug the USB cable while the install is running.** The only risk window is the ~2 seconds during `write_word fw+0x24`. If interrupted, reflash your `flash.bin` backup (see Recovery).

### Verify the install

After installing, unplug the phone, then:
1. Insert the SD card (with `fpbin/` on it)
2. Hold **Center** + power on
3. If the fpmenu appears → sdboot is installed correctly ✓

---

## Phase 7 — Get Game Data Files

The prebuilt package includes game **binaries** but not game **data** (copyright reasons). You must obtain these separately. **All filenames must be lowercase on the SD card.**

### Doom 1 (free shareware)

The Doom shareware WAD (v1.9, ~4.2 MB) is freely distributable:

```sh
cd ~/Documents/itel-i2163-sideload
curl -L -o doom19.zip https://www.doomworld.com/idgames/idstuff/doom/doom19.zip
unzip doom19.zip
find . -iname "doom1.wad"
```

Copy to the SD card:
```sh
sudo mount /dev/sdX1 /mnt/itelboot
sudo cp DOOMSW/DOOM1.WAD /mnt/itelboot/games/doom1/doom1.wad
# ensure lowercase:
sudo mv /mnt/itelboot/games/doom1/DOOM1.WAD /mnt/itelboot/games/doom1/doom1.wad 2>/dev/null
ls -l /mnt/itelboot/games/doom1/
sudo umount /mnt/itelboot
```

### Wolfenstein 3D (shareware v1.4)

Needs these files in `games/wolf3d/` (all lowercase):
- `maphead.wl1`
- `gamemaps.wl1`
- `audiohed.wl1`
- `audiot.wl1`
- `vswap.wl1`

Search for "Wolfenstein 3D shareware 1.4 download". The installer is a DOS .exe that's often a self-extracting ZIP:
```sh
unzip wolf3d_sw.exe -d wolf3d_extracted
cd wolf3d_extracted
for f in *.WL1; do mv "$f" "$(echo $f | tr A-Z a-z)"; done
```

Copy to SD card:
```sh
sudo mount /dev/sdX1 /mnt/itelboot
sudo cp ~/Documents/itel-i2163-sideload/wolf3d_extracted/*.wl1 /mnt/itelboot/games/wolf3d/
sudo umount /mnt/itelboot
```

### Retris (self-contained — no data files needed)

Retris is a homebrew Tetris port with no external data files. It works immediately with just `fpbin/retris.bin`. The `games/retris/` directory can be empty.

### NES / GameBoy / SNES ROMs

You provide your own legally-obtained ROM files:
- NES: `games/nes/*.nes`
- GameBoy: `games/gameboy/*.gb` or `*.gbc`
- SNES: `games/snes/*.smc`

You may need to edit `fpbin/config.txt` on the SD card to point to your actual ROM filenames.

---

## Phase 8 — Build Games from Source (Optional)

Skip this if you're using the prebuilt binaries. Only needed if you want to modify a game or the prebuilt binary doesn't work.

### The critical flag: LIBC_SDIO=3

From the fpdoom build system (`build_sc6531.make`):
```makefile
LIBC_SDIO ?= 0    # defaults to USB mode!
```

| Value | Mode | File I/O via |
|-------|------|-------------|
| `0` | USB mode (default) | libc_server on PC (needs USB cable) |
| `3` | SD card mode | FAT32 filesystem on SD card |

**For SD card boot, you MUST build with `LIBC_SDIO=3`.** Without it, the game tries to read data files via USB and crashes (black screen → reboot) when libc_server isn't there.

### Build Retris (self-contained, works even with LIBC_SDIO=0)

```sh
cd ~/Documents/itel-i2163-sideload/fpdoom
git clone https://github.com/ilyakurdyukov/fpdoom.git .  # if not already cloned
cd retris
make -f helper.make all patch    # download + patch upstream retris source
make LIBC_SDIO=3 TOOLCHAIN=$TC
# produces retris.bin
```

### Build Wolfenstein 3D

```sh
cd ~/Documents/itel-i2163-sideload/fpdoom/wolf3d
make -f helper.make all patch
make LIBC_SDIO=3 TOOLCHAIN=$TC
# produces wolf3d.bin
```

### Build Doom (from source)

```sh
cd ~/Documents/itel-i2163-sideload/fpdoom
git clone https://github.com/id-Software/DOOM.git doom_src_tmp
cp -r doom_src_tmp/linuxdoom-1.10 doom_src
patch -d doom_src -p1 -i ../doom.patch
cd fpdoom
make LIBC_SDIO=3 TOOLCHAIN=$TC
# produces fpdoom.bin
```

### Copy rebuilt binaries to SD card

```sh
sudo mount /dev/sdX1 /mnt/itelboot
sudo cp wolf3d.bin /mnt/itelboot/fpbin/wolf3d.bin
sudo cp retris.bin /mnt/itelboot/fpbin/retris.bin
sudo cp fpdoom.bin /mnt/itelboot/fpbin/fpdoom.bin
sudo umount /mnt/itelboot
```

---

## Phase 9 — Boot and Play

### Boot from SD card (battery only, no USB needed)

1. SD card in phone (with `fpbin/` + game data)
2. Phone powered off (hold red End Call button to shut down, or remove battery 5s and reinsert)
3. Hold **Center** (or **Up**, or **Left** — any one works)
4. While holding, press and hold the **Power button** (red End Call key) for 2–3 seconds until the screen lights up
5. Keep holding **Center** for ~1 more second, then release
6. The fpmenu appears

### Navigate the menu

| Key | Action |
|-----|--------|
| Up / Down | Move selection |
| Center | Select / launch |
| Right Soft | Back / exit menu |

### Selecting a game

When you select a game (e.g. "Doom 1"), fpmenu:
1. Reads the corresponding `.bin` from `fpbin/` on the SD card
2. Loads it into RAM at 0x14000000
3. Jumps to it

The game then reads its data files from `games/<name>/` on the SD card and starts.

### Important notes

- **Hold only ONE SD-boot key** (Center, Up, or Left). Holding all three triggers USB boot mode due to keypad matrix limitations.
- **If it boots into the normal itel RTOS** instead of the menu, the timing was off — try holding Center slightly longer after the screen lights up.
- **The `--charge 2` in config.txt** allows running on USB power without a battery, but does NOT prevent battery operation. Your phone runs fine on battery.
- **To power off:** hold the red End Call button for ~5 seconds, or remove the battery.

---

## In-Game Controls

These are the default controls for Doom / Wolf3D / similar games (from the fpdoom README):

| Key | Action |
|-----|--------|
| Left Soft | Use (open doors, activate) |
| Right Soft | Menu |
| D-pad (Up/Down/Left/Right) | Movement |
| Center | Fire |
| Dial (green call key) | Run |
| 2 / 4 / 5 / 6 | Up / Left / Down / Right (alternate movement) |
| 1 / 3 | Strafe left / right |
| 7 / 9 | Previous / next weapon |

For Retris (Tetris):
| Key | Action |
|-----|--------|
| Left / Right | Move piece |
| Up / 2 | Rotate |
| Down | Soft drop |
| Center | Hard drop |

---

## Troubleshooting

### `sudo: spd_dump: command not found`

spd_dump isn't in any package repo. Build it from source (Phase 1), or use `./spd_dump` from the build directory.

### `lsusb` shows no Spreadtrum device

Phone not in boot mode. Remove battery, wait 10 seconds, hold LSOFT firmly, then plug USB. Try a data cable (some are charge-only).

### `usb_send failed: LIBUSB_ERROR_TIMEOUT` after `BSL_REP_VER`

**This is normal during the sdboot test run.** It means spd_dump successfully sent sdboot1.bin, sdboot took over USB, and spd_dump's poll timed out because sdboot isn't in FDL mode anymore. The phone continues running sdboot. Ignore this message.

### Screen stays black after Method B (USB RAM load)

The entry code's USB handshake may be blocking. Run `libc_server` in a second terminal:
```sh
cd ~/Documents/itel-i2163-sideload/fpdoom
make -C libc_server
./libc_server
```
Then retry the `spd_dump fdl` command.

### Screen stays black after selecting a game from the menu

The game binary is trying to read data files and can't find them. Check:
1. Is the data file in the right directory? (`games/doom1/doom1.wad`, etc.)
2. Is the filename **lowercase**? (`doom1.wad`, not `DOOM1.WAD`)
3. Is the game binary built with `LIBC_SDIO=3`? (prebuilt `sdcard/fpbin/` binaries are; `usb/` binaries are NOT)

### Game loads then reboots (Wolf3D, Doom)

Likely built with `LIBC_SDIO=0` (USB mode) instead of `LIBC_SDIO=3` (SD card mode). Rebuild with `LIBC_SDIO=3` (Phase 8).

### Build error: `cc start.s -c` fails with ARM instruction errors

The Makefile defaulted to host `cc` instead of the ARM cross-compiler. Pass the toolchain explicitly:
```sh
make LIBC_SDIO=3 TOOLCHAIN=$TC
```

### Booting with Center held still boots into normal RTOS

sdboot may not be installed (Phase 6), or the timing is off. Try:
1. Hold Center **before** pressing power, hold it **longer** after screen lights up
2. Try **Up** or **Left** instead of Center
3. Verify sdboot was installed (re-run the install command)

### `fphelper` says "not an executable file"

fphelper is in a subdirectory with its own Makefile. Build it:
```sh
cd spreadtrum_flash/fphelper
make
./fphelper ../flash.bin scan
```

---

## Recovery and Uninstall

### Remove sdboot (restore original boot behavior)

Use the `sdboot remove` command from your `fphelper flash.bin sdboot` output:

```sh
cd ~/Documents/itel-i2163-sideload/spreadtrum_flash
sudo ./spd_dump fdl nor_fdl1.bin 0x40004000 \
  write_word fw+0x24 0x549d0 \
  erase_flash fw+0x370000 4K
```

This restores the original boot pointer and erases the sdboot slot. The phone boots normally afterward.

### Full firmware restore (nuclear option)

If anything goes wrong and the above doesn't work, reflash your entire `flash.bin` backup:

```sh
cd ~/Documents/itel-i2163-sideload/spreadtrum_flash
sudo ./spd_dump fdl nor_fdl1.bin 0x40004000 \
  fdl nor_fdl.bin 0x14000000 \
  write_flash 0x80000003 0 0x400000 flash.bin
```

Only the first 4 MiB of flash.bin is meaningful (your flash is 4 MiB). This completely restores the phone to its factory state.

---

## Brick Risk Summary

| Step | Risk | Why |
|------|------|-----|
| Build tools | None | Host-only compilation |
| Dump firmware (read) | **Zero** | Read-only, no flash writes |
| Test run (RAM load) | **Zero** | RAM-only, reverts on power off |
| Install sdboot | **Low** | Writes to dedicated empty flash space (0x370000, after firmware end 0x36f150). Boot pointer is a single 4-byte write. Recovery is one command. |
| Boot from SD | **Zero** | Read-only from SD, no flash changes |
| Remove sdboot | **Zero** | Just undoes the install |

**The phone is never truly bricked.** The SC6531E's boot ROM lives in mask ROM (read-only silicon) — it can't be corrupted. Even if the NOR flash is fully zeroed, holding LSOFT + plugging USB enters download mode, and spd_dump can write a fresh firmware. The only permanent damage would be physical (broken USB port, dead chip).

---

## Credits

This guide and repository would not exist without the work of:

- **Ilya Kurdyukov** — author of [fpdoom](https://github.com/ilyakurdyukov/fpdoom), [spreadtrum_flash](https://github.com/ilyakurdyukov/spreadtrum_flash), the sdboot loader, the syscode bare-metal driver library, and the supported-phones table that identified the itel i2163A as compatible. All credit for the technical framework goes to him.
- **id Software** — original Doom source code release ([github.com/id-Software/DOOM](https://github.com/id-Software/DOOM))
- **KS-Presto** — Wolf4SDL port ([github.com/KS-Presto/Wolf4SDL](https://github.com/KS-Presto/Wolf4SDL))
- **The retris author** — homebrew Tetris port

See [CREDITS.md](CREDITS.md) for details.

---

## License

This guide is released under the [GPLv2 LICENSE](LICENSE). The upstream fpdoom and spreadtrum_flash projects are under the Unlicense. Game data files (WADs, ROMs) retain their original copyrights and are NOT (haha they are) included in this repository.

---

*Tested on itel i2163 (SC6531E) with CachyOS. Last verified: November 2026.*

# HELP TAKEN FROM GLM-5.2 HIGH
