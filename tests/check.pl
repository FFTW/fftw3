#! /usr/bin/perl -w

$program = "./bench";
$default_options = "";
@failures = ();
$verbose = 0;
$paranoid = 0;
$exhaustive = 0;
$patient = 0;
$estimate = 0;
$nowisdom = 0;
$rounds=0;
$maxsize = 60000;
$maxcount = 100;
$do_1d = 0;
$do_2d = 0;
$do_random = 0;
$keepgoing = 0;

sub make_options {
    my $options = $default_options;
    $options = "--verify-rounds=$rounds $options" if $rounds;
    $options = "-o paranoid $options" if $paranoid;
    $options = "-o exhaustive $options" if $exhaustive;
    $options = "-o patient $options" if $patient;
    $options = "-o estimate $options" if $estimate;
    $options = "-o nowisdom $options" if $nowisdom;
    return $options;
}

sub do_problem {
    my $problem = shift;
    my $doablep = shift;
    my $options = &make_options;
    
    if ($doablep) {
	print "Executing \"$program $options --verify $problem\"\n" 
	    if $verbose;
	if (system("$program $options --verify $problem") != 0) {
	    print "FAILED: $problem does not verify\n";
	    exit 1 unless $keepgoing;
	    @failures = ($problem, @failures);
	}
    } else {
	print "Executing \"$program $options --can-do $problem\"\n" 
	    if $verbose;
	if (`$program $options --can-do $problem` ne "#f\n") {
	    print "FAILED: $problem is not undoable\n";
	    exit 1 unless $keepgoing;
	    @failures = ($problem, @failures);
	}
    }
}

# given geometry, try both directions and in place/out of place
sub do_geometry {
    my $geom = shift;
    my $doablep = shift;
    do_problem("if$geom", $doablep);
    do_problem("of$geom", $doablep);
    do_problem("ib$geom", $doablep);
    do_problem("ob$geom", $doablep);
}

# given size, try all transform kinds (complex, real, etc.)
sub do_size {
    my $size = shift;
    my $doablep = shift;
    do_geometry("c$size", $doablep);
    # TODO: add more 
}

sub small_1d {
    do_size (0, 0);
    for ($i = 1; $i <= 100; ++$i) {
	do_size ($i, 1);
    }
}

sub small_2d {
    do_size ("0x0", 0);
    for ($i = 1; $i <= 100; ++$i) {
	my $ub = 900/$i;
	$ub = 100 if $ub > 100;
	for ($j = 1; $j <= $ub; ++$j) {
	    do_size ("${i}x${j}", 1);
	}
    }
}

sub rand_small_factors {
    my $l = shift;
    my $n = 1;
    my $maxfactor = 13;
    my $f = int(rand($maxfactor) + 1);
    while ($n * $f < $l) {
	$n *= $f;
	$f = int(rand($maxfactor) + 1);
    };
    return $n;
}

# way too complicated...
sub one_random_test {
    my $q = int(2 + rand($maxsize));
    my $rnk = int(1 + rand(3));
    my $g = int(2 + exp(log($q) / $rnk));
    my $first = 1;
    my $sz = "";

    while ($q > 1) {
	my $r = rand_small_factors(int(rand($g) + 10));
	if ($r > 1) {
	    $sz = "${sz}x" if (!$first);
	    $first = 0;
	    $sz = "${sz}${r}";
	    $q = int($q / $r);
	    if ($g > $q) { $g = $q; }
	}
    }
    do_size($sz, 1)
}

sub random_tests {
    my $i;
    for ($i = 0; $i < $maxcount; ++$i) {
	&one_random_test;
    }
}

sub parse_arguments (@)
{
    local (@arglist) = @_;

    while (@arglist)
    {
	if ($arglist[0] eq '-v') { ++$verbose; }
	elsif ($arglist[0] eq '--verbose') { ++$verbose; }
	elsif ($arglist[0] eq '-p') { ++$paranoid; }
	elsif ($arglist[0] eq '--paranoid') { ++$paranoid; }
	elsif ($arglist[0] eq '--exhaustive') { ++$exhaustive; }
	elsif ($arglist[0] eq '--patient') { ++$patient; }
	elsif ($arglist[0] eq '--estimate') { ++$estimate; }
	elsif ($arglist[0] eq '--nowisdom') { ++$nowisdom; }
	elsif ($arglist[0] eq '-k') { ++$keepgoing; }
	elsif ($arglist[0] eq '--keep-going') { ++$keepgoing; }
	elsif ($arglist[0] =~ /^--verify-rounds=(.+)$/) { $rounds = $1; }
	elsif ($arglist[0] =~ /^--count=(.+)$/) { $count = $1; }
	elsif ($arglist[0] =~ /^-c=(.+)$/) { $count = $1; }
	
	elsif ($arglist[0] eq '-1d') { ++$do_1d; }
	elsif ($arglist[0] eq '-2d') { ++$do_2d; }
	elsif ($arglist[0] eq '-r') { ++$do_random; }
	elsif ($arglist[0] eq '--random') { ++$do_random; }
	elsif ($arglist[0] eq '-a') { 
	    ++$do_1d; ++$do_2d; ++$do_random; 
	}

	else { $program=$arglist[0]; }
	shift (@arglist);
    }
}

# MAIN PROGRAM:

&parse_arguments (@ARGV);

&small_1d if $do_1d;
&small_2d if $do_2d;
&random_tests if $do_random;

