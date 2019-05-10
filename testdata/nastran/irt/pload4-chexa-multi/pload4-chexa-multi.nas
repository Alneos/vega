
ID LINEAR,PLOAD4-CHEXA-MULTIv4
SOL 101
TIME 5
CEND
TITLE = PRESSURE ON CHEXA
SPC = 1

DISP=ALL
SUBCASE 1
    LOAD = 1
SUBCASE 2
    LOAD = 2
SUBCASE 3
    LOAD = 3
SUBCASE 4
    LOAD = 4
SUBCASE 5
    LOAD = 5
SUBCASE 6
    LOAD = 6
    
BEGIN BULK
GRID           1              0.      0.      0.                                
GRID           2              1.      0.      0.                                
GRID           3              0.      1.      0.                                
GRID           4              1.      1.      0.                                
GRID           6              0.      0.      1.                                
GRID           8              1.      0.      1.                                
GRID          10              0.      1.      1.                                
GRID          12              1.      1.      1.                                
CHEXA         13       1       1       6       8       2       3      10+      1
+      1      12       4                                                        
$
$$ Name of Material 1 : MAT1
MAT1           1 210000.             0.3                                        
$
$ Name of Property 1 : PART_1
PSOLID         1       1       0   THREE    GRID    FULL   SMECH                

SPC1           1     123       6                                                
SPC1           1      12       1                                                
SPC1           1       2       2                                                
$

PLOAD4         1      13    100.                               1       8
PLOAD4         2      13    100.                               2      12
PLOAD4         3      13    100.                               4      10
PLOAD4         4      13    100.                               3       6
PLOAD4         5      13    100.                               1       4
PLOAD4         6      13    100.                               6      12


ENDDATA
