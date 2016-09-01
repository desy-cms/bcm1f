#!/bin/bash

echo >> specificrate.log 2>&1
echo "======================" >> specificrate.log 2>&1
echo " ---> specificrate.sh - START" >> specificrate.log 2>&1
date >> specificrate.log 2>&1

echo >> specificrate.log 2>&1

root -l -b -q "SpecificRatev2.C(0,1)" >> specificrate.log 2>&1
root -l -b -q "SpecificRatev2.C(1,1)" >> specificrate.log 2>&1
root -l -b -q "SpecificRatev2.C(0,2)" >> specificrate.log 2>&1
root -l -b -q "SpecificRatev2.C(1,2)" >> specificrate.log 2>&1

echo >> specificrate.log 2>&1
date >> specificrate.log 2>&1
echo " ---> specificrate.sh - END" >> specificrate.log 2>&1
echo "======================" >> specificrate.log 2>&1
echo >> specificrate.log 2>&1

