$   FILENAME - DMIGSTFS.DAT
ID BAR DMIG
SOL 101
TIME 10
DIAG 8
CEND
$
TITLE = PLANER CANTILEVER BEAM
SUBTITLE = USE DMIG TO BRING IN STIFFNESS MATRIX OF ELEMENT # 5 USING SMALL FIELD
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
$   DMIG HEADER ENTRY
$
DMIG    UGSTIF  0       6       1                                              
$
$   DMIG DATA COLUMN ENTRIES
$
DMIG    UGSTIF  5       3               5       3       500039.          +000001
 +0000015       5       -250019.        6       3       -500039.         +000002
 +0000026       5       -250019.
$
DMIG    UGSTIF  5       5               5       5       166680.          +000004
 +0000046       3       250019.         6       5       83340.          
$
DMIG    UGSTIF  6       3               6       3       500039.          +000006
 +0000066       5       250019.
$
DMIG    UGSTIF  6       5               6       5       166680.                 
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
$
ENDDATA
