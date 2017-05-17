#!/usr/bin/python3

import urllib.request
import tarfile
import platform
import os
import shutil
from multiprocessing import Pool

def dlAndExtract(tup):
	url = tup[0]
	filename = tup[1]
	extractto = tup[2]
	
	print("downloading " + filename)
	
	if not os.path.isfile(filename):
		# download
		with urllib.request.urlopen(url) as response, open(filename, 'wb') as out_file:
			shutil.copyfileobj(response, out_file)
	
	print("extracting " + filename + " to " + extractto)
	
	# extract
	tar = tarfile.open(filename)
	tar.extractall(extractto)


chigraphDir = os.path.dirname(os.path.realpath(__file__))
thirdPartyDir = os.path.join(chigraphDir, "third_party")

urls=[]

print("Downloading dependencies for system: {}".format(platform.system()))

if platform.system() == "Linux":
	
	urls.append(('https://github.com/chigraph/chigraph/releases/download/dependencies/kf5-5.32.0-debug-gcc-linux64.tar.xz', os.path.join(thirdPartyDir, "kf5-debug.tar.xz"), thirdPartyDir))
	urls.append(('https://github.com/chigraph/chigraph/releases/download/dependencies/kf5-5.32.0-release-gcc-linux64.tar.xz', os.path.join(thirdPartyDir, "kf5-release.tar.xz"), thirdPartyDir))
	
elif platform.system() == "Windows":
	urls.append(('https://github.com/chigraph/chigraph/releases/download/dependencies/flexbison-win32.tar.xz', os.path.join(thirdPartyDir, "flexbison-win32.tar.xz"), thirdPartyDir))
	urls.append(('https://github.com/chigraph/chigraph/releases/download/dependencies/gettext-win64.tar.xz', os.path.join(thirdPartyDir, "gettext-win64.tar.xz"), thirdPartyDir))
	urls.append(('https://github.com/chigraph/chigraph/releases/download/dependencies/iconv-win64.tar.xz', os.path.join(thirdPartyDir, "iconv-win64.tar.xz"), thirdPartyDir))
	urls.append(('https://github.com/chigraph/chigraph/releases/download/dependencies/zlib-win64.tar.xz', os.path.join(thirdPartyDir, "zlib-win64.tar.xz"), thirdPartyDir))
    
	urls.append(('https://github.com/chigraph/chigraph/releases/download/dependencies/kf5-5.30.0-debug-msvc14-win64.tar.xz', os.path.join(thirdPartyDir, "kf5-debug.tar.xz"), thirdPartyDir))
	urls.append(('https://github.com/chigraph/chigraph/releases/download/dependencies/kf5-5.30.0-release-msvc14-win64.tar.xz', os.path.join(thirdPartyDir, "kf5-release.tar.xz"), thirdPartyDir))
elif platform.system() == "Darwin":
    pass
else:
    print("Unrecognized system: {}".format(platform.system()))

for url in urls:
	dlAndExtract(url)
