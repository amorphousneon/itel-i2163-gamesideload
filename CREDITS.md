# Credits

## Original Author — This Project Wouldn't Exist Without Him

### Ilya Kurdyukov

All credit for the technical framework this guide depends on goes to **Ilya Kurdyukov**. He wrote, maintains, and released the following projects that make running native code on Spreadtrum feature phones possible:

| Project | Repository | Role |
|---------|-----------|------|
| **fpdoom** | [github.com/ilyakurdyukov/fpdoom](https://github.com/ilyakurdyukov/fpdoom) | The Doom port + the **syscode** bare-metal driver library (MMU, LCD, keypad, timers, SDIO), the **sdboot** SD-card boot loader, the **fpmenu** game launcher, the **pack_reloc** tool, and the entire build system. Also maintains the supported-phones table. |
| **spreadtrum_flash** | [github.com/ilyakurdyukov/spreadtrum_flash](https://github.com/ilyakurdyukov/spreadtrum_flash) | The **spd_dump** USB tool (firmware read/write, RAM-load), **fphelper** (firmware scanner + sdboot patcher), and **custom_fdl** (the `nor_fdl1.bin` download-mode loader). |

His work is released under the **Unlicense** (public domain). If you find this workflow useful, star his repos and consider [sponsoring him on GitHub](https://github.com/sponsors/ilyakurdyukov).

---

## Game Source / Data Credits

| Game | Original Source | License / Status |
|------|----------------|-----------------|
| Doom | [id-Software/DOOM](https://github.com/id-Software/DOOM) (linuxdoom-1.10) | GPL-2.0 (source). Shareware `doom1.wad` is freely distributable. |
| Wolfenstein 3D | [KS-Presto/Wolf4SDL](https://github.com/KS-Presto/Wolf4SDL) | GPL-2.0 (port). Game data files are copyrighted by id Software / Bethesda; shareware v1.4 data is freely distributable. |
| Retris | [ilyakurdyukov/retris](https://github.com/ilyakurdyukov/retris) | Homebrew Tetris clone, self-contained. |
| Duke 3D / Blood / Shadow Warrior | [fpbuild](https://github.com/ilyakurdyukov/fpdoom/tree/master/fpbuild) | Build engine ports. May not fit in the itel i2163's 4 MB RAM. |
| infones | [ilyakurdyukov/fpdoom](https://github.com/ilyakurdyukov/fpdoom) | NES emulator port. |
| gnuboy | [ilyakurdyukov/fpdoom](https://github.com/ilyakurdyukov/fpdoom) | GameBoy/GameBoy Color emulator port. |
| snes9x | [ilyakurdyukov/fpdoom](https://github.com/ilyakurdyukov/fpdoom) | SNES emulator port. |

**Game data files (WADs, ROMs) are NOT included in this repository.** They retain their original copyrights. Users must obtain them legally.

---

## This Guide

This guide was written for the **itel i2163 (SC6531E)** specifically, documenting a complete workflow tested on **CachyOS (Arch Linux)**. It was developed through a collaborative session between the repository owner and an AI assistant.

### How to Credit the AI Assistant

If you want to acknowledge the AI tool that helped produce this guide, you have a few options:

#### Option 1 — Acknowledgment in the README (already done)

The credits section of this README already credits Ilya Kurdyukov as the original author. You can add a line like:

```markdown
This guide was produced with assistance from [Z.ai](https://z.ai) (GLM-5.2).
```

#### Option 2 — Co-author trailer in commits

GitHub supports co-author trailers in commit messages. You can add this to commits that were AI-assisted:

```
Co-authored-by: Z.ai <noreply@z.ai>
```

This makes the commits show a co-author in GitHub's UI. To do this with git:

```sh
git commit -m "your commit message

Co-authored-by: Z.ai <noreply@z.ai>"
```

Or configure a commit template:
```sh
git config commit.template .git-commit-template
echo "

Co-authored-by: Z.ai <noreply@z.ai>" > .git-commit-template
```

#### Option 3 — Contributors section in CREDITS.md

You can list contributors here:

```markdown
## Contributors

- **amorphn** — guide author, tested on itel i2163, documented the full workflow
- **Z.ai (GLM-5.2)** — AI assistant that helped research, write, and format this guide
```

### Important Note on AI Contributors

GitHub's contributor graph counts commits by author email. An AI doesn't have a GitHub account or email, so:

- **Co-authored-by trailers** will show in individual commits but won't add to the contributors list on the repo page (unless you use a real email)
- The most transparent approach is an acknowledgment line in the README/CREDITS — it's honest about the role AI played without implying a GitHub account

Choose whatever feels right to you. The key credit goes to Ilya Kurdyukov — without his work, none of this is possible.
