$
$ filename - gpstress.dat
$
ID     LINEAR,gpstress
SOL    101 
TIME   5
CEND
TITLE = GPSTRESS EXAMPLE
DISP = ALL
LOAD = 1
SPC = 1
STRESS=ALL
SET 10 = 100,200
GPSTRESS = 10
STRFIELD = 10
$
OUTPUT(POST)
 SET 1 = 1,2,4,5,7,8
 SET 2 = ALL
 SURFACE 100 SET 1 NORMAL Z
 SURFACE 200 SET 2 NORMAL Z
BEGIN BULK
PARAM   POST    -1                                    
PARAM   K6ROT   0.                                      
GRID    1               0.0     -10.    0.0                                    
GRID    2               10.     -10.    0.0                                    
GRID    3               20.     -10.    0.0                                    
GRID    4               30.     -10.    0.0                                    
GRID    5               0.0     0.0     0.0                                    
GRID    6               10.     0.0     0.0                                    
GRID    7               20.     0.0     0.0                                    
GRID    8               30.     0.0     0.0                                    
GRID    9               0.0     10.     0.0                                    
GRID    10              10.     10.     0.0                                    
GRID    11              20.     10.     0.0                                    
GRID    12              30.     10.     0.0                                    
GRID    13              0.0     20.     0.0                                    
GRID    14              10.     20.     0.0                                    
GRID    15              20.     20.     0.0                                    
GRID    16              30.     20.     0.0                                    
$
CQUAD4  1       1       1       2       6       5       
CQUAD4  2       1       2       3       7       6       
CQUAD4  3       1       3       4       8       7       
CQUAD4  4       1       5       6       10      9       
CQUAD4  5       1       6       7       11      10      
CQUAD4  6       1       7       8       12      11      
CQUAD4  7       1       9       10      14      13       
CQUAD4  8       1       10      11      15      14      
CQUAD4  9       1       11      12      16      15     
$
FORCE   1       16      0       5.0     1. 
FORCE   1       12      0       10.     1. 
FORCE   1       8       0       10.     1.      
FORCE   1       4       0       5.0     1.           
$
SPC1    1       13      1       5       9       13
SPC1    1       2456    1
SPC1    1       6       1       THRU    16
$
PSHELL  1       1       .1      1                                       
$
MAT1    1       1.+7            .3                                    
ENDDATA
