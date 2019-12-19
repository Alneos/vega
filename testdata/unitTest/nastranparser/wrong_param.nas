SOL    101
TIME  200
CEND
DISP = ALL
SPC = 1
$ Next parameter is wrong, to check if it is correctly handled by the parser
KKZ
SET 100 = 1,2,3
SUBCASE 1
LABEL = STATIC LOAD
LOAD = 1
SPCF = ALL
DISP = 100
