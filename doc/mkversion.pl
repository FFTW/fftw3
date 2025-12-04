#!/usr/bin/env perl
use strict;
use locale;
use POSIX;

setlocale("LC_ALL", "C");

my $fmt = "\@set UPDATED %d %B %Y
\@set UPDATED-MONTH %B %Y
\@set EDITION @ARGV[1]
\@set VERSION @ARGV[1]
";

my $stat = (stat(@ARGV[0]))[9];
print (strftime($fmt, localtime($stat)))
