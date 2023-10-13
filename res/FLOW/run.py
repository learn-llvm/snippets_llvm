#!/usr/bin/python3

import os
import subprocess

current_dir = os.path.dirname(os.path.realpath(__file__))
for f in os.listdir(current_dir):
    if f.endswith(".dot"):
        pdf = f[:-4] + ".pdf"
        if not os.path.isfile(pdf) or\
           os.path.getmtime(f) >= os.path.getmtime(pdf):
            try:
                os.remove(pdf)
            except OSError:
                pass
            cmd_str = "dot {:s} -Tpdf -o {:s}".format(f, pdf)
            print(cmd_str)
            subprocess.call(cmd_str.split())
