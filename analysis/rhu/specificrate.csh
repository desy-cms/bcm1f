#!/bin/csh -f

echo >>& specificrate.log
echo "======================" >>& specificrate.log
echo " ---> specificrate.csh - START" >>& specificrate.log
date >>& specificrate.log

echo >>& specificrate.log

root -l -b -q "SpecificRatev2.C(0,1)" >>& specificrate.log
root -l -b -q "SpecificRatev2.C(1,1)" >>& specificrate.log
root -l -b -q "SpecificRatev2.C(0,2)" >>& specificrate.log
root -l -b -q "SpecificRatev2.C(1,2)" >>& specificrate.log

echo >>& specificrate.log
date >>& specificrate.log
echo " ---> specificrate.csh - END" >>& specificrate.log
echo "======================" >>& specificrate.log
echo >>& specificrate.log

