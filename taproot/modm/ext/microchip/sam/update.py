import io
import re
import sys
import shutil
import zipfile
import subprocess
import urllib.request
from pathlib import Path
from time import strftime
from collections import defaultdict

sam_families = [
    "SAMD21", "SAMD51", "SAML21"
]
packurl = "http://packs.download.atmel.com/"

# gets the pack version out of the Download URL of the pack (or just the pack name)
def get_remote_pack_version(pack_dl_url):
    vmatch = re.search(r'_DFP\.([0-9]+\.[0-9]+\.[0-9]+)\.atpack', pack_dl_url)
    return vmatch.group(1) if vmatch else None

# find the pack name of a device from the entire pack website HTML and create a download url
def get_remote_pack_url(html, family):
    atpack = re.search(r'data-link="(Atmel\.{}_DFP\..*?\.atpack)"'.format(family), html)
    return packurl + atpack.group(1) if atpack else None

# extract a friendly and readable pack name from the download URL
def get_pack_from_url(packUrl):
    packname = re.search(r'(Atmel\..*?_DFP\..*?\.atpack)', packUrl)
    return packname.group(1) if packUrl else None

pack_remote_version = {}
pack_dl_url = {}
# parse the versions and download links from the Microchip Packs site
with urllib.request.urlopen(packurl) as response:
    html = response.read().decode("utf-8")
    for family in sam_families:
        # find the pack file name in the html
        pack_dl_url[family] = get_remote_pack_url(html, family)
        if not pack_dl_url[family]:
            print("No zip download link for", family)
            exit(1)
        pack_remote_version[family] = get_remote_pack_version(pack_dl_url[family])
        if not pack_dl_url[family]:
            print("No version match in remote html for", family)
            exit(1)

# easiest way to get the pack date is to check modification date of files
# only other location is in the .pdsc file changelog (which would be nice
# to parse eventually). just make sure to get date BEFORE modifying file!
pack_remote_date = {}
family_folders = defaultdict(set)

# Download packs needing updates and extract into appropriate directory
for family in sam_families:
    # remove old versions
    shutil.rmtree(family, ignore_errors=True)
    # download the new pack
    familyUrl = pack_dl_url[family]
    print( "Downloading '{}'...".format( get_pack_from_url(familyUrl) ))
    with urllib.request.urlopen(familyUrl) as content:
        z = zipfile.ZipFile(io.BytesIO(content.read()))
        print("Extracting '{}'...".format( get_pack_from_url(familyUrl) ))
        # for now extract just the include directory for each chip
        # but retain structure. Different directory for each variant
        for f in z.infolist():
            pack_remote_date[family] = f.date_time + (0,0,-1)
            if f.filename.startswith("sam"):
                family_folders[family].add(f.filename.split("/")[0])
            if re.match(r'sam.*?\/include\/.*?\.h', f.filename):
                print(f.filename)
                dest = Path(f.filename)
                dest.parent.mkdir(parents=True, exist_ok=True)
                with z.open(f, "r") as rfile, dest.open("w") as wfile:
                    wfile.writelines(l.rstrip().decode("utf-8")+"\n" for l in rfile.readlines())

def update_readme(readme, family, new_version, new_date, new_url):
    match = r"\[{0}: v.+? created .+?\]\(.+?\)".format(family.upper())
    replace = "[{0}: v{1} created {2}]({3})".format(
                    family.upper(), new_version, new_date, new_url)
    return re.sub(match, replace, readme)

readme_path = Path("README.md")
for family in sam_families:
    readme = readme_path.read_text()
    readme = update_readme(readme, family,
                           pack_remote_version[family],
                           strftime("%d-%B-%Y", pack_remote_date[family]),
                           pack_dl_url[family])
    readme_path.write_text(readme)
    subprocess.run("git add README.md {}".format(" ".join(family_folders[family])), shell=True)
    if subprocess.call("git diff-index --quiet HEAD --", shell=True):
        subprocess.run('git commit -m "Update {} headers to v{}"'.format(family, pack_remote_version[family]), shell=True)

