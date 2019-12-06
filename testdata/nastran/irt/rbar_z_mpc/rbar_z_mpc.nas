SOL 101
DIAG 8
TIME 10000
CEND
$
TITLE=STIFFNESS IN TORSION
SUBTITLE=TORSION
LABEL=STIFFNESS
ECHO=NONE
DISPLACEMENT=ALL
FORCE=ALL
SPCFORCE=ALL
STRESS=ALL
LOAD=1
SPC=1
MPC=17
AUTOSPC=NO
MPCFORCES=ALL
$
BEGIN BULK
GRID    1       0       0.00E+000.00E+000.00E+00
GRID    2       0       1.0000000.00E+000.00E+00
GRID    3       0       1.00E+001.0000000.00E+00
GRID    4       0       0.0000001.00E+000.00E+00
GRID    5       0       0.00E+000.00E+001.00E+00
GRID    6       0       1.0000000.00E+001.00E+00
GRID    7       0       1.00E+001.0000001.00E+00
GRID    8       0       0.0000001.00E+001.00E+00
GRID    9       0       2.00E+003.0000004.00E+00
CHEXA          1      34       1       2       3       4       5       6+000002
+000002        7       8
$SPC
SPC1           1     123       1
SPC1           1       1       4
SPC1           1      12       5
MPC           17       2       1   -1.+0       6       1    1.+0
MPC           17       2       2   -1.+0       6       2    1.+0
MPC           17       2       3   -1.+0       6       3    1.+0
FORCE          1       7        .1234567     -1.     -2.     -3.
MAT1           1 210000.             0.3  7.85-9                        +01FE706
+01FE706
PSOLID        34       1       0   THREE    GRID    FULL   SMECH
ENDDATA
