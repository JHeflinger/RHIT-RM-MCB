#!/usr/bin/env python3
import os
import sys
import json
import shutil
import subprocess
from pathlib import Path
import urllib.request

source_paths = [
    "LICENCE.txt",
    "src/**/*",
    "lvgl.h",
]

with urllib.request.urlopen("https://api.github.com/repos/lvgl/lvgl/releases/latest") as response:
   tag = json.loads(response.read())["tag_name"]

# clone the repository
if "--fast" not in sys.argv:
    print("Cloning LVGL repository at tag {}...".format(tag))
    shutil.rmtree("lvgl_src", ignore_errors=True)
    subprocess.run("git clone --depth=1 --branch {} ".format(tag) +
                   "https://github.com/lvgl/lvgl.git  lvgl_src", shell=True)

# remove the sources in this repo
shutil.rmtree("src", ignore_errors=True)
if os.path.isfile("lvgl.h"): os.remove("lvgl.h")

print("Copying LVGL sources...")
for pattern in source_paths:
    for path in Path("lvgl_src").glob(pattern):
        if not path.is_file(): continue
        dest = path.relative_to("lvgl_src")
        dest.parent.mkdir(parents=True, exist_ok=True)
        print(dest)
        # Copy, normalize newline and remove trailing whitespace
        with path.open("r", newline=None, encoding="utf-8", errors="replace") as rfile, \
                           dest.open("w", encoding="utf-8") as wfile:
            wfile.writelines(l.rstrip()+"\n" for l in rfile.readlines())


subprocess.run("git add lvgl.h LICENCE.txt src", shell=True)
if subprocess.call("git diff-index --quiet HEAD --", shell=True):
    subprocess.run('git commit -m "Update LVGL to {}"'.format(tag), shell=True)
