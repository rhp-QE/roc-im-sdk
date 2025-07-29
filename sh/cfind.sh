#!/bin/bash

# 参数校验：严格单输入
if [ $# -ne 1 ]; then
  echo "Usage: $0 '类名模糊匹配字符串'"
  echo "示例: $0 vec"
  exit 1
fi

# 构造顺序敏感正则（如 "vec" → "v[A-Za-z0-9_]*e[A-Za-z0-9_]*c"）
REGEX=$(echo "$1" | sed 's/[[:space:]]//g' | sed 's/./&[A-Za-z0-9_]*/g' | sed 's/[A-Za-z0-9_]*$//; s/$/[A-Za-z0-9_]*/')

# 精确匹配 class 后第一个单词为类名
rg -i -n --type cpp --glob '*.{h,hpp,cpp,cc}' \
   "(^|\s)(class|struct)\s+(\w*${REGEX}\w*)(\s*<[^>]+>|\s*:|;|\{)" \
   -r '$3' \
   --no-heading
