#! /usr/bin/perl -w

$program = "./bench";
$options = "-o nowisdom";
@failures = ();
$quiet = 0;

sub do_problem {
    my $problem = shift;

    print "Trying $problem\n" unless $quiet;

    if (system("$program $options --verify $problem") != 0) {
	print "FAILED: $problem\n";
	@failures = ($problem, @failures);
    }
}

# given geometry, try both directions and in place/out of place
sub do_geometry {
    my $geom = shift;
    do_problem("if$geom");
    do_problem("of$geom");
    do_problem("ib$geom");
    do_problem("ob$geom");
}

# given size, try all transform kinds (complex, real, etc.)
sub do_size {
    my $size = shift;
    do_geometry("c$size");
    # TODO: add more 
}

sub small_integers {
    for ($i = 0; $i <= 100; ++$i) {
	do_size $i;
    }
}

small_integers;

