#!/usr/bin/python3

import sys
import subprocess
import os

if len(sys.argv) < 3:
    print("usage: {:s} [c_src] [llvm-options]".format(sys.argv[0]))
    exit(1)

options = []
clang_level = ""
for arg in sys.argv:
    if arg.endswith(".c"):
        src = arg
    elif arg.startswith("-O"):
        clang_level = arg
    elif arg.startswith("-"):
        options.append(arg)

ir = src[:-2] + ".ll"
mem2reg_ir = "m2r-" + ir
opt_ir = "opt-" + ir

current_dir = os.path.dirname(os.path.realpath(__file__))
for f in os.listdir(current_dir):
    if f.endswith(".ll") or f.endswith(".bc"):
        os.remove(f)

cmds = [
    ["clang", "-emit-llvm", clang_level, src, "-S", "-o", ir],
    ["opt", "-mem2reg", ir, "-S", "-o", mem2reg_ir],
    ["opt"] + options + [mem2reg_ir, "-S", "-o", opt_ir]
]

for cmd in cmds:
    print(" ".join(c for c in cmd))
    subprocess.call(cmd)

os.remove(os.path.join(current_dir, ir))
