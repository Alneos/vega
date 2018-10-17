$
$    cbush1.dat
$
TIME 10                                                                         
SOL 108
CEND   
TITLE = VERIFICATION PROBLEM, FREQ. DEP. IMPEDANCE      BUSHVER  
SUBTITLE = SINGLE DOF, CRITICAL DAMPING, 3 EXCITATION FREQUENCIES
ECHO = BOTH                                                      
SPC = 1002                                                       
METHOD = 1                                                       
DLOAD = 1                                                        
DISP = ALL                                                       
FREQ = 10                                                        
ELFO = ALL                                                       
BEGIN BULK                                                       
$ CONVENTIONAL INPUT FOR MOUNT                                   
GRDSET, ,       ,       ,       ,       ,       ,       23456 $ PS              
$ TIE DOWN EVERYTHING EXCEPT THE 1 DOF                                          
GRID,   11,     ,       0.,     0.,     0.0 $ GROUND                            
=,      12,     =,      =,      =,      ,   $ ISOLATED DOF                      
SPC1,   1002,   123456, 11 $ GROUND                                             
CONM2,  12,     12,     ,       1.0 $ THE ISOLATED MASS                         
$
$	EID	PID	GA	GB	GO/X1	X2	X3	CID
$
CBUSH	1000	2000	11	12				0
$
PBUSH	2000	K	1.0
		B	0.0
$
PBUSHT	2000	K	2001
	        B	2002
$	
TABLED1, 2001 $ STIFFNESS TABLE                                                 
,       0.9     0.81,   1.0,    1.0,    1.1,    1.21    ENDT                    
TABLED1, 2002  $ DAMPING TABLE                                                  
,       0.9     .2864789, 1.0,  .3183099, 1.1,  .3501409 ENDT                   
$ CONVENTIONAL INPUT FOR FREQUENCY RESPONSE                                     
PARAM,  WTMASS, .0253303 $ 1/(2*PI)**2.  GIVES FN=1.0                           
DAREA,  1,      12,     1,      2. $ CAUSES UNIT DEFLECTION                     
FREQ,   10,     0.9,    1.0,    1.1 $ BRACKET THE NATURAL FREQENCY              
RLOAD1, 1,      1,      ,       ,       3                                       
TABLED1,3 $ TABLE FOR FORCE VS. FREQUENCY                                       
,       0.9,    0.81,   1.,     1.,     1.1,    1.21,ENDT $ P = K               
ENDDATA
