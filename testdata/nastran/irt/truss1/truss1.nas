$ FILENAME - TRUSS1.DAT
$
ID LINEAR,TRUSS1
SOL 101
TIME 2
CEND
TITLE = LINEAR STATICS USER’S SAMPLE INPUT FILE
SUBTITLE = TRUSS STRUCTURE
LABEL = POINT LOAD AT GRID POINT 4
LOAD = 10
SPC = 11
DISPLACEMENT = ALL
ELFORCE = ALL
ELSTRESS = ALL
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
$ PROPERTIES OF ROD ELEMENTS
$
PROD    21      22      4.      1.27
$
$ MATERIAL PROPERTIES
$
MAT1    22      30.E6           .3
$
$ POINT LOAD
$
FORCE   10      4               1000.   0.      -1.     0.
$
SPC1    11      123456  1       2
$
ENDDATA
