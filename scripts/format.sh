#!/usr/bin/env bash
set -euo pipefail

if ! command -v clang-format >/dev/null 2>&1; then
    echo "[format] clang-format 未安装，请先安装（例如: brew install clang-format）"
    exit 1
fi

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT_DIR}"

FILES=()
while IFS= read -r file; do
    FILES+=("${file}")
done < <(git ls-files -co --exclude-standard -- "*.h" "*.hh" "*.hpp" "*.c" "*.cc" "*.cpp" "*.cxx")

if [ "${#FILES[@]}" -eq 0 ]; then
    echo "[format] 没有需要格式化的源码文件"
    exit 0
fi

echo "[format] 正在格式化 ${#FILES[@]} 个文件..."
clang-format -i "${FILES[@]}"
echo "[format] 完成"
