import tables, pandas as pd, pylab as py, sys, numpy
import matplotlib.dates as md
import datetime as dt
import time
import os
import glob

import vdmtime

from ROOT import TCanvas, TFile, TProfile, TNtuple, TH1F, TH2F, TTree
from ROOT import gROOT, gBenchmark, gRandom, gSystem, Double
from array import array

gROOT.Reset()

fillnr = sys.argv[1] 
#myalgoid = 2
test = False

directory = '/brildata/16/'+fillnr
filenames = glob.glob(directory+'/*.hd5')


skipMinutes = -1
durationInMinutes = 40
lastTimeStamp = 0
firstTimeStamp = 0
endVdM = int(vdmtime.blocks(fillnr)[0][1])

print "Will consider data only after the miniVdM scan, i.e. after utc time", endVdM

first = True

filename = 'rhu_'+fillnr+'.root'

if test:
   filename = 'test.root'

#if myalgoid == 2:
rootfile = TFile( '/tmp/roberval/rhuv4/'+filename, 'RECREATE' )
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

isFinished = False
hasStableBeams = False


for ifile,filename in enumerate(filenames):

    print "Opening file", filename
    
    lastBcmIndex = -1
    lastBestLumiIndex = -1

    h5file = tables.open_file(filename)
    beam = h5file.root.beam
    bcm1f = h5file.root.bcm1fagghist
    bestlumi = h5file.root.bestlumi
    tcds = h5file.root.tcds
    
    if len(beam) == 0:
       h5file.close()
       continue
       
    if tcds[0]['cmson'] == 0:  # this has to be checked; is the run for stable beams always starting with cmson == 1?
       h5file.close()
       continue
    
#    print beam.colnames
#    print bcm1f.colnames
    
#    sys.exit("oioi")

    #BCM1F
    hasBcmInfo = False
    hasBeamInfo = False 
    hasBestlumiInfo = False
    
    lastBeam = 0
    lastLumi = 0
    
    nbeam = len(beam)
    nlumi = len(bestlumi)
    
    if firstTimeStamp == 0:
       for ibeam in range(0,nbeam):
          beamTimeStamp = int(beam[ibeam]['timestampsec'])
          if beam[ibeam]['status']  == 'STABLE BEAMS':
            firstTimeStamp = endVdM+300  # allow 5 minutes to stabilise beams
            # BEAM INFO tree
            bxcfg1 = beam[ibeam]['bxconfig1']  # for the configuration, the one entry is enough
            for i,d in enumerate(bxcfg1):
               bxconfig1[i] = d
            bxcfg2 = beam[ibeam]['bxconfig2']  # for the configuration, the one entry is enough
            for i,d in enumerate(bxcfg2):
               bxconfig2[i] = d
            treeBeamInfo.Fill()
            break
            
    
    for irow,row in enumerate(bcm1f.iterrows()):
    
       nowTimeStamp = int(row['timestampsec'])
       
       if nowTimeStamp < firstTimeStamp:
          continue
       
       if nowTimeStamp > lastTimeStamp: # will not get for every bcm1f entry, only if time changes
       
           # Fill Tree! Previous time has all info, now it is time to fill it.
           if hasBestlumiInfo and hasBeamInfo:
              mytime[0] = lastTimeStamp
              tree.Fill()
              if (lastTimeStamp - firstTimeStamp) % 60 == 0:
                 print mytime[0], nowTimeStamp
              if (lastTimeStamp - firstTimeStamp) > durationInMinutes*60:
                 isFinished = True
                 break
                 
           lastTimeStamp = nowTimeStamp
                 
          
           hasBeamInfo = False 
           # BEAM INTENSITIES
#           for ibeam,beamrow in enumerate(beam.iterrows()):
           for ibeam in range(lastBeam,nbeam):
              beamTimeStamp = int(beam[ibeam]['timestampsec'])
              if nowTimeStamp == beamTimeStamp:
                 b1 = beam[ibeam]['bxintensity1']
                 b2 = beam[ibeam]['bxintensity2']
                 for idx,b1v in enumerate(b1):
                    bxi1[idx] = b1v
                 for idx,b2v in enumerate(b2):
                    bxi2[idx] = b2v
                    
                 lastBeam = ibeam
                    
                 hasBeamInfo = True
                 break
              if nowTimeStamp < beamTimeStamp:
                 break
                 
           if not hasBeamInfo:
              continue
     
           # BEST LUMI
           hasBestlumiInfo = False
#           for ilumi,lumirow in enumerate(bestlumi.iterrows()):
           for ilumi in range(lastLumi,nlumi):
              lumiTimeStamp = int(bestlumi[ilumi]['timestampsec'])
              if lumiTimeStamp == nowTimeStamp:
                 bxl = bestlumi[ilumi]['bxdelivered']
                 for ibxl,bxlv in enumerate(bxl):
                    bxlumi[ibxl] = bxlv

                 lastLumi = ilumi
                 
                 hasBestlumiInfo = True
                 break
                 
              if nowTimeStamp < lumiTimeStamp:
                 break

           if not hasBestlumiInfo:
              continue
              
                 
       # BCM1F
       channelid = int(row['channelid'])-1
       algoid = int(row['algoid'])
       data = row['data']
       for idx,d in enumerate(data):
          if algoid == 1 and channelid < 48:
             dataid1[(channelid*3564)+idx] = d
          if algoid == 2 and channelid < 48:
             dataid2[(channelid*3564)+idx] = d
       lastBcmIndex = irow

    # close h5d file
    h5file.close()
    
    if isFinished:
       break
#end of files loop          
       
rootfile.Write()
rootfile.Close()
       
