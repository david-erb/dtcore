#!/usr/bin/env bash
set -euo pipefail

APP=$1
REPO_DIR="$(cd "$(dirname "$0")/.." && pwd)"
APP_DIR="$REPO_DIR/apps_linux/$APP"

echo "REPO_DIR: $REPO_DIR"

if [ -d /opt/docker30 ]; then
    for f in /opt/docker30/*.yml; do
        echo "=== $f ==="
        cat "$f"
    done
fi

cd "$APP_DIR"
echo "Running CMake build for $APP_DIR..."
rm -rf build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
build/app
