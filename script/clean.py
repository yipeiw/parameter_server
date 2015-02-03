#!/usr/bin/env python

import sys
import os.path as path

filelist = sys.argv[1]
cleanfile = sys.argv[2]

def Extract(logfile):
	filterTag = {}
	iterInfo = [-1, '']
	delay = -1
	for line in open(logfile):
		pos = line.find('delayed block')
		linelist = line.strip().split()
		if pos!=-1:
			delay = int(line[pos-2])
		if line.find('round filter')!=-1:
			filterTag['round'] = int(linelist[len(linelist)-1])
		if line.find('kkt_filter')!=-1:
			filterTag['kkt'] = int(linelist[len(linelist)-1])
		if line.find('Stopped: ')!=-1:
			iterInfo = [1, lastline]
		if line.find('Reached maximal')!=-1:
			iterInfo = [0, lastline]

		lastline = line
	return iterInfo, filterTag, delay

def Output(fout, iterInfo, filterTag, delay):
	if iterInfo[0]>0:
		fout.write("converge\n"+iterInfo[1])
	else:
		fout.write("unconverge\n" + iterInfo[1])
	for name, tag in filterTag.items():
		fout.write("%s %s; " % (name, tag))
	fout.write("\ndelay blocks: %s\n" % delay)

	
fout = open(cleanfile, 'w')
iter_rank = []
for line in open(filelist):
	logfile = line.strip()
	logname = path.basename(logfile)
	fout.write(logname+'\n')
	iterInfo, filterTag, delay = Extract(logfile)
	iterNum = int(iterInfo[1].split()[0].strip())
	iter_rank += [(iterNum, logname)]
	Output(fout, iterInfo, filterTag, delay)
fout.close()

ranks = sorted(iter_rank, key=lambda item:item[0])

print ranks[0]
print ranks[1]
print ranks[2]
