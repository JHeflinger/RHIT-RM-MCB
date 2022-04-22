# Script is tested on OS X 10.12
# YOUR MILEAGE MAY VARY

import sys
import json
import shutil
import subprocess
from pathlib import Path
import urllib.request

source_paths = [
    "LICENSE",
    "src/**/*",
]

with urllib.request.urlopen("https://api.github.com/repos/hathach/tinyusb/releases/latest") as response:
   tag = json.loads(response.read())["tag_name"]

# clone the repository
if "--fast" not in sys.argv:
    print("Cloning TinyUSB repository at tag v{}...".format(tag))
    shutil.rmtree("tinyusb_src", ignore_errors=True)
    subprocess.run("git clone --depth=1 --branch {} ".format(tag) +
                   "https://github.com/hathach/tinyusb.git tinyusb_src", shell=True)

# remove the sources in this repo
shutil.rmtree("src", ignore_errors=True)

print("Copying TinyUSB sources...")
for pattern in source_paths:
    for path in Path("tinyusb_src").glob(pattern):
        if not path.is_file(): continue;
        dest = path.relative_to("tinyusb_src")
        dest.parent.mkdir(parents=True, exist_ok=True)
        print(dest)
        # Copy, normalize newline and remove trailing whitespace
        with path.open("r", newline=None, encoding="utf-8", errors="replace") as rfile, \
                           dest.open("w", encoding="utf-8") as wfile:
            wfile.writelines(l.rstrip()+"\n" for l in rfile.readlines())

subprocess.run("git add src LICENSE", shell=True)
if subprocess.call("git diff-index --quiet HEAD --", shell=True):
    subprocess.run('git commit -m "Update TinyUSB to v{}"'.format(tag), shell=True)
