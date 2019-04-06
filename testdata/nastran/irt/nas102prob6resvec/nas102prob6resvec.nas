ID SEMINAR, PROB6
SOL 111
CEND
TITLE = FREQUENCY RESPONSE WITH PRESSURE AND POINT LOADS
SUBTITLE = USING THE MODAL METHOD WITH LANCZOS
ECHO = UNSORTED
SEALL = ALL
SPC = 1
$RESVEC(NOINRL,APPLOAD,NORVDOF) = YES
RESVEC = YES
SET 111 = 11, 33, 55
DISPLACEMENT(PHASE, PLOT) = 111
METHOD = 100
FREQUENCY = 100
SDAMPING = 100
SUBCASE 1
DLOAD = 100
$
OUTPUT (XYPLOT)
$
XTGRID= YES
YTGRID= YES
XBGRID= YES
YBGRID= YES
YTLOG= YES
YBLOG= NO
XTITLE= FREQUENCY (HZ)
YTTITLE= DISPLACEMENT RESPONSE AT LOADED CORNER, MAGNITUDE
YBTITLE= DISPLACEMENT RESPONSE AT LOADED CORNER, PHASE
XYPLOT DISP RESPONSE / 11 (T3RM, T3IP)
YTTITLE= DISPLACEMENT RESPONSE AT TIP CENTER, MAGNITUDE
YBTITLE= DISPLACEMENT RESPONSE AT TIP CENTER, PHASE
XYPLOT DISP RESPONSE / 33 (T3RM, T3IP)
YTTITLE= DISPLACEMENT RESPONSE AT OPPOSITE CORNER, MAGNITUDE
YBTITLE= DISPLACEMENT RESPONSE AT OPPOSITE CORNER, PHASE
XYPLOT DISP RESPONSE / 55 (T3RM, T3IP)
$
BEGIN BULK
PARAM,COUPMASS,1
PARAM,WTMASS,0.00259
$
$ PLATE MODEL DESCRIBED IN NORMAL MODES EXAMPLE
$
INCLUDE 'plate.bdf'
$
$ EIGENVALUE EXTRACTION PARAMETERS
$
EIGRL, 100, 10., 2000.
$
$ SPECIFY MODAL DAMPING
$
TABDMP1, 100, CRIT,
+, 0., .03, 10., .03, ENDT
$
$ APPLY PRESSURE LOAD
$
PLOAD2, 300, 1., 1, THRU, 40
RLOAD2, 400, 300, , ,310
$
TABLED1, 310,,
, 10., 1., 1000., 1., ENDT
$
$ POINT LOAD
$
$ IF 'DAREA' CARDS ARE REFERENCED, THEN
$ 'DPHASE' AND 'DELAY' CAN BE USED
$
DAREA, 600, 11, 3, 1.0
$
RLOAD2, 500, 600, , 320, 310
$
DPHASE, 320, 11, 3, -45.
$
$
$ COMBINE LOADS
$
DLOAD, 100, 1., .1, 400, 1.0, 500
$
$ SPECIFY FREQUENCY STEPS
$
FREQ1, 100, 20., 20., 49
$FREQ4, 100, 20., 1000., .03, 5
$
ENDDATA
