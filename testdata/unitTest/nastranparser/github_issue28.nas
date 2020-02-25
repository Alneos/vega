ID LINEAR,GRAV2
SOL 101
TIME 2
CEND
$
  FORCE   = ALL
  DISP    = ALL
SUBCASE 1
  LOAD    = 1
SUBCASE 2
  LOAD    = 2
SUBCASE 3
  LOAD    = 3
$
BEGIN BULK
$
GRID    1               0.0     0.0     0.0             123456
GRID    2               10.     0.0     0.0
GRID    3               11.     1.0     1.0
$
CBAR    10      1       1       2       1.0     0.0     1.0
CBAR    11      2       2       3       1.0     0.0     1.0    
$
PBAR    1       1       1.0     1.0     1.0     1.0    
PBAR    2       1       2.0     2.0     2.0     2.0 
$
MAT1    1       30.E6           .3
$
CONM2   101     3       608     .1
$
GRAV    1               386.4   1.0     0.0     0.0
GRAV    2               386.4   0.0     1.0     0.0
GRAV    3               386.4   0.0     0.0     1.0
$

CORD2R  608             0.      0.      0.      0.      0.      1.
+       1.      0.      0.




ENDDATA
