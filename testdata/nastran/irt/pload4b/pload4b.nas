ID LINEAR,PLOADB
SOL 101
CEND


$ Case Control Commands
TITLE = PLOADB
SUBCASE 1
  LABEL = PLOAD4
  LOAD = 1
  SPC = 1
  DISPLACEMENT = ALL
SUBCASE 2
  LABEL = PLOAD4B
  LOAD = 2
  SPC = 1
  DISPLACEMENT = ALL
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
PLOAD4  1       100     -26.4                           5       8
PLOAD4  2       100     -26.4                           5       8       +
+               1.

ENDDATA

