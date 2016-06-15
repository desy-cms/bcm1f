import tables, pandas as pd, pylab as py, sys, numpy
import matplotlib.dates as md
import datetime as dt
import time
import os

from ROOT import TCanvas, TFile, TProfile, TNtuple, TH1F, TH2F, TTree
from ROOT import gROOT, gBenchmark, gRandom, gSystem, Double
from array import array

gROOT.Reset()

fillnr = sys.argv[1] 
myalgoid = 2

fillInfo = {}

fillInfo[4958] = [1464437700,'/brildata/16/4958/4958_274157_1605281109_1605281437.hd5']
fillInfo[4964] = [1464619500,'/brildata/16/4964/4964_274241_1605301442_1605302220.hd5']
fillInfo[4976] = [1464753000,'/brildata/16/4976/4976_274284_1606010334_1606010456.hd5']
fillInfo[4979] = [1464793800,'/brildata/16/4979/4979_274314_1606011424_1606011529.hd5']
fillInfo[4980] = [1464861000,'/brildata/16/4980/4980_274335_1606020847_1606021517.hd5']
fillInfo[4985] = [1464983400,'/brildata/16/4985/4985_274387_1606031835_1606032127.hd5']
fillInfo[4988] = [1465073400,'/brildata/16/4988/4988_274420_1606041940_1606042125.hd5']
fillInfo[4990] = [1465156200,'/brildata/16/4990/4990_274440_1606051856_1606052209.hd5']
fillInfo[5005] = [1465646700,'/brildata/16/5005/5005_274958_1606111202_1606111411.hd5']
fillInfo[5013] = [1465689900,'/brildata/16/5013/5013_274968_1606112250_1606120639.hd5']
#fillInfo[5017] = [1465793400,'/brildata/16/5017/5017_274998_1606130354_1606130901.hd5']
fillInfo[5017] = [1465884000,'/brildata/16/5017/5017_275001_1606131811_1606140736.hd5']


timestampBeg = fillInfo[int(fillnr)][0]
timestampEnd = 9999999999

skipMinutes = -1
durationInMinutes = 40

filenames = [ 
              fillInfo[int(fillnr)][1],
              ]
#fillnr = filenames[0].split("/")[3]

firstTimeStamp = 0
lastTimeStamp  = 0
first = True

filename = 'rhu_'+fillnr+'_'+str(timestampBeg)+'.root'

rootfile = TFile( '/tmp/roberval/rhu/'+filename, 'RECREATE' )

print 'Producing file: ',filename

nbins = 3564
nchannels = 48
mytime = array('l',[0])
mydata = array('f',171072*[0.])  # nbins * nchannels
bxi1 = array('f',nbins*[0.])  # nbins * nchannels
bxi2 = array('f',nbins*[0.])  # nbins * nchannels

#channelData = [[0 for x in range(nbins)] for y in range(nch)] 

tree = TTree("rhu","RHU tree")
tree.Branch('time',mytime,'time/I')
tree.Branch('data',mydata,'data[171072]/F')
tree.Branch('bxi1',bxi1,'bxi1[3564]/F')
tree.Branch('bxi2',bxi2,'bxi2[3564]/F')


for filename in filenames:

    print "Opening file", filename

    h5file = tables.open_file(filename)
    beam = h5file.root.beam
    bcm1f = h5file.root.bcm1fagghist
    
#    print beam.colnames
#    print bcm1f.colnames
    
    for irow,row in enumerate(bcm1f.iterrows()):
       nowTimeStamp = int(row['timestampsec'])
       
       if first:
          if skipMinutes > 0:
             timestampBeg = int(row['timestampsec'])+(skipMinutes*60)
          if durationInMinutes > 0:
             timestampEnd = timestampBeg + (durationInMinutes*60)
          else:
             timestampEnd = 9999999999
          print "Time from ", timestampBeg, " until time ", timestampEnd
          firstTimeStamp = timestampBeg
          lastTimeStamp  = int(row['timestampsec'])
          first = False
          
       if nowTimeStamp < int(timestampBeg): # not in time range
          continue
       if nowTimeStamp > int(timestampEnd) :
          break  
          
       if nowTimeStamp > lastTimeStamp: # new time; time to fill the tree 
          # BEAM INTENSITIES
          hasbeaminfo = False 
          for ibeam,beamrow in enumerate(beam.iterrows()):
             beamTS = int(beamrow['timestampsec'])
             if abs(beamTS - nowTimeStamp) < 2:
                hasbeaminfo = True
                b1 = beamrow['bxintensity1']
                b2 = beamrow['bxintensity2']
                for idx,b1 in enumerate(b1):
                   bxi1[idx] = b1
                for idx,b2 in enumerate(b2):
                   bxi2[idx] = b2
                break;
          if hasbeaminfo:
             tree.Fill()
          lastTimeStamp = nowTimeStamp
          
       # BCM1F DATA
       channelid = int(row['channelid'])-1
       algoid = int(row['algoid'])
       if algoid == myalgoid and channelid < 48:
          data = row['data']
          for idx,d in enumerate(data):
             mydata[(channelid*3564)+idx] = d
       
       mytime[0] = nowTimeStamp - firstTimeStamp
       
rootfile.Write()
rootfile.Close()
       
