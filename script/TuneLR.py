#!/usr/bin/env python

import os.path as path
import sys

tmpDir = '../config/tmp/'
logDir = '../config/tmp/log/'

conffile = sys.argv[1]
runfile=sys.argv[2]
lr = [0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0]

fout = open(runfile, 'w')
fout.write("#!/bin/bash\n\n\n")

fws = {}
confname = path.splitext(path.basename(conffile))[0]
loglist = confname+'.meta.log'
fl = open(loglist, 'w')
for i in range(0, len(lr)):
	filename = confname+'_'+str(lr[i])
	tmpfile = path.join(tmpDir, filename+'.conf')
	logfile = path.join(logDir, filename + '.txt')
	fws[i] = open(tmpfile, 'w')
	fout.write("echo \""+"./local.sh 1 4 "+tmpfile + " 2>"+logfile+'\"\n\n')
	fout.write("./local.sh 1 4 "+tmpfile + " 2>"+logfile+'\n\n\n')

	fl.write(logfile+'\n')

fout.close()
fl.close()

for line in open(conffile):
	if line.find("eta")==0:
		for i in range(0, len(lr)):
			output = "eta: "+str(lr[i]) + '\n'
			fws[i].write(output)
	else:
		for i in range(0, len(lr)):
			fws[i].write(line)

for i in range(0, len(lr)):
	fws[i].close()

