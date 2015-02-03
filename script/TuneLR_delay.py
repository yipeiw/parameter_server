#!/usr/bin/env python

import os.path as path
import sys

tmpDir = '../config/tmp/'
logDir = '../config/tmp/log/'

conffile = sys.argv[1]
runfile=sys.argv[2]
d = int(sys.argv[3])

l1 = [0.9,1.0,1.2,1.5,2.0]
l2 = [0.2, 0.25, 0.3, 0.35, 0.4]

if conffile.find('l1')!=-1:
	lr = l1
else:
	lr = l2

fout = open(runfile, 'w')
fout.write("#!/bin/bash\n\n\n")

fws = {}
confname = path.splitext(path.basename(conffile))[0]
loglist = 'Delay%s_' %(d)+confname+'.meta.log'
fl = open(loglist, 'w')
for i in range(0, len(lr)):
	filename = confname+'_'+str(lr[i])
	tmpfile = path.join(tmpDir, 'Delay%s_' % (d)+filename+'.conf')
	logfile = path.join(logDir, 'Delay%s_' % (d)+filename + '.txt')
	fws[i] = open(tmpfile, 'w')
	fout.write("echo \""+"./local.sh 1 4 "+tmpfile + " 2>"+logfile+'\"\n\n')
	fout.write("./local.sh 1 4 "+tmpfile + " 2>"+logfile+'\n\n\n')

	fl.write(logfile+'\n')

fout.close()
fl.close()

for line in open(conffile):
	if line.find("max_block_delay")!=-1:
		for i in range(0, len(lr)):
                	fws[i].write("max_block_delay : %s\n" % d)
		continue
 
	if line.find("eta")==0:
		for i in range(0, len(lr)):
			output = "eta: "+str(lr[i]) + '\n'
			fws[i].write(output)
		continue
	for i in range(0, len(lr)):
		fws[i].write(line)

for i in range(0, len(lr)):
	fws[i].close()

