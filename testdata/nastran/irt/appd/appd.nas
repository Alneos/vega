ID SEMINAR, Appendix D
SOL 101
TIME 60
CEND
SEALL = ALL
SUPER = ALL
TITLE = CBAR Element Shear Factor, K
ECHO = SORT
MAXLINES = 999999999
SUBCASE 1
SUBTITLE=Default
 SPC = 2
 LOAD = 2
 DISPLACEMENT(SORT1,REAL)=ALL
 SPCFORCES(SORT1,REAL)=ALL
 STRESS(SORT1,REAL,VONMISES,BILIN)=ALL
BEGIN BULK
CBAR    1       1       1       2       0.      1.      0.
CBAR    2       1       2       3       0.      1.      0.
CBAR    3       1       3       4       0.      1.      0.
CBAR    4       1       4       5       0.      1.      0.
FORCE   1       5       0       100.    0.      -1.     0.
GRID    1               0.      0.      0.
GRID    2               1.      0.      0.
GRID    3               2.      0.      0.
GRID    4               3.      0.      0.
GRID    5               4.      0.      0.
LOAD    2       1.      1.      1
MAT1    1       1.+7            .3
$PARAM   ALTRED  NO
$PARAM   AUTOSPC YES
PARAM   COUPMASS0
PARAM   GRDPNT  0
$PARAM   INREL   0
$PARAM   K6ROT   0.
PARAM   NOCOMPS -1
PARAM   PATVER  3.
PARAM   POST    -1
PARAM   WTMASS  .00259
PBAR    1       1       .25     .0052   .0052   .0088                    +000001
++000001.25     .25     .25     -.25    -.25    .25     -.25    -.25     +000002
++000002.8333   .8333
SPC1    1       123456  1
SPCADD  2       1
ENDDATA
