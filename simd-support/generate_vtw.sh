#!/bin/bash

echo "/* auto-generated */"
for A in VTW1 VTW2 VTWS; do
	echo "#if defined(REQ_$A)"
	for X in 1 2 4 8 16 32 64 128 256; do
		echo "#if defined(VTW_SIZE) && VTW_SIZE == $X"
		echo "#warning \"using $A with $X\""
		./generate_vtw $A $X 
		echo "#endif // VTW_SIZE == $X"
	done
	echo "#endif // REQ_$A"
done
