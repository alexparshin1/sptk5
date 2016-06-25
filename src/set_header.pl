#!/usr/bin/perl -w

sub printHeader($$)
{
    my ($output,$file) = @_;
    printf $output "/*\n";
    printf $output "╔══════════════════════════════════════════════════════════════════════════════╗\n";
    printf $output "║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║\n";
    printf $output "║                       %-50s     ║\n", "$file - description";
    printf $output "╟──────────────────────────────────────────────────────────────────────────────╢\n";
    printf $output "║  begin                Thursday May 25 2000                                   ║\n";
    printf $output "║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║\n";
    printf $output "║  email                alexeyp\@gmail.com                                      ║\n";
    printf $output "╚══════════════════════════════════════════════════════════════════════════════╝\n";
    printf $output "┌──────────────────────────────────────────────────────────────────────────────┐\n";
    printf $output "│   This library is free software; you can redistribute it and/or modify it    │\n";
    printf $output "│   under the terms of the GNU Library General Public License as published by  │\n";
    printf $output "│   the Free Software Foundation; either version 2 of the License, or (at your │\n";
    printf $output "│   option) any later version.                                                 │\n";
    printf $output "│                                                                              │\n";
    printf $output "│   This library is distributed in the hope that it will be useful, but        │\n";
    printf $output "│   WITHOUT ANY WARRANTY; without even the implied warranty of                 │\n";
    printf $output "│   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library   │\n";
    printf $output "│   General Public License for more details.                                   │\n";
    printf $output "│                                                                              │\n";
    printf $output "│   You should have received a copy of the GNU Library General Public License  │\n";
    printf $output "│   along with this library; if not, write to the Free Software Foundation,    │\n";
    printf $output "│   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.               │\n";
    printf $output "│                                                                              │\n";
    printf $output "│   Please report all bugs and problems to alexeyp\@gmail.com.                  │\n";
    printf $output "└──────────────────────────────────────────────────────────────────────────────┘\n";
    printf $output "*/\n\n";
}

sub processFile($)
{
    my ($fileName) = @_;
    my $file;

    open(my $output, ">$fileName.new");
    printHeader($output, $fileName) || die("Can't open $fileName.new");

    open($file, $fileName) || die("Can't open $fileName");
    my $header = 1;
    while (<$file>) {
        my $row = $_;
        if ($header && $row =~ /^[\s\/\*]/) {
            next;
        } else {
            $header = 0;
            print $output $row;
        }
    }
    close $file;
    close($output);

    rename "$fileName.new", "$fileName";
}

opendir(my $dir, ".");
while (readdir $dir) {
    my $fileName = $_;
    if ($fileName !~ /(\.cpp|\.h)$/) {
        next;
    }
    print "$fileName\n";
    processFile($fileName);
}
closedir($dir);
