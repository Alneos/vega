$ Linear Static Analysis, Database
SOL 101
TIME 600
$ Direct Text Input for Executive Control
CEND
SEALL = ALL
SUPER = ALL
TITLE = CRASH
ECHO = NONE

$ Direct Text Input for Global Case Control Data
SUBCASE 1
   SUBTITLE=TEST
   SPC = 1
   LOAD = 1
   DISPLACEMENT(PLOT,SORT1,REAL)=ALL
   SPCFORCES(PLOT,SORT1,REAL)=ALL
   STRESS(PLOT,SORT1,REAL,VONMISES,BILIN)=ALL


BEGIN BULK




$ Referenced Material Records
$ Material Record : STEEL
$ Description of Material :
PSHELL   1       1       1.5     1               1
MAT1     1       205800. 79153.8 .3      7.9-9

$ Nodes 
GRID     4117            421.993 172.078-441.45
GRID     4118            427.526 164.169-434.787
GRID     8118            435.517 169.757-434.789
GRID     8119            429.984 177.666-441.452

$ Elements
CQUAD4   5820    1       8119    8118    4118    4117    0.      0.


$ Constraint
SPC1     1       123456  4117    4118


$ Nodal Forces of Load Set : jouge
FORCE    1       8118    5       100.    0.      0.      1.
CORD2R   5               459.002 212.504-413.869 459.002 212.504 239.682
         994.595 587.028-413.869


ENDDATA ce761292
