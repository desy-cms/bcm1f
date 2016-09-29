# Ntuplizer v3
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
#myalgoid = 2
test = False

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
fillInfo[5017] = [1465793400,'/brildata/16/5017/5017_274998_1606130354_1606130901.hd5']
fillInfo[5020] = [1465926300,'/brildata/16/5020/5020_275066_1606141744_1606141826.hd5']
fillInfo[5024] = [1466124300,'/brildata/16/5024/5024_275283_1606170042_1606170134.hd5']
fillInfo[5029] = [1466354400,'/brildata/16/5029/5029_275370_1606191547_1606191812.hd5']
fillInfo[5045] = [1466961600,'/brildata/16/5045/5045_275832_1606261717_1606261942.hd5']
fillInfo[5080] = [1467990900,'/brildata/16/5080/5080_276525_1607081340_1607090827.hd5']
fillInfo[5105] = [1468953000,'/brildata/16/5105/5105_277069_1607191720_1607191952.hd5']
fillInfo[5117] = [1469476500,'/brildata/16/5117/5117_277420_1607251911_1607252128.hd5']
fillInfo[5258] = [1472422500,'/brildata/16/5258/5258_279715_1608282133_1608290204.hd5']

fillInfo[5161] = [1470250800,'/brildata/16/5161/5161_278167_1608031804_1608040842.hd5']
fillInfo[5187] = [1470768300,'/brildata/16/5187/5187_278509_1608091753_1608100402.hd5']
fillInfo[5211] = [1471419300,'/brildata/16/5211/5211_278969_1608170649_1608171618.hd5']
fillInfo[5251] = [1472160600,'/brildata/16/5251/5251_279588_1608252028_1608260440.hd5']
fillInfo[5282] = [1473176700,'/brildata/16/5282/5282_280242_1609061542_1609061948.hd5']

fillInfo[5096] = [1468569900,'/brildata/16/5096/5096_276831_1607150722_1607160054.hd5']


fillInfo[5287] = [1473324300,'/brildata/16/5287/5287_280330_1609080845_1609081421.hd5']
fillInfo[5288] = [1473412500,'/brildata/16/5288/5288_280385_1609090910_1609092220.hd5']
fillInfo[5264] = [1472602500,'/brildata/16/5264/5264_279794_1608302310_1608310626.hd5']
fillInfo[5267] = [1472715300,'/brildata/16/5267/5267_279844_1609010652_1609010847.hd5']

fillInfo[5110] = [1469281560,'/brildata/16/5110/5110_277180_1607231303_1607231443.hd5']
fillInfo[5111] = [1469298060,'/brildata/16/5111/5111_277194_1607231740_1607240708.hd5']

#fillInfo[5069] = [1467459780,'/brildata/16/5069/5069_276226_1607021142_1607021224.hd5']
#fillInfo[5017] = [1465884000,'/brildata/16/5017/5017_275001_1606131811_1606140736.hd5']

fillInfo[5331] = [1474783200,'/brildata/16/5331/5331_281613_1609250523_1609251118.hd5']
fillInfo[5332] = [1474812600,'/brildata/16/5332/5332_281616_1609251337_1609251638.hd5']
fillInfo[5338] = [1474876560,'/brildata/16/5338/5338_281639_1609260744_1609260836.hd5']
fillInfo[5339] = [1474909200,'/brildata/16/5339/5339_281693_1609261656_1609270711.hd5']


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

if test:
   filename = 'test.root'

#if myalgoid == 2:
rootfile = TFile( '/tmp/roberval/rhu/'+filename, 'RECREATE' )
#else:
#   rootfile = TFile( '/tmp/roberval/rhu/algoid1/'+filename, 'RECREATE' )

print 'Producing file: ',filename

nbins = 3564
nchannels = 48
mytime = array('l',[0])
dataid1 = array('f',171072*[0.])  # nbins * nchannels
dataid2 = array('f',171072*[0.])  # nbins * nchannels
bxi1 = array('f',nbins*[0.])  # nbins
bxi2 = array('f',nbins*[0.])  # nbins
bxlumi = array('f',nbins*[0.])  # nbins

#channelData = [[0 for x in range(nbins)] for y in range(nch)] 

tree = TTree("rhu","RHU tree")
tree.Branch('time',mytime,'time/I')
tree.Branch('dataid1',dataid1,'dataid1[171072]/F')
tree.Branch('dataid2',dataid2,'dataid2[171072]/F')
tree.Branch('bxi1',bxi1,'bxi1[3564]/F')
tree.Branch('bxi2',bxi2,'bxi2[3564]/F')
tree.Branch('bxlumi',bxlumi,'bxlumi[3564]/F')

bxconfig1 = array('i',nbins*[0])  # nbins
bxconfig2 = array('i',nbins*[0])  # nbins
treeBeamInfo = TTree("beaminfo","beam info")
treeBeamInfo.Branch('bxconfig1',bxconfig1,'bxconfig1[3564]/I')
treeBeamInfo.Branch('bxconfig2',bxconfig2,'bxconfig2[3564]/I')


for ifile,filename in enumerate(filenames):

    print "Opening file", filename

    h5file = tables.open_file(filename)
    beam = h5file.root.beam
    bcm1f = h5file.root.bcm1fagghist
    bestlumi = h5file.root.bestlumi
    
#    print beam.colnames
#    print bcm1f.colnames
    
#    sys.exit("oioi")
    
    if ifile == 0:
       try:
          bxcfg1 = beam[0]['bxconfig1']  # fot hte configuration, the first entry is enough
          for i,d in enumerate(bxcfg1):
             bxconfig1[i] = d
          bxcfg2 = beam[0]['bxconfig2']  # fot hte configuration, the first entry is enough
          for i,d in enumerate(bxcfg2):
             bxconfig2[i] = d
       except:
          continue
          
       treeBeamInfo.Fill()
       
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
#             if abs(beamTS - nowTimeStamp) < 2 and beamrow['status'] == 'STABLE BEAMS':
             if abs(beamTS - nowTimeStamp) < 2:
                hasbeaminfo = True
                b1 = beamrow['bxintensity1']
                b2 = beamrow['bxintensity2']
                for idx,b1v in enumerate(b1):
                   bxi1[idx] = b1v
                for idx,b2v in enumerate(b2):
                   bxi2[idx] = b2v
                break
          # BEST LUMI
          hasbestlumiinfo = False
          for ilumi,lumirow in enumerate(bestlumi.iterrows()):
             lumiTS = int(lumirow['timestampsec'])
             if abs(lumiTS - nowTimeStamp) < 2:
                hasbestlumiinfo = True
                bxl = lumirow['bxdelivered']
                for ibxl,bxlv in enumerate(bxl):
                   bxlumi[ibxl] = bxlv
                break
          # fill tree!
          if hasbeaminfo and hasbestlumiinfo:
#             print "filling tree"
             tree.Fill()
             
          lastTimeStamp = nowTimeStamp
          
       # BCM1F DATA
       channelid = int(row['channelid'])-1
       algoid = int(row['algoid'])
       data = row['data']
       for idx,d in enumerate(data):
          if algoid == 1 and channelid < 48:
             dataid1[(channelid*3564)+idx] = d
          if algoid == 2 and channelid < 48:
             dataid2[(channelid*3564)+idx] = d
       
       mytime[0] = nowTimeStamp - firstTimeStamp
       
rootfile.Write()
rootfile.Close()
       
