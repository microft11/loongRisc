#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT_DIR}"

if [ ! -d ".git" ]; then
    echo "[hooks] 当前目录不是 git 仓库根目录"
    exit 1
fi

mkdir -p .git/hooks
cp .githooks/pre-push .git/hooks/pre-push
chmod +x .git/hooks/pre-push

echo "[hooks] 已安装 pre-push hook"
