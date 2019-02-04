$
$  FILENAME - TRUSS5.DAT
$
ID LINEAR,TRUSS5
SOL 101
TIME 2
CEND
TITLE = LINEAR STATICS USER'S GUIDE
SUBTITLE = TRUSS STRUCTURE
LOAD = 10
SPC = 11
DISPLACEMENT = ALL
$
SUBCASE 1
  LABEL = POINT LOAD AT GRID POINT 4
  LOAD = 10
$
SUBCASE 2
  LABEL = POINT LOAD AT GRID POINT 3
  LOAD = 11
  ELFORCE = ALL
$
SUBCOM 3
  SUBSEQ 1.0,0.5
  ELFORCE = ALL
SUBCOM 4
  SUBSEQ 1.0,-0.5
  ELFORCE = ALL
$
BEGIN BULK
$
$ THE GRID POINTS LOCATIONS 
$ DESCRIBE THE GEOMETRY
$
GRID    1               0.      0.      0.              3456
GRID    2               0.      120.    0.              3456
GRID    3               600.    120.    0.              3456
GRID    4               600.    0.      0.              3456
$
$ MEMBERS ARE MODELED USING
$ ROD ELEMENTS
$
CROD    1       21      2       3
CROD    2       21      2       4
CROD    3       21      1       3
CROD    4       21      1       4
CROD    5       21      3       4
$
$ PROPERTIES  OF ROD ELEMENTS
$
PROD    21      22      4.      1.27
$
$ MATERIAL PROPERTIES 
$
MAT1    22      30.E6           .3
$
SPC1    11      123456  1       2
$
$ POINT LOAD SUBCASE 1
FORCE   10      4               1000.   0.      -1.     0.
$
$
$ POINT LOAD SUBCASE 2
FORCE   11      3               2000.   1.      0.      0.
$
ENDDATA 
