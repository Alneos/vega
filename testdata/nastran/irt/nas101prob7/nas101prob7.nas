ID SEMINAR, PROB7
SOL 105
TIME 600
CEND
TITLE = ELASTIC STABILITY of PLATES
SUBCASE 1
 SPC = 2
 LOAD = 2
 DISPLACEMENT=ALL
 SPCFORCES=ALL
SUBCASE 2
 SPC = 2
 METHOD = 1
 VECTOR=ALL
 SPCFORCES=ALL
BEGIN BULK
CQUAD4  1       1       1       2       13      12                              
CQUAD4  2       1       2       3       14      13                              
CQUAD4  3       1       3       4       15      14                              
CQUAD4  4       1       4       5       16      15                              
CQUAD4  5       1       5       6       17      16                              
CQUAD4  6       1       6       7       18      17                              
CQUAD4  7       1       7       8       19      18                              
CQUAD4  8       1       8       9       20      19                              
CQUAD4  9       1       9       10      21      20                              
CQUAD4  10      1       10      11      22      21                              
CQUAD4  11      1       12      13      24      23                              
CQUAD4  12      1       13      14      25      24                              
CQUAD4  13      1       14      15      26      25                              
CQUAD4  14      1       15      16      27      26                              
CQUAD4  15      1       16      17      28      27                              
CQUAD4  16      1       17      18      29      28                              
CQUAD4  17      1       18      19      30      29                              
CQUAD4  18      1       19      20      31      30                              
CQUAD4  19      1       20      21      32      31                              
CQUAD4  20      1       21      22      33      32                              
CQUAD4  21      1       23      24      35      34                              
CQUAD4  22      1       24      25      36      35                              
CQUAD4  23      1       25      26      37      36                              
CQUAD4  24      1       26      27      38      37                              
CQUAD4  25      1       27      28      39      38                              
CQUAD4  26      1       28      29      40      39                              
CQUAD4  27      1       29      30      41      40                              
CQUAD4  28      1       30      31      42      41                              
CQUAD4  29      1       31      32      43      42                              
CQUAD4  30      1       32      33      44      43                              
CQUAD4  31      1       34      35      46      45                              
CQUAD4  32      1       35      36      47      46                              
CQUAD4  33      1       36      37      48      47                              
CQUAD4  34      1       37      38      49      48                              
CQUAD4  35      1       38      39      50      49                              
CQUAD4  36      1       39      40      51      50                              
CQUAD4  37      1       40      41      52      51                              
CQUAD4  38      1       41      42      53      52                              
CQUAD4  39      1       42      43      54      53                              
CQUAD4  40      1       43      44      55      54                              
EIGRL   1                       5       0                                       
FORCE   1       11      0       1.      -1.     0.      0.                      
FORCE   1       55      0       1.      -1.     0.      0.                      
FORCE   3       22      0       2.      -1.     0.      0.                      
FORCE   3       33      0       2.      -1.     0.      0.                      
FORCE   3       44      0       2.      -1.     0.      0.                      
GRID    1               0.      0.      0.                                      
GRID    2               2.      0.      0.                                      
GRID    3               4.      0.      0.                                      
GRID    4               6.      0.      0.                                      
GRID    5               8.      0.      0.                                      
GRID    6               10.     0.      0.                                      
GRID    7               12.     0.      0.                                      
GRID    8               14.     0.      0.                                      
GRID    9               16.     0.      0.                                      
GRID    10              18.     0.      0.                                      
GRID    11              20.     0.      0.                                      
GRID    12              0.      2.      0.                                      
GRID    13              2.      2.      0.                                      
GRID    14              4.      2.      0.                                      
GRID    15              6.      2.      0.                                      
GRID    16              8.      2.      0.                                      
GRID    17              10.     2.      0.                                      
GRID    18              12.     2.      0.                                      
GRID    19              14.     2.      0.                                      
GRID    20              16.     2.      0.                                      
GRID    21              18.     2.      0.                                      
GRID    22              20.     2.      0.                                      
GRID    23              0.      4.      0.                                      
GRID    24              2.      4.      0.                                      
GRID    25              4.      4.      0.                                      
GRID    26              6.      4.      0.                                      
GRID    27              8.      4.      0.                                      
GRID    28              10.     4.      0.                                      
GRID    29              12.     4.      0.                                      
GRID    30              14.     4.      0.                                      
GRID    31              16.     4.      0.                                      
GRID    32              18.     4.      0.                                      
GRID    33              20.     4.      0.                                      
GRID    34              0.      6.      0.                                      
GRID    35              2.      6.      0.                                      
GRID    36              4.      6.      0.                                      
GRID    37              6.      6.      0.                                      
GRID    38              8.      6.      0.                                      
GRID    39              10.     6.      0.                                      
GRID    40              12.     6.      0.                                      
GRID    41              14.     6.      0.                                      
GRID    42              16.     6.      0.                                      
GRID    43              18.     6.      0.                                      
GRID    44              20.     6.      0.                                      
GRID    45              0.      8.      0.                                      
GRID    46              2.      8.      0.                                      
GRID    47              4.      8.      0.                                      
GRID    48              6.      8.      0.                                      
GRID    49              8.      8.      0.                                      
GRID    50              10.     8.      0.                                      
GRID    51              12.     8.      0.                                      
GRID    52              14.     8.      0.                                      
GRID    53              16.     8.      0.                                      
GRID    54              18.     8.      0.                                      
GRID    55              20.     8.      0.                                      
LOAD    2       1.      1.      1       1.      3                               
MAT1    1       2.9+7           .3                                              
PSHELL  1       1       .01     1               1                               
SPC1    1       123     1       12      23      34      45                      
SPC1    3       23      11      22      33      44      55                      
SPC1    4       3       1       THRU    11                                      
SPC1    4       3       45      THRU    55                                      
SPCADD  2       1       3       4                                               
ENDDATA
