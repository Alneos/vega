SOL 105
TIME 600
CEND
TITLE = MSC/NASTRAN job created on 15-Jan-98 at 14:11:15
ECHO = NONE
MAXLINES = 999999999
SUBCASE 1
 SUBTITLE=Default
 SPC = 2
 LOAD = 2
 DISPLACEMENT(SORT2,REAL)=ALL
SUBCASE 2
 SUBTITLE=Default
 SPC = 2
 METHOD = 1
BEGIN BULK
CROD    1       1       1       2
EIGB    1       INV     0.      3.      20      2       2               +A
+A      MAX
FORCE   1       2       0       6.      0.      -1.     0.
GRID    1               0.      0.      0.
GRID    2               100.    1.      0.
LOAD    2       1.      1.      1
MAT1    1       1.+8
$PARAM   AUTOSPC YES
PARAM   COUPMASS-1
PARAM   K6ROT   0.
PARAM   NOCOMPS -1
PARAM   PATVER  3.
PARAM   POST    -1
PARAM   PRTMAXIMYES
PARAM   WTMASS  1.
PROD    1       1       .1
SPC1    1       123456  1
SPC1    3       13456   2
SPCADD  2       1       3
ENDDATA
