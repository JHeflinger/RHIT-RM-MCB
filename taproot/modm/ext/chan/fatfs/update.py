import re, os, io, time
import shutil
import zipfile
import subprocess
import urllib.request
from pathlib import Path

source_paths = [
    "LICENSE",
    "source/",
]

urls = {
    "full": ("http://elm-chan.org/fsw/ff/00index_e.html",
            r"arc/ff.+?\.zip",
             "http://elm-chan.org/fsw/ff/patches.html",
            r"\"(patch/ff.+_p\d+\.diff)\""),
    "tiny": ("http://elm-chan.org/fsw/ff/00index_p.html",
            r"arc/pff.+?\.zip",
             "http://elm-chan.org/fsw/ff/pfpatches.html",
            r"\"(patch/ff.+_p\d+\.diff)\"")
}
versions = {}

hdr = {'User-Agent': 'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.11 (KHTML, like Gecko) Chrome/23.0.1271.64 Safari/537.11',
       'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8',
       'Accept-Charset': 'ISO-8859-1,utf-8;q=0.7,*;q=0.3',
       'Accept-Encoding': 'none',
       'Accept-Language': 'en-US,en;q=0.8',
       'Connection': 'keep-alive'}

def get_file(url):
    attempts = 60
    file = None
    while attempts > 0:
        try:
            file = urllib.request.urlopen(urllib.request.Request(url, headers=hdr)).read()
            break
        except:
            attempts -= 1
            print("Failed '{}' #{}".format(url, attempts))
            time.sleep(1)
    if file is None:
        print("URL Request expired")
        exit(1)
    return file

def get_regex(url, regex):
    return re.findall(regex, get_file(url).decode("utf-8"))

for key, (umain, rzip, upatch, rpatch) in urls.items():
    # Download main zip file
    zipf = os.path.join(os.path.dirname(umain), get_regex(umain, rzip)[0])
    versions[key] = os.path.basename(zipf).replace("pff", "").replace("ff", "").replace(".zip", "")
    z = zipfile.ZipFile(io.BytesIO(get_file(zipf)))
    shutil.rmtree(key, ignore_errors=True)
    # Extract, normalize newlines and remove trailing whitespace
    for n in z.namelist():
        if any(n.startswith(s) for s in source_paths):
            outpath = Path(key, n)
            print(str(outpath))
            outpath.parent.mkdir(parents=True, exist_ok=True)
            with z.open(n, "r") as rfile, outpath.open("w") as wfile:
                wfile.writelines(l.rstrip().decode("utf-8")+"\n" for l in rfile.readlines())
    # Download and apply patches
    patches = get_regex(upatch, rpatch)
    if patches:
        versions[key] = patches[-1].replace("patch/ff", "").replace(".diff", "")
    print(versions[key])
    patches = (get_file(os.path.join(os.path.dirname(upatch), p)).decode("utf-8") for p in patches)
    # Reformat the patch *file name*: ff14a_p2.c -> ff.c
    patches = (re.sub(r"([-+]{3} [\w]+?)\d+?.*?_p\d+(\.[ch])", r"\1\2", p) for p in patches)
    for patch in patches:
        print(key, patch)
        subprocess.run("patch -l", input=patch, encoding='ascii',
                       cwd="{}/source".format(key), shell=True)

subprocess.run("git add tiny", shell=True)
if subprocess.call("git diff-index --quiet HEAD --", shell=True):
    subprocess.run('git commit -m "Update Petit FatFs to v{}"'.format(versions["tiny"]), shell=True)

subprocess.run("git add full", shell=True)
if subprocess.call("git diff-index --quiet HEAD --", shell=True):
    subprocess.run('git commit -m "Update FatFs to v{}"'.format(versions["full"]), shell=True)
