import sys
import os

if len(sys.argv) < 2:
   fillBegin = "4958"
else:
   fillBegin = sys.argv[1]

command = "brilcalc lumi -b 'STABLE BEAMS' --begin " + fillBegin + " | awk -F '|' '{print $2 $6}' | grep ':' | awk '{print $1"'":"'"$2}' "

os.system("rm -f .temp")
os.system(command+" > .temp")


f = open('.temp', 'r')
lines = f.readlines()[1:]
f.close()

lumiFill = dict()

for line in lines:
   line = line.strip()
   run = int(line.split(":")[0])
   fill = int(line.split(":")[1])
   lumi = float(line.split(":")[2])/1E6
   
   keys = lumiFill.keys()
   if fill in keys:
      lumiFill[fill] += lumi
   else:
      lumiFill[fill] = lumi
      
# Open an output file
fo = open('fillReport.csv','w')

# sort by keys in reverse order
for key, value in sorted(lumiFill.iteritems(),reverse=True):
   ilumi = float('%.2f'%value)
   if ilumi > 0:
      fo.write('%i,%.2f\n'% (key,value))

fo.close()


