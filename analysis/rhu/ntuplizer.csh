#!/bin/csh -f

if ( $#argv < 1 ) then
   echo "Usage: ntuplizer.csh <fill number>"
   exit
endif

set dir = /tmp/bcm1f/rhu/
if ( ! -d $dir ) then
   mkdir -p $dir
endif

set fill = $1
python NtuplizerV4.py $fill
set file = /tmp/bcm1f/rhu/rhu_$fill.root
if ( -e $file ) then
   root -l "CheckRates.C($fill)"
endif
