$
$  FILENAME - TRUSS4.DAT
$
ID LINEAR,TRUSS4
SOL 101
TIME 2
CEND
TITLE = LINEAR STATICS USER'S GUIDE
SUBTITLE = TRUSS STRUCTURE
LOAD = 10
SPC = 11
DISPLACEMENT = ALL
SET 1  = 3,4
SET 2  = 1,2
SET 3  = 3 THRU 5
$
SUBCASE 1
  LABEL = POINT4A10
  LOAD = 10
  ELFORCE = 2
$
SUBCASE 2
  LABEL = POINT4B
  ELFORCE = 3
  DISPLACEMENT = NONE
$
SUBCASE 3
  LABEL = POINT LOAD AT GRID POINT 3
  LOAD = 11
  DISPLACEMENT = 1
  ELFORCE = 1
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
