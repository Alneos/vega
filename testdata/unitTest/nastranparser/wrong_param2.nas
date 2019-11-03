$ Executive Control Statements
ID LINEAR,WRONGPAR
SOL 101
CEND

$ Dummy commands
AZERTY=UIOP
QSDF

$ Case Control Commands
TITLE = WRONGPAR
ECHO = UNSORTED
SUBCASE 1
  LABEL = PLOAD
  LOAD = 1
  SPC = 1
  DISPLACEMENT = ALL
  GHJK=LMWX
  CVBN

$ Bulk data
BEGIN BULK

$
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
PSOLID  1       1       0       THREE   GRID    FULL    SMECH
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
PLOAD   1       -26.4   1       4       8       5
$

ENDDATA
