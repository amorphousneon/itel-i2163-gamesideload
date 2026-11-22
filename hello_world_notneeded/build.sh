#!/usr/bin/env bash
# build.sh - build hello.bin for the itel i2163 (SC6531E) feature phone.
#
# Usage:
#   ./build.sh                       # auto-detect toolchain
#   ./build.sh /path/to/arm-prefix   # explicit toolchain prefix
#
# The script expects to be run from inside a clone of the fpdoom tree,
# i.e. the hello/ directory sits next to fptest/ and fpmenu/.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# --- locate an ARM cross-compiler --------------------------------------
TC=""
if [[ "${1:-}" != "" ]]; then
    TC="$1"
elif command -v arm-linux-gnueabi-gcc >/dev/null 2>&1; then
    TC="arm-linux-gnueabi"
elif command -v arm-none-eabi-gcc >/dev/null 2>&1; then
    TC="arm-none-eabi"
else
    # fall back to a locally extracted Bootlin toolchain (see README)
    for candidate in \
        "$HOME/toolchain/armv5-eabi--glibc--stable-2024.05-1/bin/arm-buildroot-linux-gnueabi" \
        "$SCRIPT_DIR/../toolchain/armv5-eabi--glibc--stable-2024.05-1/bin/arm-buildroot-linux-gnueabi"; do
        if [[ -x "$candidate-gcc" ]]; then
            TC="$candidate"
            break
        fi
    done
fi

if [[ -z "$TC" ]]; then
    echo "ERROR: no ARM cross-compiler found." >&2
    echo "Install gcc-arm-linux-gnueabi  (Debian/Ubuntu: sudo apt install gcc-arm-linux-gnueabi)" >&2
    echo "or download the Bootlin armv5 toolchain (see README.md, Prerequisites)." >&2
    exit 1
fi

echo "Using toolchain: ${TC}-gcc"
"$TC"-gcc --version | head -1

# --- sanity: this dir must live inside the fpdoom tree -----------------
if [[ ! -f "../buildinc.make" || ! -d "../fpdoom" ]]; then
    echo "ERROR: ../buildinc.make or ../fpdoom/ not found." >&2
    echo "This directory must be placed inside a clone of the fpdoom repo." >&2
    echo "Clone:  git clone https://github.com/ilyakurdyukov/fpdoom.git" >&2
    echo "Then:   cp -r hello/ fpdoom/hello/  &&  cd fpdoom/hello  &&  ./build.sh" >&2
    exit 1
fi

# --- build the pack_reloc host tool if missing -------------------------
if [[ ! -x "../pack_reloc/pack_reloc" ]]; then
    echo "Building pack_reloc host tool..."
    make -C ../pack_reloc
fi

# --- build hello.bin ---------------------------------------------------
echo "Building hello.bin..."
make clean 2>/dev/null || true
make TOOLCHAIN="$TC"

if [[ -f hello.bin ]]; then
    echo
    echo "SUCCESS: $(ls -la hello.bin | awk '{print $5}') bytes -> $(pwd)/hello.bin"
    echo "SHA-256: $(sha256sum hello.bin | awk '{print $1}')"
    echo
    echo "Next: copy hello.bin to your SD card as fpbin/fpmain.bin"
    echo "(see README.md for the full sideload procedure)."
else
    echo "ERROR: hello.bin was not produced." >&2
    exit 1
fi
