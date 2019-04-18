ID LINEAR,PLOADC
SOL 101
CEND


$ Case Control Commands
TITLE = PLOADC
$SUBCASE 1
$ LABEL = PLOAD4
$  LOAD = 1
$  SPC = 1 
$  DISPLACEMENT = ALL
SUBCASE 2
  LABEL = PLOAD4C
  LOAD = 2
  SPC = 1 
  DISPLACEMENT = ALL
  FORCE = ALL
  SPCFORCES = ALL
  OLOAD = ALL
$ Bulk data
BEGIN BULK

$2345678123456781234567812345678123456781234567812345678
$GRDSET                                                  456
GRID    1               0.0     0.0     0.0                                    
GRID    2               1.      0.0     0.0                                    
GRID    3               0.0     1.      0.0                                    
GRID    4               1.      1.      0.0                                    
GRID    5               0.0     0.0     1.                                     
GRID    6               1.      0.0     1.                                     
GRID    7               0.0     1.      1.                                     
GRID    8               1.      1.      1.
$
$ Material properties
$
$2345678123456781234567812345678123456781234567812345678
PSOLID  1       1       0
MAT1    1       3.+7            .3                                                                      
$
$ Constraints
$
SPC     1       1       123     0.0
SPC     1       2       23      0.0
SPC     1       3       13      0.0
SPC     1       4       3       0.0


CHEXA   100     1       1       5       6       2       3       7       +
+       8       4

$
$ PLOAD FORCE
$
$2345678123456781234567812345678123456781234567812345678
PLOAD4  1       100     -26.4                           5       8
PLOAD4  2       100     -26.4                           5       8       +
+       0       1.      0.      0.
$FORCE   2       5               -6.6    1.0 
$FORCE   2       6               -6.6    1.0
$FORCE   2       7               -6.6    1.0
$FORCE   2       8               -6.6    1.0
ENDDATA

