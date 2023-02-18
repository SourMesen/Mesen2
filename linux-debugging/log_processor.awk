#!/usr/bin/gawk -f

BEGIN {
	cmd = "llvm-symbolizer --output-style GNU -Ce bin/linux-x64/Release/MesenCore.so" addr
}

$4 ~ /MesenCore\.so/ {
	match($4, /0x[0-9A-Fa-f]+/)
	ofs = substr($4, RSTART, RLENGTH)
	print ofs |& cmd
	cmd |& getline func_name
	cmd |& getline src

	$2 = func_name
	$3 = src
}

{ print }
