ID CBAR w/ ,MSC/N
SOL SESTATICS
TIME 10000
CEND
 ECHO = NONE
 DISPLACEMENT(PLOT) = ALL
 OLOAD(PLOT) = ALL
 SPCFORCE(PLOT) = ALL
 FORCE(PLOT) = ALL
 STRESS(PLOT) = ALL
 SPC = 1
 LOAD = 1
BEGIN BULK
PARAM,PRGPST,NO
PARAM,POST,0
PARAM,AUTOSPC,NO
PARAM,K6ROT,100.
$PARAM,MAXRATIO,1.E+8
PARAM,GRDPNT,0
PARAM,WTMASS,0.00259
CBAR    1       1       1       2       0.      1.      0.
CBAR    2       1       2       3       0.      1.      0.
CBAR    3       1       3       4       0.      1.      0.
CBAR    4       1       4       5       0.      1.      0.
CORD2C  1       0       0.      0.      0.      0.      0.      1.      +MSC/NC1
+MSC/NC11.      0.      1.
CORD2S  2       0       0.      0.      0.      0.      0.      1.      +MSC/NC2
+MSC/NC21.      0.      1.
FORCE   1       5       0       1.      0.      -100.   0.
GRID    1       0       0.      0.      0.      0
GRID    2       0       1.      0.      0.      0
GRID    3       0       2.      0.      0.      0
GRID    4       0       3.      0.      0.      0
GRID    5       0       4.      0.      0.      0
MAT1    1       1.E+7           0.3     0.      0.      0.
PBAR    1       1       0.25    0.0052  0.0052  0.0088  0.              +PR1
+PR1    0.25    0.25    0.25    -0.25   -0.25   0.25    -0.25   -0.25   +PA1
+PA1    0.8333  0.8333  0.
SPC     1       1       123456  0.
ENDDATA
