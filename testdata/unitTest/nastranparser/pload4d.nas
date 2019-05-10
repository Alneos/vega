SOL 101
TIME 5
CEND

PARAM,POST,-2
TITLE = SIMPLE TENSION
DISP = ALL
SPC = 1
SUBCASE 11
  LABEL = LOAD1
  LOAD = 3
SUBCASE 12
  LABEL = LOAD2
  LOAD = 4

BEGIN BULK
GRID           1              0.    -60.     -2.                                
GRID           2             60.    -60.     -2.                                
GRID           3            120.    -60.     -2.                                
GRID           4              0.    -30.      0.                                
GRID           5             60.    -30.      0.                                
GRID           6            120.    -30.      0.                                
GRID           7              0.     30.      0.                                
GRID           8             60.     30.      0.                                
GRID           9            120.     30.      0.                                
GRID          10              0.     60.     -2.                                
GRID          11             60.     60.     -2.                                
GRID          12            120.     60.     -2.                                
CQUAD4         1       1       1       2       5       4                        
CQUAD4         2       1       2       5       6       3                        
CQUAD4         3       1       4       5       8       7                        
CQUAD4         4       1       5       6       9       8                        
CQUAD4         5       1       7       8      11      10                        
CQUAD4         6       1       8       9      12      11                        
MAT1           47100000.             0.3                                        
PSHELL         1       4     0.5       4                                        
SPC1           1  123456       1       4       7      10                        
PLOAD4         3       1    -0.5                            THRU       6        
PLOAD4         4       4     50.     50.     50.     50.                        
ENDDATA
