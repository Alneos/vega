$ rbe2_z.nas
ID rbe2_z.nas,NASTRAN
SOL 101
TIME  10000
CEND
SET 26 = 1 $ PROP_34
DISP = ALL
STRESSES = ALL
SUBCASE 9
  LOAD=1
  SPC=1
  MPC=17
MAXLINES=999999
$
TITLE=Vega Exported Model
BEGIN BULK
$---1--][---2--][---3--][---4--][---5--][---6--][---7--][---8--][---9--][--10--]
GRID           1              0.      0.      0.
GRID           2            1.+0      0.      0.
GRID           3            1.+0    1.+0      0.
GRID           4              0.    1.+0      0.
GRID           5              0.      0.    1.+0
GRID           6            1.+0      0.    1.+0
GRID           7            1.+0    1.+0    1.+0
GRID           8              0.    1.+0    1.+0
GRID           9            2.+0    3.+0    4.+0
$---1--][---2--][---3--][---4--][---5--][---6--][---7--][---8--][---9--][--10--]
CHEXA          1      34       1       2       3       4       5       6+20
+20            7       8        
$---1--][---2--][---3--][---4--][---5--][---6--][---7--][---8--][---9--][--10--]
MAT1           1   2.1+58.0769+4    3.-1  7.85-9
$---1--][---2--][---3--][---4--][---5--][---6--][---7--][---8--][---9--][--10--]
PSOLID        34       1       0   THREE    GRID    FULL   SMECH
$---1--][---2--][---3--][---4--][---5--][---6--][---7--][---8--][---9--][--10--]
MPC           17       2       1   -1.+0       2       5   -1.+0        +21
+21                    6       1    1.+0
MPC           17       2       2   -1.+0       2       4    1.+0        +22
+22                    6       2    1.+0
MPC           17       2       3   -1.+0       6       3    1.+0
SPC1           1     123       1
SPC1           1       1       4
SPC1           1      12       5
$---1--][---2--][---3--][---4--][---5--][---6--][---7--][---8--][---9--][--10--]
FORCE          1       7       0    1.+0-1.235-1-2.469-1-3.704-1
ENDDATA
