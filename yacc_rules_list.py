import os
import glob
def rule_list(filewithpath):
    # import pdb; pdb.set_trace()
    fid = open(filewithpath,'r')
    fid2 = open('yacc.rules.tmp','w')
    ct = 1
    prog = 0
    linecount = 0
    print('begin')
    while 1:
        linecount = linecount + 1
        line = fid.readline()
        if (len(line)==0):
            break
        if line.find('%%') >= 0:
            print('%% found')
            prog = prog + 1
        if prog == 1:
            pos_colon = line.find(':')
            if pos_colon >= 0:
                pos_sep = line.find('|')
                if (pos_sep >= 0) & (pos_sep < pos_colon):
                    pos_colon = -1
                pos_space = line.find(' ')
                if (pos_space >= 0) & (pos_space < pos_colon):
                    pos_colon = -1
            if pos_colon >= 0:
                group = line[0:pos_colon]
                line = line[pos_colon+1:len(line)]
                fid2.write("%s:\n" % group)
                pos_sep = line.find('|')
                if pos_sep<0:
                    ct = ct + 1
                    pos_comment = line.find('/*')
                    if pos_comment>= 0:
                        line = line[0:pos_comment]
                    line = line.strip()
                    if len(line) == 0:
                        line = "(null)"
                    fid2.write("\t%d\t%s\n" % (ct, line))
                else:
                    pos_sep_last = -1
                    while pos_sep >= 0:
                        ct = ct + 1
                        fid2.write("\t%d%s\n" % (ct, line[pos_sep_last+1:pos_sep]))
                        pos_sep_last = pos_sep
                        pos_sep = line.find('|', pos_sep_last+1)
                        if pos_sep < 0:
                            ct = ct + 1
                            fid2.write("\t%d%s" % (ct, line[pos_sep_last+1:len(line)]))
            else:
                pos_sep = line.find('|')
                if pos_sep > 0:
                    parsed = line[0:pos_sep].strip()
                    line = line[pos_sep+1:len(line)].strip()
                    if len(parsed) > 0:
                        ct = ct + 1
                        fid2.write("\t%d\t%s\n" % (ct, parsed))
                    pos_sep = line.find('|')
                    while pos_sep > 0:
                        parsed = line[0:pos_sep].strip()
                        line = line[pos_sep+1:len(line)].strip()
                        if len(parsed) > 0:
                            ct = ct + 1
                            fid2.write("\t%d\t%s\n" % (ct, parsed))
                        pos_sep = line.find('|')
                    ct = ct + 1
                    fid2.write("\t%d\t%s\n" % (ct, line[0:len(line)]))
        elif prog == 2:
            break
    fid.close()
    fid2.close()
    print('end')


rule_list('sigproc\psycon.y')