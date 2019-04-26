ID NAS103 WORKSHOP 5 SOLUTION
SOL 101
TIME 30
CEND
TITLE = SIMPLE TENSION
SUBTITLE = DISPLACEMENT CONTROL MPCs + 1 SPCD
DISP = ALL
MPC=100
SUBCASE 100 $ UNIAXIAL TENSION
 NLPARM=10
 SPC=100
 LOAD=1000
BEGIN BULK
CHEXA   1       1       1       2       3       4       5       6        +00000P
++00000P7       8
GRID    1               0.      0.      0.              123456
GRID    2               1.      0.      0.              23456
GRID    3               1.      1.      0.              3456
GRID    4               0.      1.      0.              13456
GRID    5               0.      0.      -1.             12456
GRID    6               1.      0.      -1.             2456
GRID    7               1.      1.      -1.             456
GRID    8               0.      1.      -1.             1456
MAT1    1       1.+7            0.3

MPC     100     2       1       1.      7       1       -2.
MPC     100     3       1       1.      7       1       -2.
MPC     100     3       2       1.      7       2       -1.
MPC     100     4       2       1.      7       2       -1.
MPC     100     5       3       1.      7       3       -1.
MPC     100     6       1       1.      7       1       -1.
MPC     100     6       3       1.      7       3       -1.
MPC     100     8       2       1.      7       2       -1.
MPC     100     8       3       1.      7       3       -1.
NLPARM  10      48              AUTO    1                       YES
PARAM   LGDISP  1
PARAM   POST    -1
PSOLID  1       1       0       THREE   GRID    FULL    SMECH
SPC     100     7       1
SPCD    1000    7       1       6.
ENDDATA
