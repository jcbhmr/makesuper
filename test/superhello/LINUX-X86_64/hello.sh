#!/usr/bin/env bash
set -ex
script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd -P)
cat "$script_dir/message.txt"
