#!/usr/bin/perl
#
# 1. Run this with the name of the new module
# 2. edit SurgeXT.h and .cpp to add the model extern and add model
# 3. edit plugin.json to add the module
#
# voila


$fn = $ARGV[0];
$ns = lc $fn;
print $fn . " " . $ns . "\n";


open( IN, "< Blank12.h") || die "Cant open h";
open( OUT, "> ../../src/${fn}.h") || die "Cant open output";
while(<IN>)
{
   s/blank12/$ns/g;
   s/BLANK12/$fn/g;
   print OUT;
}

close(IN);
close(OUT);

open( IN, "< Blank12.cpp") || die "Cant open h";
open( OUT, "> ../../src/${fn}.cpp") || die "Cant open output";
while(<IN>)
{
   s/blank12/$ns/g;
   s/BLANK12/$fn/g;
   print OUT;
}

close(IN);
close(OUT);


