# Script is tested on OS X 10.12
# YOUR MILEAGE MAY VARY

import sys
import shutil
import fnmatch
import subprocess
from pathlib import Path

source_paths = {
    "CrashCatcher": [
        "README.creole",
        "Core/src/**/*",
        "include/CrashCatcher.h",
    ],
    "CrashDebug": [
        "README.creole",
        "bins/**/*"
    ]
}

# clone the repository
for repo in source_paths:
    if "--fast" not in sys.argv:
        shutil.rmtree(f"{repo}_src", ignore_errors=True)
        print(f"Cloning {repo} repositories...")
        subprocess.run(f"git clone --depth=1 https://github.com/adamgreen/{repo}.git {repo}_src", shell=True)

    # remove the sources in this repo
    shutil.rmtree(repo, ignore_errors=True)

    print(f"Copying {repo} sources...")
    for pattern in source_paths[repo]:
        for path in Path(f"{repo}_src").glob(pattern):
            if not path.is_file(): continue
            dest = Path(repo) / path.relative_to(f"{repo}_src")
            dest.parent.mkdir(parents=True, exist_ok=True)
            print(dest)
            # Copy, normalize newline and remove trailing whitespace
            with path.open("r", newline=None, encoding="utf-8", errors="replace") as rfile, \
                               dest.open("w", encoding="utf-8") as wfile:
                wfile.writelines(l.rstrip()+"\n" for l in rfile.readlines())

subprocess.run("git add CrashCatcher CrashDebug", shell=True)
if subprocess.call("git diff-index --quiet HEAD --", shell=True):
    subprocess.run('git commit -m "Update CrashCatcher to latest version"', shell=True)
