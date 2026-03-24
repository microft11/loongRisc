#!/usr/bin/env bash
set -euo pipefail

if ! command -v clang-format >/dev/null 2>&1; then
    echo "[format-check] clang-format 未安装，请先安装（例如: brew install clang-format）"
    exit 1
fi

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT_DIR}"

FILES=()
while IFS= read -r file; do
    FILES+=("${file}")
done < <(git ls-files -co --exclude-standard -- "*.h" "*.hh" "*.hpp" "*.c" "*.cc" "*.cpp" "*.cxx")

if [ "${#FILES[@]}" -eq 0 ]; then
    echo "[format-check] 没有需要检查的源码文件"
    exit 0
fi

echo "[format-check] 正在检查 ${#FILES[@]} 个文件..."
clang-format --dry-run -Werror "${FILES[@]}"
echo "[format-check] 通过"
