
ID LINEAR,PLOAD4-CTETRA-MULTI
SOL 101
TIME 5
CEND
TITLE = PRESSURE ON CTETRA
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

BEGIN BULK
GRID           1              0.      0.      0.                                
GRID           2              3.      0.      0.                                
GRID           3              1.      1.      0.                                
GRID           4              0.      0.      1.                                
GRID           5              0.      0.     -1.                                
CTETRA        13       1       1       2       3       4
CTETRA        14       1       1       2       3       5

$$ Name of Material 1 : MAT1
MAT1           1 210000.             0.3                                        
$
$ Name of Property 1 : PART_1
PSOLID         1       1       0   THREE    GRID    FULL   SMECH                

SPC1           1     123       1       5
SPC1           1      2        2                                                
$SPC1           1       2       3                                                
$SPC1           1       3       4                                                
$

PLOAD4         1      13    100.                               1      2
PLOAD4         2      13    100.                               1      3
PLOAD4         3      13    100.                               1      4
PLOAD4         4      13    100.                               2      1


ENDDATA
