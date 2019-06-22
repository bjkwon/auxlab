# The purpose of this script is to add fprintf lines for CSignal::timestretch() in csignals.cpp
# or remove them if csignals.cpp already has them
# At the conclusion of this script, temp_csignals_timestretch_log.cpp is created for a backup of the input csignals.cpp used. You may delete it if the operation was done successfully.
# Two things to remember 
# 1) provide // timestretch_log.py #_ to the lines to insert the fprintf lines to in csignals.cpp
# 2) provide the lines for fprintf (and more lines such as fopen, fclose and lines with braces as needed) for each // timestretch_log.py #_ line provided, for the newlines variable below, one for each line
# BJ Kwon 6/17/2019
import pdb; # call pdb.set_trace() later 
import os
from shutil import copyfile
def addfprintf(fname1):
	copyfile(fname1,'temp_timestretch_log.cpp')
	fid = open(fname1)
	fname2 = fname1 + '2'
	fid2 = open(fname2, 'w')
	lineID=0
	matchID=0
	newlines = ['\tFILE*fp = fopen(\"c:\\\\temp\\\\timestretch_log1.txt\", \"at\");\n\tfprintf(fp, \"=============================================\\nfs=%d, Input size=%d\\n\", input2.GetFs(), nSamples);\n',\
	'\tfprintf(fp, \"winLen = %d, winLenHalf = %d, tolerance = %d\\nnew_buff_in[%d] <== old_in[0], new_buff_in[%d] <== old_in[%d]\\n%d outbuffer allocated with zeropadding\\n\", winLen, winLenHalf, tolerance, winLenHalf + tolerance, nSamples + winLenHalf + tolerance - 1, nSamples - 1, input2.nSamples);\n',\
	'\tfprintf(fp, \"loop%8s%8s%7s%3s%15s%15s    %-29s%6s%5s%7s%8s\\n\",\"ingr\", \"outgr\", \"ratio\", \"\", \"INPUT -->\", \"OUTPUT\", \"Input Autocorr w/ next block\", \"maxid\", \"del\", \"Hop_in\", \"Hop_out\");\n',\
	'\tdouble ratio0 = (double)_synHop / (ingr[m + 1] - ingr[m]);\n\tfprintf(fp, \"%3d:%8d%8d %6.3f   [%6d:%6d]->[%6d:%6d]  (%6d:%6d)(%6d:%6d) \", m, ingr[m], outgr[m], ratio0, \n\txid0, xid0 + winLen, yid0, yid0 + winLen, corrIDX1, corrIDX1+len1-1, corrIDX2, corrIDX2+winLen-1);\n',\
	'\tfprintf(fp, \"%4d ", maxid);\n',\
	'fprintf(fp, \"%4d%7d%7d\\n\", del, ingr[m+1]-ingr[m], outgr[m+1]-outgr[m]);\n',\
	'else\n\tfprintf(fp, \"%3d:%8d%8d%10s[%6d:%6d]->[%6d:%6d]\\n\", m, ingr[m], outgr[m], "", xid0, xid0 + winLen, yid0, yid0 + winLen);\n',\
	'\tfclose(fp);\n',\
	'\tFILE*fp = fopen(\"c:\\\\temp\\\\timestretch_log.txt\", \"at\");\n\tfprintf(fp, \"outbuffer from %d to %d copied as the result, size=%d\\n\", lastOutIndex - targetSize + 1, lastOutIndex, targetSize);\nfclose(fp);\n']
	# if // timestretch_log.py #0 BEGIN is found, the file is flagged as logging (normal2log set False)
	# else if // timestretch_log.py #0 is found, the file is flagged as normal (normal2log set True)
	# else nothing happens.
	logging = False
	normal2log = True # true for normal source to logging source.
	while 1:
		line = fid.readline()
		if (len(line)==0):
			break
		lineID += 1
		lookfor = '// timestretch_log.py #' + str(matchID)
		if logging:
			#just find out if this is // timestretch_log.py END, 
			if line.find(lookfor + ' END\n') >= 0:
				matchID += 1
				logging = False
		else:
			if line.find(lookfor) >= 0:
				# if // timestretch_log.py #_ is found, first check logging (if logging already started)
					lookfor1 = lookfor + ' BEGIN\n'
					if line.find(lookfor1) >= 0:
						normal2log = False
					if normal2log == True:
						fid2.write(lookfor1)
						fid2.write(newlines[matchID])
						matchID += 1
						lookfor2 = lookfor + ' END\n'
						fid2.write(lookfor2)
					else: # if // timestretch_log.py #_ BEGIN is found, skip the line until // timestretch_log.py #_ END is found
						logging = True
						fid2.write(lookfor+'\n') # writing // timestretch_log.py only... no BEGIN or END
			else:
				fid2.write(line)
	fid.close()
	fid2.close()
	os.remove(fname1)
	os.rename(fname2,fname1)
	
addfprintf('c:/Users/BJ/Documents/repos/auxlab/aux_builtin_ext/csignalsExt.cpp')
