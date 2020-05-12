import os
import glob
def v141_142(path):
	os.chdir(path)
	files=glob.glob('*.vcxproj')
	for file in files:
		fid = open(file,'r')
		fid2 = open(file+'0','w')
		lastpt = 0
		fid.seek(0,0)
		while 1:
			line = fid.readline()
			pt = fid.tell()
			if (len(line)==0):
				break
			loc = line.find('<PlatformToolset>')
			if  loc >= 0:
				loc1 = line.find('</PlatformToolset>')
				if loc1 < 0:
					print('<PlatformToolset> not matched in ' + file + ' !')
					fid.close()
					return
				fid2.write(line[0:loc+len('<PlatformToolset>')])
				targetStr = line[loc+len('<PlatformToolset>'):loc1]
				if targetStr == 'v141':
					fid2.write('v142')
				else:
					fid2.write(targetStr)
				after_targetStr = line[loc+len('<PlatformToolset>')+4:len(line)]
				fid2.write(after_targetStr)
			else:
				fid2.write(line)
		fid.close()
		fid2.close()
		os.remove(file)
		os.rename(file+'0',file)

v141_142("c:\\temp\\temp")


