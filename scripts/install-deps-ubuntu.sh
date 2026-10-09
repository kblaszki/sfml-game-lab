#!/usr/bin/env bash
# Install Ubuntu build dependencies for sfml-game-lab.
# Prefers the newest packages available from the distro apt repositories
# (no Kitware/LLVM PPAs unless apt is too old — see below).
set -euo pipefail

usage() {
  cat <<'EOF'
Usage: sudo ./scripts/install-deps-ubuntu.sh [--devtools] [-h|--help]

Installs CMake, Ninja, pkg-config, and SFML system -dev libraries from apt
(latest candidates; no version pins).

  --devtools   Also install clang-format, clang-tidy, clang, ccache, gcovr
  -h, --help   Show this help

Requires root (sudo). SFML and GoogleTest are still fetched by CMake; this
script only installs the OS toolchain and system libraries.
EOF
}

DEVTOOLS=0
for arg in "$@"; do
  case "$arg" in
    --devtools) DEVTOOLS=1 ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "Unknown argument: $arg" >&2
      usage >&2
      exit 2
      ;;
  esac
done

if [[ "${EUID}" -ne 0 ]]; then
  echo "Run as root: sudo $0 $*" >&2
  exit 1
fi

if ! command -v apt-get >/dev/null 2>&1; then
  echo "apt-get not found; this script is for Debian/Ubuntu." >&2
  exit 1
fi

export DEBIAN_FRONTEND=noninteractive

REQUIRED_PKGS=(
  cmake
  ninja-build
  pkg-config
  libgl1-mesa-dev
  libudev-dev
  libxrandr-dev
  libxcursor-dev
  libxi-dev
  libx11-dev
  libfreetype6-dev
  libharfbuzz-dev
  libvorbis-dev
  libflac-dev
  libopenal-dev
)

DEVTOOLS_PKGS=(
  clang
  clang-format
  clang-tidy
  ccache
  gcovr
)

apt-get update
apt-get install -y "${REQUIRED_PKGS[@]}"

if [[ "${DEVTOOLS}" -eq 1 ]]; then
  apt-get install -y "${DEVTOOLS_PKGS[@]}"
fi

# Escalate only when distro CMake is older than the project floor.
CMAKE_MIN_MAJOR=3
CMAKE_MIN_MINOR=20
if command -v cmake >/dev/null 2>&1; then
  CMAKE_VER="$(cmake --version | head -n1 | awk '{print $3}')"
  CMAKE_MAJOR="${CMAKE_VER%%.*}"
  CMAKE_REST="${CMAKE_VER#*.}"
  CMAKE_MINOR="${CMAKE_REST%%.*}"
  if (( CMAKE_MAJOR < CMAKE_MIN_MAJOR )) ||
     { (( CMAKE_MAJOR == CMAKE_MIN_MAJOR )) && (( CMAKE_MINOR < CMAKE_MIN_MINOR )); }; then
    cat <<EOF >&2

WARNING: cmake ${CMAKE_VER} is below ${CMAKE_MIN_MAJOR}.${CMAKE_MIN_MINOR}.
This distro apt package is too old for sfml-game-lab. Install a newer CMake
from https://apt.kitware.com/ (or an official Kitware binary), then re-run
\`cmake --version\`.
EOF
    exit 1
  fi
fi

echo
echo "Installed toolchain versions:"
command -v cmake >/dev/null && cmake --version | head -n1
command -v ninja >/dev/null && ninja --version | awk '{print "ninja " $0}'
command -v pkg-config >/dev/null && pkg-config --version | awk '{print "pkg-config " $0}'
command -v g++ >/dev/null && g++ --version | head -n1
if [[ "${DEVTOOLS}" -eq 1 ]]; then
  command -v clang-format >/dev/null && clang-format --version | head -n1
  command -v clang-tidy >/dev/null && clang-tidy --version | head -n1
  command -v ccache >/dev/null && ccache --version | head -n1
  command -v gcovr >/dev/null && gcovr --version | head -n1
fi

echo
echo "Done. Next: cmake --preset debug"
