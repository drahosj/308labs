#! /bin/bash
# pipe this script into the stdin of the print server
# ./test.sh | ./printserver -l log_file.log

## Test a few prints against black_white

echo "NEW"
echo "NAME:outputbw0.pdf"
echo "FILE:samplec.ps"
echo "DESCRIPTION:The first test file"
echo "PRINTER:black_white"
echo "PRINT"

echo "NEW"
echo "NAME:outputbw1.pdf"
echo "FILE:samplec.ps"
echo "DESCRIPTION:The first test file"
echo "PRINTER:black_white"
echo "PRINT"

echo "NEW"
echo "NAME:outputbw2.pdf"
echo "FILE:samplec.ps"
echo "DESCRIPTION:The first test file"
echo "PRINTER:black_white"
echo "PRINT"

echo "NEW"
echo "NAME:outputbw3.pdf"
echo "FILE:samplec.ps"
echo "DESCRIPTION:The first test file"
echo "PRINTER:black_white"
echo "PRINT"

echo "NEW"
echo "NAME:outputbw4.pdf"
echo "FILE:samplec.ps"
echo "DESCRIPTION:The first test file"
echo "PRINTER:black_white"
echo "PRINT"

## Test a few prints on color

echo "NEW"
echo "NAME:outputc0.pdf"
echo "FILE:samplec.ps"
echo "DESCRIPTION:The first test file"
echo "PRINTER:black_white"
echo "PRINT"

echo "NEW"
echo "NAME:outputc1.pdf"
echo "FILE:samplec.ps"
echo "DESCRIPTION:The first test file"
echo "PRINTER:black_white"
echo "PRINT"

echo "NEW"
echo "NAME:outputc2.pdf"
echo "FILE:samplec.ps"
echo "DESCRIPTION:The first test file"
echo "PRINTER:black_white"
echo "PRINT"

## Massively spam it to get a bunch of print jobs flowing

for i in $(seq 1 20)
do
	echo "NEW"
	echo "NAME:output_spam_c${i}.pdf"
	echo "FILE:samplec.ps"
	echo "DESCRIPTION:A spam file"
	echo "PRINTER:color"
	echo "PRINT"

	echo "NEW"
	echo "NAME:output_spam_bw${i}.pdf"
	echo "FILE:samplec.ps"
	echo "DESCRIPTION:A spam file"
	echo "PRINTER:black_white"
	echo "PRINT"
done

echo "EXIT"
