import sys
import os

def blocks(fill, margin = 0):
   command = "cat /brildata/vdm/* | awk -v fill='" + fill + "' -F ',' '{if ($1==fill) print $0 }'"
   
   os.system("rm -f .temp")
   os.system(command+" > .temp")
   
   f = open('.temp', 'r')
   lines = f.readlines()
   f.close()
   
   utcprev = int(lines[0].strip().split(",")[4])
   block = 0
   blocks = {}
   begin = utcprev
   end = -1
   for line in lines:
      line = line.strip()
      fillnr = int(line.split(",")[0])
      utctime = int(line.split(",")[4])
      utcdiff = utctime-utcprev
      if utcdiff > 360:  # new block - arbitrary, not expecting several VdM within minutes
         blocks[block] = [begin-margin,utcprev+margin]
         begin = utcprev
         end = -1
         block += 1
#      print utctime,utcdiff
      utcprev = utctime
   if end < 0:
      blocks[block] = [begin-margin,utcprev+margin]
   
   return blocks
   
