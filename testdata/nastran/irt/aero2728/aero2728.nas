$ NASTRAN input file created by the MSC MSC.Nastran input file
$ Direct Text Input for Nastran System Cell Section
$ Direct Text Input for File Management Section
$ Normal Modes Analysis, Database
SOL 111
TIME 400
$ Direct Text Input for Executive Control
CEND

TITLE = FEM  MODEL
SUBTITLE = MODAL FREQUENCY RESPONSE ANALYSIS

SEALL = ALL
SUPER = ALL
ECHO = NONE
$
$ SPECIFY SPC
SPC = 3
$
$ SPECIFY MODAL EXTRACTION
METHOD = 777
$
$ SPECIFY DYNAMIC INPUT
DLOAD = 2
$ SDAMPING = 4
LOADSET = 3
FREQUENCY = 5

$ OUTPUT REQUEST

$ Frequencies for which output will be printed in frequency response
SET 123 = 61629341	
$ SET 123 = 61629341,61626080,61625305	
DISPLACEMENT(PHASE,PLOT)=123
$
$ XYPLOTS
$
$... X-Y plot commands ...
$

OUTPUT(XYPLOT)
$ XTGRID: Controls the drawing of the grid lines parallel to the x-axis at the y-axis tic marks on upper half frame
$ curves only.
XTGRID = YES
YTGRID = YES
$ XBGRID: Controls the drawing of the grid lines parallel to the x-axis at the y-axis tic marks on lower half frame
$ curves only.
XBGRID = YES
YBGRID = YES
$
$ PLOT RESULTS
XTITLE = FREQUENCY
$
$ YTLOG: Selects logarithmic or linear y-axis for upper half frame curves only. YES - > Plot a logarithmic y-axis. NO Plot a linear y-axis
$ T2RM, Ti stands for the i-th translational component, RM means real or magnitude, IP means imaginary or phase

YTLOG = YES
YTTITLE = DISPL. MAG. 61629341
YBTITLE = DISPL. PHASE 61629341
XYPLOT,XYPUNCH,DISP /61629341(T1RM,T1IP)

YTTITLE = DISPL. MAG. 61629341
YBTITLE = DISPL. PHASE 61629341
XYPLOT,XYPUNCH,DISP /61629341(T2RM,T2IP)

YTTITLE = DISPL. MAG. 61629341
YBTITLE = DISPL. PHASE 61629341
XYPLOT,XYPUNCH,DISP /61629341(T3RM,T3IP)




   
BEGIN BULK
$
$.......2.......3.......4.......5.......6.......7.......8.......9.......10.....$
$
$
$ NORMAL MODES TO 200 HZ
$EIGRL  SID     V1      V2
EIGRL   777     -0.1       1000.

$ EXCITATION FREQUENCY DEFINITION 0 TO 140 HZ
$FREQ1   SID     F1      DF     NDF
FREQ1    5       0.0     0.2    800




$ DYNAMIC LOADING
$RLOAD1 SID      DAREA    DELAY  DPHASE  TB       TP
					
RLOAD1   2      61629341                 22 
$
$LSEQ   SID      DAREA    LID     TID
LSEQ     3      61629341   1

$TABLED1 TID 
																	
$+TABL1  X1      Y1      X2      Y2      ETC.
TABLED1 22                                                               
        0.0      1.0     1000.0   1.0     ENDT

$$$  LOAD  $$$

$ Loads for Load Case : DWD
$LOAD     2      1.      -9.      1
$ Loads for Load Case : FWD
$LOAD     4      1.      -9.      3
$ Loads for Load Case : SWD
$LOAD     6      1.      3.       7
$ Loads for Load Case : '-SWD
$LOAD     8      1.      -3.      7


$ Gravity Loading of Load Set : gx
$GRAV     3       0      9810.   1.       0.      0.
$ Gravity Loading of Load Set : gy
$GRAV     7       0      9810.    0.     1.       0.
$ Gravity Loading of Load Set : gz
$GRAV     1       0      9810.    0.      0.     1.

$ Gravity Loading of Load Set : DWD with gx, 9810 X -9 = -88290
GRAV     1       0      -88290.   1.       0.      0.

	

$ Direct Text Input for Bulk Data
include 'Bulk_DFEM.bdf'



ENDDATA e86af801
