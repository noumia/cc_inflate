# run.py

import os
import sys
import subprocess

def run(file) :
	cmd = [ "../Release/cc_ungzip.exe", file ]
	subprocess.check_call(cmd)

args = sys.argv[1:]
if len(args) == 0:
	sys.exit(-1)

print "BasePath:", args[0]

for root, dirs, files in os.walk(args[0]):
	for file in files:
		path = os.path.join(root, file)
		(name, ext) = os.path.splitext(path)
		if ext != ".gz":
			continue
		print "Path:", path
		run(path)

