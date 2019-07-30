sol 101
diag 8
$time 3.15e7
cend
$
title=test
subtitle=test analyse modale
label=modes de vibrations
method=1
$resvec(noinrel)=yes
echo=none
displacement=all
$stress=all
$
$subcase 1
$bc=1
$
$subcase 2
$bc=2
$ alneos uncomment next 1 line
spc=1
$
begin bulk
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
grid      551673        434.2049-258.195371.9998
grid      863773        434.2049-258.195371.9998
grid      495459        434.2049-258.195371.9998
grid      495460        434.2049-258.195371.9998
grid      993307        434.2049-258.195371.9998
grid      993305        434.2049-258.195371.9998
grid      991694        434.2049-258.195371.9998
grid      493945        434.2049-258.195371.9998
ctetra   9820244      11 2518081 2504271 2514927 2509217
param,grdpnt,0
$param,k6rot,1.0
$param,autospc,yes
param,prgpst,no
$param,newseq,-1
$param,maxratio,1.e7
$param,snorm,20.
param,coupmass,-1
param,tiny,0.0
param,post,-2
$ alneos comment next 1 line
$eigrl          1    50.0   100.0
$ alneos add next 2 lines
eigrl          1                      50
eigrl         10        1600.0                                      mass
$eigrl          1                      5       0
$eigrl          1                      1.      100.
$gc next line added for test
force   1       4               5000.   0.      -1.     0.
spc1           1  123456  551673
enddata
