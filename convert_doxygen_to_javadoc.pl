#!/usr/bin/perl

use strict;

sub processFile($)
{
    my ($fileName) = @_;

    open(my $file, $fileName);

    my $newFileName = "$fileName.new";
    open(my $newFile, ">$newFileName");

    my @rows = <$file>;
    close($file);

    my $commentStarted = 0;
    my $indent;
    for my $row (@rows) {
        if (!$commentStarted && $row =~ /^(\s*)\/\/\/(.*)$/) {
            print $newFile "$1/**\n$1 *$2\n";
            $indent = $1;
            $commentStarted = 1;
            next;
        }
        if ($commentStarted) {
            if ($row !~ /^\s*\/\/\//) {
                $commentStarted = 0;
                print $newFile "$indent */\n$row";
            } else {
                $row =~ s/^(\s*)\/\/\/(.*)/\1 *\2/;
                print $newFile $row;
            }
            next;
        }

        if ($row =~ /^(\s+)(\S.*\S)\s+\/\/\/<\s+(.*)$/) {
            print $newFile "$1/**\n$1 * $3\n$1 */\n$1$2\n\n";
            next;
        }

        print $newFile $row;
    }

    close($newFile);

    rename ($newFileName, $fileName);
}

opendir(my $dir, ".");
my @files = grep { $_ =~ /\.h$/ } readdir($dir);
closedir($dir);

for my $fileName (@files) {
    processFile($fileName);
}
