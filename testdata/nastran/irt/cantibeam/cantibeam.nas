
$ cantibeam.mail
SOL 111
DIAG 8
TIME 10000
CEND
$
TITLE=Vega Exported Model
LABEL=Beam
METHOD=100
FREQUENCY = 100
SDAMPING = 100
$RESVEC(NOINRL,APPLOAD,NORVDOF) = YES
$RESVEC(NOINRL) = NO
RESVEC = YES
SET 111 = 2
ECHO=PUNCH
DISPLACEMENT=111
$FORCE=111
$SPCFORCE=ALL
$STRESS=ALL
SUBCASE 1
    SPC=1
    DLOAD=100

BEGIN BULK
PARAM,COUPMASS,0
PARAM,POST,-1
$---1--][---2--][---3--][---4--][---5--][---6--][---7--][---8--][---9--][--10--]
EIGRL, 100, 0.5, 70.
$---1--][---2--][---3--][---4--][---5--][---6--][---7--][---8--][---9--][--10--]

GRID           1             0.0     0.0     0.0
GRID           2            10.0     0.0     0.0
GRID           3             1.0     0.0     0.0
GRID           4             2.0     0.0     0.0
GRID           5             3.0     0.0     0.0
GRID           6             4.0     0.0     0.0
GRID           7             5.0     0.0     0.0
GRID           8             6.0     0.0     0.0
GRID           9             7.0     0.0     0.0
GRID          10             8.0     0.0     0.0
GRID          11             9.0     0.0     0.0

$---1--][---2--][---3--][---4--][---5--][---6--][---7--][---8--][---9--][--10--]

CBEAM          1       1       1       3     0.0     1.0     0.0
CBEAM          2       1       3       4     0.0     1.0     0.0
CBEAM          3       1       4       5     0.0     1.0     0.0
CBEAM          4       1       5       6     0.0     1.0     0.0
CBEAM          5       1       6       7     0.0     1.0     0.0
CBEAM          6       1       7       8     0.0     1.0     0.0
CBEAM          7       1       8       9     0.0     1.0     0.0
CBEAM          8       1       9      10     0.0     1.0     0.0
CBEAM          9       1      10      11     0.0     1.0     0.0
CBEAM         10       1      11       2     0.0     1.0     0.0

$---1--][---2--][---3--][---4--][---5--][---6--][---7--][---8--][---9--][--10--]

SPC1           1  123456       1

$---1--][---2--][---3--][---4--][---5--][---6--][---7--][---8--][---9--][--10--]

MAT1           1   2e+11             0.3  7830.0

$---1--][---2--][---3--][---4--][---5--][---6--][---7--][---8--][---9--][--10--]
PBEAML         1       1        BAR                                     +B1
+B1          0.5     1.0

$---1--][---2--][---3--][---4--][---5--][---6--][---7--][---8--][---9--][--10--]
$
$ SPECIFY MODAL DAMPING
$
TABDMP1, 100, CRIT,
+, 0., .03, 100., .03, ENDT
$
$ POINT LOAD
$
$ IF 'DAREA' CARDS ARE REFERENCED, THEN
$ 'DPHASE' AND 'DELAY' CAN BE USED
$
DAREA, 600, 2, 3, -1E8
$
TABLED1, 310,,
, 1., 1., 100., 1., ENDT
RLOAD2, 100, 600, , , 310
$
$ SPECIFY FREQUENCY STEPS
$
FREQ1, 100, 2, 3.8, 10
ENDDATA
