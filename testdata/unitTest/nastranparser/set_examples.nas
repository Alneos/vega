SOL 101
CEND
$ 1. SET 1 consists of elements 1, 5, 10, 11, 13, 14, 15, 20, 22, 24, and 26.
SET 1=INCLUDE 1, 5, 10 THRU 15 EXCEPT 12, INCLUDE 20 THRU 26 BY 2
$ 2. SET 2 consists of all CTRIA3 and CQUAD4 elements except element 21.
SET 2=QUAD4 TRIA3 EXCEPT 21
$ 3. SET 10 includes all CTRIAR elements plus elements 70 through 80.
SET 10=TRIAR INCLUDE ELEMENTS 70 THRU 80
$ 4. SET 15 includes all elements from 15 to 20 and 26 to 100.
SET 15=15 THRU 100 EXCEPT 21 THRU 25
$ 5. SET 2 includes all elements except CTETRA elements.
SET 2=ALL EXCEPT TETRA
