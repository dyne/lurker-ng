package KW;

use 5.008;
use strict;
use warnings;

require Exporter;

our @ISA = qw(Exporter);

# This allows declaration	use KW ':all';
our %EXPORT_TAGS = ( 'all' => [ qw(
	ESort::Writer::opendb
	ESort::Reader::opendb
	ESort::FORWARD
	Esort::BACKWARD
) ] );
our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );
our $VERSION = '0.02';

require XSLoader;
XSLoader::load('KW', $VERSION);

package ESort;
$FORWARD  = $Forward  = 1;
$BACKWARD = $Backward = 2;

package ESort::Writer;
our @ISA = qw(ESort::Reader);

1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

KW - Perl extension for blah blah blah

=head1 SYNOPSIS

  use KW;
  blah blah blah

=head1 ABSTRACT

  This should be the abstract for KW.
  The abstract is used when making PPD (Perl Package Description) files.
  If you don't want an ABSTRACT you should also edit Makefile.PL to
  remove the ABSTRACT_FROM option.

=head1 DESCRIPTION

Stub documentation for KW, created by h2xs. It looks like the
author of the extension was negligent enough to leave the stub
unedited.

Blah blah blah.

=head2 EXPORT

None by default.



=head1 SEE ALSO

Mention other useful documentation such as the documentation of
related modules or operating system documentation (such as man pages
in UNIX), or any relevant external documentation such as RFCs or
standards.

If you have a mailing list set up for your module, mention it here.

If you have a web site set up for your module, mention it here.

=head1 AUTHOR

Wesley W. Terpstra, E<lt>terpstra@c47.orgE<gt>

=head1 COPYRIGHT AND LICENSE

Copyright 2003 by Wesley W. Terpstra

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself. 

=cut
