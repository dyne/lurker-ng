package ESort;

use 5.008;
use strict;
use warnings;

require Exporter;

our @ISA = qw(Exporter);

# This allows declaration	use ESort ':all';
our %EXPORT_TAGS = ( 'all' => [ qw(
	ESort::Writer::opendb
	ESort::Reader::opendb
	ESort::FORWARD
	Esort::BACKWARD
) ] );
our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );
our $VERSION = '1.0';

require XSLoader;
XSLoader::load('ESort', $VERSION);

$ESort::FORWARD  = $ESort::Forward  = 1;
$ESort::BACKWARD = $ESort::Backward = 2;

1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

ESort - Perl extension online external sort

=head1 SYNOPSIS

  use ESort;
  
  $db = ESort::Writer("fred");
  $db->insert("turnips");
  $db->insert("jump");
  $db->commit();
  
  $db = ESort::Reader("fred");
  $w = $db->seek("zzz", $ESort::BACKWARD);
  while ($w->advance() >= 0)
  {
    print $w->key, "\n";
  }

=head1 ABSTRACT

  ESort is a library which provides an insert-only set of keys.
  This set supports only a single writer, but allows multiple atomic
  snapshots to be taken from the active database. It is extremely fast
  at inserting multiple keys and in-order reading.

=head1 DESCRIPTION

What is ESort? -- An Online External Sort

ESort provides a very primitive database API: An insert-only set of keys.

There are very different trade-offs when compared with a standard database.

N = number of keys in the set, M = number of operations
The seeks needed to for each repeated operation in one transaction:

                   ESort           BTree            Hash
Insertion          O(1)            O(M*log(N))      O(M)
Read in sequence   O(1)            O(M)             impossible
Seek to key        O(M*log^2(N))   O(M*log(N))      O(M)
Delete             impossible      O(M*log(N))      O(M)

From this table, one can conclude that ESort allows blindly fast insertion
and sequential read, but pays in the cost to seek. This is what makes it
highly appropriate for keyword indexes.

An additional restriction is that ESort only supports a single-writer.
An additional advantage is that ESort readers get snap-shots.

=head2 EXPORT

The API has two entry points ESort::Reader and ESort::Writer.
These objects both support $db->seek(key, dir) and $db->seekp(prefix, kex, dir).
A seek returns a Walker which supports $w->key() and $w->advance().
$w->advance() returns the bytes in common with the last key or -1 on EOF.
The writer supports insert(key), commit(), rollback().

=head1 SEE ALSO

The esort.h header file for the C++ bindings.

The project is part of lurker on http://lurker.sourceforge.net/

=head1 AUTHOR

Wesley W. Terpstra, E<lt>terpstra@users.sourceforge.netE<gt>

=head1 COPYRIGHT AND LICENSE

Copyright 2003 by Wesley W. Terpstra

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

=cut
