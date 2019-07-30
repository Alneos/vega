$   FILENAME - DMIGSTFL.DAT
ID BAR DMIG
SOL 101
TIME 10
DIAG 8
CEND
TITLE = PLANER CANTILEVER BEAM
SUBTITLE = DMIG TO BRING IN STIFFNESS MATRIX FOR ELEMENT # 5 USING LARGE FIELD
K2GG = UGSTIF
SPC = 10
DISP = ALL
$
SUBCASE 1
LABEL = TIP LOAD AT END
LOAD = 10
$  
BEGIN BULK
$
CBAR    1       1       1       2       10                                      
CBAR    2       1       2       3       10                                      
CBAR    3       1       3       4       10                                      
CBAR    4       1       4       5       10                                      
$
$   DMIG HEADER ENTRY
$
DMIG    UGSTIF  0       6       2                                              
$  
$   DMIG DATA COLUMN ENTRIES
$
DMIG*    UGSTIF          5               3                              *A
*A       5               3                5.000388 D+5                  *B
*B       5               5               -2.500194 D+5                  *C
*C       6               3               -5.000388 D+5                  *D
*D       6               5               -2.500194 D+5
$
DMIG*    UGSTIF          5               5                              *A2
*A2      5               5               1.666796 D+5                   *B2
*B2      6               3               2.500194 D+5                   *C2
*C2      6               5               8.33398  D+4                      
$
DMIG*    UGSTIF          6               3                              *A3
*A3      6               3               5.000388 D+5                   *B3
*B3      6               5               2.500194 D+5
$
DMIG*    UGSTIF          6               5                              *A4
*A4      6               5               1.666796 D+5
$
FORCE   10      6               100.    0.      0.      1.                      
GRAV    20              9.8     0.      0.      1.                              
GRID    1               0.      0.      0.              246                    
GRID    2               1.      0.      0.              246                     
GRID    3               2.      0.      0.              246                     
GRID    4               3.      0.      0.              246                     
GRID    5               4.      0.      0.              246                     
GRID    6               5.      0.      0.              246                     
GRID    10              0.      0.      10.             123456                  
MAT1    1       7.1+10          .33     2700.                                   
PBAR    1       1       2.654-3 5.869-7                                         
SPC1    10      123456  1                                                       
ENDDATA
