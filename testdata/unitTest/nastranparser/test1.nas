ABBA$PAPPA  
SOL 101
DIAG 8
$TIME 3.15E7
CEND                                                                            
$                                                                            
TITLE=TEST
SUBTITLE=test analyse modale
LABEL=modes de vibrations
METHOD=1
$RESVEC(NOINREL)=YES
ECHO=NONE                                                                  
DISPLACEMENT=ALL
$STRESS=ALL
$
$SUBCASE 1
$BC=1
$
$SUBCASE 2
$BC=2
$ ALNEOS UNCOMMENT NEXT 1 LINE
SPC=1
$                                                                               
BEGIN BULK                                                                      
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
GRID      551673        434.2049-258.195371.9998
GRID      863773        434.2049-258.195371.9998
GRID      495459        434.2049-258.195371.9998
GRID      495460        434.2049-258.195371.9998
GRID      993307        434.2049-258.195371.9998
GRID      993305        434.2049-258.195371.9998
GRID      991694        434.2049-258.195371.9998
GRID      493945        434.2049-258.195371.9998
CTETRA   9820244      11 2518081 2504271 2514927 2509217
PARAM,GRDPNT,0
$PARAM,K6ROT,1.0
$PARAM,AUTOSPC,YES
PARAM,PRGPST,NO
$PARAM,NEWSEQ,-1
$PARAM,MAXRATIO,1.E7
$PARAM,SNORM,20.
PARAM,COUPMASS,-1
PARAM,TINY,0.0
PARAM,POST,-2    
$ ALNEOS COMMENT NEXT 1 LINE                                                               
$EIGRL          1    50.0   100.0
$ ALNEOS ADD NEXT 1 LINE
EIGRL          1                      50
$EIGRL          1                      5       0                
$EIGRL          1                      1.      100.
$GC next line added for test
FORCE   1       4               5000.   0.      -1.     0.
SPC1           1  123456  551673
ENDDATA
