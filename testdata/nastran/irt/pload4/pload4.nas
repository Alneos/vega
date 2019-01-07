ID     LINEAR,PLOAD4
SOL    101 
TIME   5
CEND
SPC = 1
TITLE = PLOAD4 Example 2
SUBCASE 1
  LOAD = 1
BEGIN BULK
$
GRID    1               0.0     0.0     0.0                                    
GRID    2               1.      0.0     0.0                                    
GRID    3               0.0     1.      0.0                                    
GRID    4               1.      1.      0.0                                    
GRID    5               0.0     0.0     1.                                     
GRID    6               1.      0.0     1.                                     
GRID    7               0.0     1.      1.                                     
GRID    8               1.      1.      1.
CHEXA   100     1       1       5       6       2       3       7       +
+       8       4
SPC     1       1       123     0.0
SPC     1       2       23      0.0
SPC     1       3       13      0.0     
SPC     1       4       3       0.0
$23456781234567812345678123456781234567812345678123456781234567812345678
PLOAD4  1       100     26.4                            5       8
PSOLID  1       1       0               GRID            SMECH   
$
MAT1    1       3.+7            .3                                                                      
ENDDATA
