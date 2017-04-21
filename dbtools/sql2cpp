#!/usr/bin/perl

############################################################################
#                STORED PROCEDURE TO C++ INTERFACE GENERATOR
#                               sql2cpp.pl
#                            -------------------
#   begin        : Tue Feb 27 2007
#   copyright    : (C) 2007-2008 by Alexey Parshin. All rights reserved.
#
#            This module creation was sponsored by Total Knowledge
#                     (http://www.total-knowledge.com).
#              Author thanks the developers of CPPSERV project
#               (http://www.total-knowledge.com/progs/cppserv)
#               for defining the requirements for this module.
############################################################################

############################################################################
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#
#  * Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#  * Neither the name of the <ORGANIZATION> nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
#  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
############################################################################

use Time::Local;

my @dayofweek = (qw(Sunday Monday Tuesday Wednesday Thursday Friday Saturday));
my @monthnames = (qw(Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec));
my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday);
#$ENV{TZ} = ':/usr/share/zoneinfo/Australia/Melbourne';
($sec,$min,$hour,$mday,$mon,$year,$wday,$yday) = localtime();
$year += 1900;
$currentDate = "$dayofweek[$wday], $monthnames[$mon] $mday, $year";

my %complexTypes = ();
my %configuration = ();
my %options = ();

sub trim {
    $string = shift;
    $string =~ s/^[\s\t]*//;
    $string =~ s/[\s\t]*$//;
    return $string;
}

sub functionNameToMethodName {
    local (@nameParts,$namePart,$methodName,$first);
    @nameParts = split(/_/,$_[0]);
    $first = 1;
    foreach $namePart (@nameParts) {
        if ($first) {
            $methodName .= lc($namePart);
            $first = 0;
        } else {
            $methodName .= ucfirst(lc($namePart));
        }
    }
    $methodName;
}

# Reads the database-specific complex types (table- and view- record types).
# All the necessary information is obtained from configuration map
# Parameters: none
sub readComplexTypes {
    local ($name,$type,$class,@classNames,$recordClassTemplate);

    $recordClassTemplate = $configuration{'typemapping:recordclasstemplate'};
    @classNames = split( /\s*,\s*/, $configuration{'typemapping:recordtypes'} );

    %complexTypes = ();
    for $name (@classNames) {
        $type = $recordClassTemplate;
        @typeReplacement = split(/=/,$name);
        if ($typeReplacement[1]) {
            $name = trim($typeReplacement[0]);
            $className = trim($typeReplacement[1]);
        } else {
            $className = ucfirst(functionNameToMethodName($name));
        }
        $type =~ s/\@recordtype\@/$className/g;
        $type =~ s/\[recordtype\]/$className/g;
        $complexTypes{lc($name)} = $type;
    }
    $complexTypes{'bytea'} = "sptk::CBuffer";
}


sub isComplexType {
    local ($class,$type);

    $type = $_[0];
    $class = $complexTypes{lc($type)};

    return defined($class);
}

# Reads the program configuration from the file.
# Parameters:
#   input_file:  string
sub readConfiguration {
    local ($fileName,$line,@lines,$lineNumber);
    local ($section,$ident,$value,$comment);
    local ($today,$templateFileName,@temp);
    local ($listStarted,$listStr);

    $listStarted = 0;
    %configuration = ();

    $fileName = $_[0];
    open(INFILE,$fileName) || die ("Can't read configuration file '$fileName'. Please use the config file \$prefix/share/sptk/doc/sql2cpp.conf as example.");
    @lines = <INFILE>;
    close(INFILE);
    chomp(@lines);

    $section = '';
    $lineNumber = 0;
    for $line (@lines) {
        $lineNumber++;
        if ($listStarted == 1) {
            $value = $line;
            $value =~ s/^\s*(.*)[\n\r]*$/\1/;
            if ( $value =~ /}/ ) {
                $value =~ s/\s*}.*//;
                $listStr .= $value;
                $configuration{lc("$section:$ident")} = $listStr;
                $listStarted = 0;
            } else {
                $listStr .= $value;
            }
        }
        elsif ( $line =~ /^\s+#/ ) {
            # Comment line
            $comment = $line;
        }
        elsif ( $line =~ /^\[.*\]/ ) {
            # Section header
            $section = $line;
            $section =~ s/\[(.*)\]\s*$/\1/;
        }
        elsif ($line =~ /=/) {
            $pos = index($line,'=');
            if ($pos < 0) {
                die ("Syntax error in configuration file $fileName($lineNumber): the $ident doesn't have value");
            }
            $ident = trim(substr($line,0,$pos));
            $value = trim(substr($line,$pos+1));
            if ($section eq '') {
                die ("Syntax error in configuration file $fileName($lineNumber): the $ident value is outside the section");
            }
            if ($ident eq '') {
                die ("Syntax error in configuration file $fileName($lineNumber): a value w/o a name");
            }
            if ($listStarted == 0 && $value =~ /{/) {
                $value =~ s/{//;
                $value =~ s/^\s*//;
                $listStarted = 1;
                $listStr = $value;
                if ( $listStr =~ /}/ ) {
                    $listStr =~ s/\s*}.*//;
                    $configuration{lc("$section:$ident")} = $listStr;
                    $listStarted = 0;
                }
            } else {
                $configuration{lc("$section:$ident")} = $value;
            }
        }
    }

    @temp = ();
    $templateFileName = $configuration{"cpp:fileheader"};
    open(FHTFILE,$templateFileName) || die ("C++ file header template file ($templateFileName) cannot be opened: $!");
    @lines = <FHTFILE>;
    close(FHTFILE);
    chomp(@lines);

    $headerTemplate = join("\n",@lines) . "\n";
    $headerTemplate =~ s/\@date\@/$currentDate/g;
    $headerTemplate =~ s/\[date\]/$currentDate/g;
    $extraIncludes = "#include " . $configuration{'cpp:extraincludes'} . "\n";
    $extraIncludes =~ s/,/\n#include /g;

    $indentString = $configuration{'cpp:indentstring'};
    $indentString =~ s/\"//g;

    readComplexTypes();
}


sub typeSubstitution {
    local($ptype,$tname);

    $ptype = lc($_[0]);
    $ptype =~ s/timestamp/sptk::CDateTime/g;
    $ptype =~ s/date/sptk::CDateTime/g;
    $ptype =~ s/decimal\(..,.\)/double/g;
    $ptype =~ s/varchar\(..\)/std::string/g;
    $ptype =~ s/varchar/std::string/g;
    $ptype =~ s/text/std::string/g;
    $ptype =~ s/bytea/sptk::CBuffer&/g;
    $ptype =~ s/boolean/bool/g;
    $ptype =~ s/int4/int32_t/g;
    $ptype =~ s/int8/uint64_t/g;
    if ($ptype =~ /\@param/) {
        $ptype =~ s/\stext\,/ std::string\,/g;
    } else {
        $ptype =~ s/\stext\,/ std::string\,/g;
        $ptype =~ s/\stext\)/ std::string\)/g;
        $ptype =~ s/\stext$/ std::string/g;
    }
    $ptype =~ s/ \(out\)/&/g;

    $ptype;
}

# Reads text starting from "RETURNS" and ending with " AS"
sub processReturnText
{
    local ($returnText,$returnType,$returnIsSetOf,$returnIsComplexType,$complexType);

    $returnText = $_[0];

    $returnIsSetOf = 0;
    $returnIsComplexType = 0;

    $pos1 = index(lc($returnText),"setof ");
    if ($pos1 >= 0) {
        $returnIsSetOf = 1;
        $returnText = substr($returnText,$pos1+6);
    }

    $returnText =~ s/ .*$//;

    $complexType = $complexTypes{lc($returnText)};
    if ($complexType) {
        $returnType = $complexType;
        $returnIsComplexType = 1;
    } else {
        $returnType = typeSubstitution($returnText);
    }

    return $returnType,$returnIsSetOf,$returnIsComplexType;
}

sub processFunction {
    local($func,$returns,$params,$functionName,$methodName,$className,@funcData);
    local(@parameterParts,@parameters,@parameterNames,@parameterTypes,@parameterAlwaysValues);
    local(@isOutputParameter,$outputParameterCount);
    local(@queryInitialization,$outputParam,$outputParamName,$i,$j);
    local($functionNameCount,$returnIsSetOf,$returnIsComplexType,$complexType);
    local($prototypeHeader,$prototypeImplementation);
    local($pname,$pvalue,$inlineSQL,$classNameTemplate);

    ($func,$returns,$className,$returnIsSetOf,$returnIsComplexType,$inlineSQL) = @_;
    $outputParameterCount = 0;

    $lastRow = pop(@functionPrototypes);
    if ($lastRow =~ / \@returns /) {
        $returnInfo = $lastRow;
    } else {
        $returnInfo = "";
        if (length($lastRow) > 6) {
            push(@functionPrototypes,$lastRow);
        }
    }

    #$returns = typeSubstitution($returns);

    $pos1 = index($func,"(");
    $functionName = substr($func,0,$pos1);
    $methodName = functionNameToMethodName($functionName);
    $pos2 = rindex($func,")");
    if ($pos2 > 0) {
        $params = substr($func,$pos1+1,$pos2-$pos1-1);
    } else {
        $params = substr($func,$pos+1,16384);
    }

    $params = typeSubstitution($params);

    $functionNames{$functionName}++;
    $functionNameCount = $functionNames{$functionName};
    $queryName = "qry" . ucfirst($methodName);

    if ($functionNameCount > 1) {
        $queryName .= $functionNameCount;
    }

    if ($configuration{'print:verbose'} > 1) {
        print "Function: $functionName\n";
    }

    @parameters = split(/,+/,$params);

    $prototypeHeader = "$methodName(";
    $prototypeImplementation = "$methodName(";
    $queryInitialization = "$queryName(db,\"SELECT $functionName(";
    $firstPrototypeParameter = 1;
    $firstQueryParameter = 1;

    foreach $parameterData (@functionData) {
        $parameterData =~ s/, *$//;
        @parameterInfo = split(/[, ] *--/,$parameterData);
        @parameterParts = split(/ +/,$parameterInfo[0]);
        if ($parameterParts[0] eq '') {
            shift(@parameterParts);
        }

        $outputParam = 0;
        if (lc($parameterParts[0]) eq 'out') {
            shift(@parameterParts);
            $outputParam = 1;
            $outputParameterCount++;
        }

        $complexType = $complexTypes{lc($parameterParts[1])};
        if ($complexType) {
            $parameterParts[1] = $complexType;
        } else {
            $parameterParts[1] = typeSubstitution($parameterParts[1]);
        }

        push(@isOutputParameter,$outputParam);
        push(@parameterNames,$parameterParts[0]);
        push(@parameterTypes,$parameterParts[1]);

        if (length($parameterInfo[1]) > 0) {
            @parameterDetails = split (/\@default /, $parameterInfo[1]);

            if ( length( $parameterDetails[1] ) > 0 ) {
                push(@parameterDetails,"default");
            } else {
                @parameterDetails = split (/\@always /, $parameterInfo[1]);
                if ( length( $parameterDetails[1] ) > 0 ) {
                    push(@parameterDetails,"always");
                }
            }

            if ($parameterDetails[2] eq "always") {
                push(@parameterAlwaysValues,$parameterDetails[1]);
            } else {
                push(@functionPrototypes,"$indentString/// \@param $parameterParts[0] $parameterParts[1], $parameterDetails[0]\n");
                push(@parameterAlwaysValues,"");
            }
        } else {
            print stderr " Not documented: $parameterParts[0] : $parameterParts[1]\n";
            exit(1);
        }

        if ($outputParam)
        {
            if ( $parameterParts[1] !~ /&/ )
            {
                $parameterParts[1] = "$parameterParts[1]&";
            }
        } else {
            if ( ($parameterParts[1] eq "std::string") && (length($parameterDetails[2]) < 1) )
            {
                $parameterParts[1] = "const $parameterParts[1]&";
            }
        }

        if ($parameterDetails[2] ne "always" )
        {
            if ($firstPrototypeParameter)
            {
                $prototypeHeader .= "$parameterParts[1] $parameterParts[0]";
                $prototypeImplementation .= "$parameterParts[1] $parameterParts[0]";
                $firstPrototypeParameter = 0;
            } else {
                $prototypeHeader .= ", $parameterParts[1] $parameterParts[0]";
                $prototypeImplementation .= ", $parameterParts[1] $parameterParts[0]";
            }
        }

        if ($outputParam == 0) {
            if ($firstQueryParameter) {
                $queryInitialization .= ":$parameterParts[0]";
                $firstQueryParameter = 0;
            } else {
                $queryInitialization .= ",:$parameterParts[0]";
            }
        }

        if ($parameterDetails[2] eq "default" ) {
            $prototypeHeader .= "=$parameterDetails[1]";
        }

        $pname = ucfirst($parameterParts[0]);
        $pname =~ tr/_/ /;
    }

    # If we have one or more output parameters, use 'SELECT * FROM' form
    if ($outputParameterCount > 1) {
        $queryInitialization =~ s/SELECT /SELECT * FROM /;
    }

    if (length($inlineSQL) > 0) {
        $queryInitialization = "$queryName(db,\"$inlineSQL";
    }

    if ($returnIsComplexType == 1) {
        if ($outputParameterCount > 1) {
            die ("Function $functionName has both complex return type and output parameters");
        }

        if ($outputParameterCount == 1) {
            $outputParameterType = pop(@parameterTypes);
            $outputParameterName = pop(@parameterNames);
            pop(@isOutputParameter);
            $prototypeHeader =~ s/,$outputParameterType& $outputParameterName//;
            $prototypeHeader =~ s/$outputParameterType& $outputParameterName//;
            $prototypeImplementation =~ s/,$outputParameterType& $outputParameterName//;
            $prototypeImplementation =~ s/$outputParameterType& $outputParameterName//;
        }

        if ($returnIsSetOf) {
            $classNameTemplate = $configuration{"cpp:querywrapperclasstemplate"};
            $classNameTemplate =~ s/\@type\@/$returns/g;
            $classNameTemplate =~ s/\[type\]/$returns/g;
            $returns = $classNameTemplate;
        } else {
            $outputParameterCount++;
            push(@isOutputParameter,1);
            if ($returns =~ "sptk::CBuffer") {
                $parameterName = "buffer";
            } else {
                $parameterName = "record";
            }

            push(@parameterNames,$parameterName);
            push(@parameterTypes,$returns);
            push(@functionPrototypes,"$indentString/// \@param $parameterName $returns&, output data\n");

            if ($firstPrototypeParameter) {
                $prototypeHeader .= "$returns& $parameterName";
                $prototypeImplementation .= "$returns& $parameterName";
                $firstPrototypeParameter = 0;
            } else {
                $prototypeHeader .= ", $returns& $parameterName";
                $prototypeImplementation .= ", $returns& $parameterName";
            }

            $returns = "void";
        }
        $queryInitialization =~ s/SELECT $functionName/SELECT \* FROM $functionName/;

    } else {

        if ($returnIsSetOf) {
            $outputParameterCount++;
            push(@isOutputParameter,1);

            $parameterName = "result";
            $returns = "std::vector<" . $returns . ">";

            push(@parameterNames,$parameterName);
            push(@parameterTypes,$returns);
            push(@functionPrototypes,"$indentString/// \@param $parameterName $returns&, output data\n");

            if ($firstPrototypeParameter) {
                $prototypeHeader .= "$returns& $parameterName";
                $prototypeImplementation .= "$returns& $parameterName";
                $firstPrototypeParameter = 0;
            } else {
                $prototypeHeader .= ", $returns& $parameterName";
                $prototypeImplementation .= ", $returns& $parameterName";
            }

            $returns = "void";
            $queryInitialization =~ s/SELECT $functionName/SELECT \* FROM $functionName/;
        }
    }

    push(@functionPrototypes,$returnInfo);
    #exit(0);

    $prototypeHeader .= ")";
    $prototypeImplementation .= ")";
    if (length($inlineSQL) < 1) {
        $queryInitialization .= ")";
    }

    $queryInitialization .= "\",__FILE__,__LINE__)";
    if ($outputParameterCount > 0) {
        $returns = "void";
    }
    push(@functionPrototypes,$indentString . "$returns $prototypeHeader;\n");

    $len = length($indentString . "sptk::CQuery $queryName;");
    if ($len < 60) {
        $len = 60;
    }
    $queryDefinition = $indentString . substr("sptk::CQuery $queryName;                                          ",0,$len);
    push(@queryDefinitions,"$queryDefinition ///< Query for $functionName stored procedure\n");

    $lastQueryInit = $#queryInitializations;
    if ($lastQueryInit >= 0) {
        @queryInitializations[$lastQueryInit] .= ",\n";
    }
    push(@queryInitializations," $queryInitialization");

    push(@functionBodies,"$returns $className\:\:$prototypeImplementation\n{\n");
    if ($returns eq "void" && $outputParameterCount == 0) {
        $i = 0;
        $j = 0;
        foreach $pname (@parameterNames) {
            $pvalue = $pname;
            if ($parameterAlwaysValues[$j]) {
                $pvalue = $parameterAlwaysValues[$j];
            }
            if ($parameterTypes[$j] eq "bool") {
                push(@functionBodies,$indentString . "$queryName.param(\"$pname\").setBool($pvalue);\n");
            } else {
                if ($parameterTypes[$j] eq "std::string") {
                    push(@functionBodies,$indentString . "$queryName.param(\"$pname\").setExternalString($pvalue);\n");
                } else {
                    push(@functionBodies,$indentString . "$queryName.param(\"$pname\") = $pvalue;\n");
                }
            }
            $j++;
        }
        push(@functionBodies,$indentString . "$queryName.exec();\n}\n\n");
    } else {
        if ($returnIsSetOf && $returnIsComplexType) {
            push(@functionBodies,$indentString . "$returns RecordSet($queryName);\n");
        } else {
            push(@functionBodies,$indentString . "sptk::CQueryGuard query($queryName);\n");
        }
        $i = 0;
        $j = 0;
        foreach $pname (@parameterNames) {
            $pvalue = $pname;
            if ($parameterAlwaysValues[$j]) {
                $pvalue = $parameterAlwaysValues[$j];
            }

            if ($isOutputParameter[$i] == 0) {
                if ($returnIsSetOf && $returnIsComplexType) {
                    $queryObject = "RecordSet";
                } else {
                    $queryObject = "query";
                }

                if ($parameterTypes[$j] eq "bool") {
                    push(@functionBodies,$indentString . "$queryObject.param(\"$pname\").setBool($pvalue);\n");
                } else {
                    if ($parameterTypes[$j] eq "std::string") {
                        push(@functionBodies,$indentString . "$queryObject.param(\"$pname\").setExternalString($pvalue);\n");
                } else {
                    push(@functionBodies,$indentString . "$queryObject.param(\"$pname\") = $pvalue;\n");
                }
            }
            $i++;
        }
        $j++;
    }

    if ($returnIsSetOf && $returnIsComplexType) {
        push(@functionBodies,$indentString . "return RecordSet;\n}\n\n");
    } else {
        push(@functionBodies,$indentString . "query.open();\n");
        if ($outputParameterCount == 0) {
            push(@functionBodies,$indentString . "return query[uint32_t(0)];\n}\n\n");
        } else {
            $i = 0;
            $j = 0;
            foreach $outputParamName (@parameterNames) {
                $outputParameterType = $parameterTypes[$i];
                if ($isOutputParameter[$i] == 1) {
                    if ($returnIsComplexType) {
                        if ($returnIsSetOf) {
                            push(@functionBodies,$indentString . "query.close();\n");
                        } else {
                            if ($outputParameterType =~ "sptk::CBuffer") {
                                push(@functionBodies,$indentString . "$outputParamName.set(query[uint32_t(0)].getBuffer(),query[uint32_t(0)].dataSize());\n");
                            } else {
                                push(@functionBodies,$indentString . "$outputParamName.load(query);\n");
                            }
                        }
                    } else {
                        if ($returnIsSetOf) {
                            $hostVariableType = $parameterTypes[$i];
                            $hostVariableType =~ s/std::vector<(.*)>/\1/;
                            push(@functionBodies,$indentString . "$hostVariableType hostVariable;\n");
                            push(@functionBodies,$indentString . "result.clear();\n");
                            push(@functionBodies,$indentString . "while (!query.eof()) {\n");
                            if ($parameterTypes[$i] =~ /string/) {
                                push(@functionBodies,$indentString . $indentString . "hostVariable = query[uint32_t($j)].asString();\n");
                            } else {
                                push(@functionBodies,$indentString . $indentString . "hostVariable = query[uint32_t($j)];\n");
                            }
                            push(@functionBodies,$indentString . $indentString . "result.push_back(hostVariable);\n");
                            push(@functionBodies,$indentString . $indentString . "query.fetch();\n");
                            push(@functionBodies,$indentString . "}\n");
                        } else {
                            if ($parameterTypes[$i] =~ /string/) {
                                push(@functionBodies,$indentString . "$outputParamName = query[uint32_t($j)].asString();\n");
                            } else {
                                push(@functionBodies,$indentString . "$outputParamName = query[uint32_t($j)];\n");
                            }
                        }
                    }
                    $j++;
                }
                $i++;
            }
            push(@functionBodies,"}\n\n");
        }
        }
    }
}

sub processDoxygenComment
{
    local($comment,$directive);
    $comment = typeSubstitution($_[0]);
    if ($comment =~ /\@brief/) {
        $directive = "\@brief ";
    } else {
        if ($comment =~ /\@param/) {
            $directive = "\@param ";
        } else {
            if ($comment =~ /\@returns/) {
                $directive = "\@returns";
            }
            if ($comment =~ /^-- /) {
                $directive = "";
            }
        }
    }
    if ($directive ne "\@param ") {
        $comment =~ s/-- $directive//;
        $comment = "/// $directive" . ucfirst($comment);
    } else {
        $comment =~ s/-- $directive//;
        $comment = "/// $directive" . $comment;
    }
    $comment;
}

sub externalCommand {
    local ($command,$sourceFile,$className);
    ($command,$sourceFile,$className) = @_;

    $command =~ s/\@filename\@/$sourceFile.sql/g;
    $command =~ s/\@classname\@/$className/g;
    $command =~ s/\@generator\@/sql2cpp.pl/g;
    $command =~ s/\[filename\]/$sourceFile.sql/g;
    $command =~ s/\[classname\]/$className/g;
    $command =~ s/\[generator\]/sql2cpp.pl/g;
    if ($configuration{'print:verbose'} < 3) {
        $command .= " 1> /dev/null";
    }
    system($command);
}

sub processFile {
    local ($sourceFile,$className,$outputDir,$functionPrototypeCount,@returnArgs);
    local ($returnType,$returnIsSetOf,$returnIsComplexType,$inlineSQL);
    local ($line,@header,@module, $cppExtention, $hppExtention);
    local ($preprocessCommand,$postprocessCommand);

    ($sourceFile,$className) = @_;

    $preprocessCommand = $configuration{'commands:preprocess'};
    if ($preprocessCommand) {
        externalCommand($preprocessCommand,$sourceFile,$className);
    }

    @queryDefinitions = ();
    @functionPrototypes = ();
    @functionBodies = ();
    @queryInitializations = ();

    open(IN,"$sourceFile.sql") || die ("Can't open file $sourceFile");
    @file = <IN>;

    chomp(@file);

    if ($className eq "") {
        $className = "CTestClass";
    }

    $objectName = $sourceFile;

    $cppExtention = $configuration{'cpp:cppextention'};
    if ($cppExtention eq '') { $extention = '.cpp'; }

    $hppExtention = $configuration{'cpp:hppextention'};
    if ($hppExtention eq '') { $extention = '.h'; }

    $line = $headerTemplate;
    $line =~ s/\@filename\@/$className.$hppExtention/g;
    $line =~ s/\[filename\]/$className.$hppExtention/g;
    push(@header,$line);
    push(@header,"#ifndef __" . uc($className) . "_H__\n");
    push(@header,"#define __" . uc($className) . "_H__\n");
    push(@header,"\n");
    push(@header,$extraIncludes);
    push(@header,"\n");
    push(@header,"/// \@brief Contains all $objectName related function wrappers.\n");
    push(@header,"///\n");
    push(@header,"/// Internal functions and triggers are not included.\n");
    push(@header,"class $className\n{\n");

    $line = $headerTemplate;
    $line =~ s/\@filename\@/$className.$cppExtention/g;
    $line =~ s/\[filename\]/$className.$cppExtention/g;
    push(@module,$line);
    push(@module,"#include \"$className.$hppExtention\"\n");
    push(@module,"#include <sptk3/db/CQueryGuard.h>\n");
    push(@module,"\n");
    push(@module,"using namespace std;\n");
    push(@module,"\n");

    $functionStarted = 0;
    $commentsStarted = 0;
    $function = "*";
    @functionData = ();
    foreach $row (@file) {
        if ($row =~ /^--/) {
            if ( ($row =~ /^-- \@brief/) && ($row !~ "(.nternal)") ) {
                $inlineSQL = "";
                $commentsStarted = 1;
                @comments = ();
                $comment = processDoxygenComment($row);
                push(@comments,$indentString . "$comment\n");
                push(@comments,$indentString . "///\n");
            } else {
                if ( $row =~ /^-- \@inline/ ) {
                    $pos1 = index($row,"\@inline ");
                    $inlineSQL = substr($row,$pos1+8,1024);
                } else {
                    if ($commentsStarted) {
                        $comment = processDoxygenComment($row);
                        push(@comments,$indentString . "$comment\n");
                    }
                }
            }
        }

        if ($commentsStarted && $row =~ /CREATE OR REPLACE FUNCTION/) {
            $commentsStarted = 0;
            if ($row !~ /functions_test/ ) {
                $pos1 = index($row,"FUNCTION");
                $function = substr($row,$pos1+length("FUNCTION")+1,255);
                $functionStarted = 1;
                @functionData = ();
            }
        } else {
            if ($functionStarted) {
                if (lc($row) =~ /returns .* as/) {
                    $functionStarted = 0;
                    $pos1 = index(lc($row),"returns ") + 8;
                    $pos2 = index(lc($row)," as");
                    ($returnType,$returnIsSetOf,$returnIsComplexType) = processReturnText(substr($row,$pos1,$pos2-$pos1));

                    if (lc($returnType) ne "trigger") {
                        $function .= substr($row,0,$pos1);
                        push(@functionPrototypes,"\n");
                        push(@functionPrototypes,@comments);
                        processFunction($function,$returnType,$className,$returnIsSetOf,$returnIsComplexType,$inlineSQL);
                    }
                } else {
                    $function .= $row;
                    push(@functionData,$row);
                }
            }
        }
    }

    close(IN);

    $functionPrototypeCount = @functionPrototypes;
    if ($functionPrototypeCount) {
        push(@header,"protected:\n");
        push(@header,@queryDefinitions);
        push(@header,"\n");

        push(@header,"public:\n");
        push(@header,$indentString . "/// \@brief Constructor\n");
        push(@header,$indentString . "///\n");
        push(@header,$indentString . "/// Initializes all $objectName-related queries\n");
        push(@header,$indentString . "/// \@param db sptk::CDatabase*, database connection\n");
        push(@header,$indentString . "$className(sptk::CDatabase* db);\n");

        push(@header,@functionPrototypes);
        push(@header,"};\n");
        push(@header,"\n#endif\n");

        push(@module,"$className\:\:$className(sptk::CDatabase\* db)\n");
        push(@module,":@queryInitializations\n{\n}\n\n");
        push(@module,@functionBodies);

        $outputDir = $configuration{'paths:outputhppdirectory'};
        if ($outputDir eq '') { $outputDir = '.'; }

        open(HDR_F,">$outputDir/$className.$hppExtention") || die("Can't write to $outputDir/$className.h");
        print HDR_F @header;
        close(HDR_F);

        $outputDir=$configuration{'paths:outputcppdirectory'};
        if ($outputDir eq '') { $outputDir = '.'; }

        open(MOD_F,">$outputDir/$className.$cppExtention") || die("Can't write to $outputDir/$className.cpp");
        print MOD_F @module;
        close(MOD_F);
    }

    $postprocessCommand = $configuration{'commands:postprocess'};
    if ($postprocessCommand) {
        externalCommand($postprocessCommand,$sourceFile,$className);
    }
}

sub help() {
    print "Sql2Cpp version 1.5. (C)opyright 2007-2008 Alexey Parshin.\n";
    print "Sponsored by Total Knowledge (www.total-knowledge.com)\n";
    print "Thin wrapper class generator.\n";
    print "Usage:\n";
    print "\nsql2cpp [OPTIONS] [sourcefile1.sql]..[sourcefileN.sql]\n\n";
    print "Reads .sql files and generates C++ wrappers.\n\n";
    print "Options:\n\n";
    print "--help                   prints this help\n\n";
    print "--config <filename>      defines the configuration file name.\n";
    print "                         If the configuration file name isn't\n";
    print "                         provided './sql2cpp.conf' is used.\n\n";
    print "--print-class-name-only  don't do anything besides printing\n";
    print "                         the C++ classname. This is useful for\n";
    print "                         makefiles.\n\n";
    print "Files with the name beginning from number are not processed. Functions with the word 'internal' in \@brief are ignored.\n";
    print "If the list of the files is omitted then all the .sql files in the current directory are processed.\n";
    exit(1);
}

sub classNameFromFileName {
    local ($className,$part,@parts);

    $fileName = $_[0];
    $fileName =~ s/\.sql$//g;
    @parts = split(/_/,$fileName);
    $className = shift(@parts);
    for $part (@parts) {
        $className .= ucfirst($part);
    }
    return $className;
}

$options{'config'} = 'sql2cpp.conf';
$options{'print-class-name-only'} = 0;

$configuration{'print:verbose'} = 1;

# Read command line options and file names
while (@ARGV) {
    $ARG = shift(@ARGV);
    if ( $ARG =~ /^--/ ) {
        if ( $ARG eq '--help' ) {
            help();
        }
        if ( $ARG =~ '-config$' ) {
            $options{'config'} = shift(@ARGV);
        }
        if ( $ARG eq '--print-class-name-only' ) {
            $options{'print-class-name-only'} = 1;
        }
    } else {
        if ( $ARG =~ /.sql$/ ) {
            push(@sqlFiles,$ARG);
        }
    }
}

readConfiguration($options{'config'});

$inputDirectory = $configuration{'paths:inputdirectory'};
if ( $inputDirectory ne '' )
{
    $inputDirectory .= '/';
    $inputDirectory =~ s/\/\//\//g;
}

if (@sqlFiles == 0) {
    opendir(DIR,$inputDirectory);
    @sqlFiles = grep {/.sql$/} readdir(DIR);
    closedir(DIR);
}

$classNames = $configuration{'typemapping:recordtypes'};

$fileClassNameTemplate = $configuration{'typemapping:fileclasstemplate'};

foreach $sqlFile (sort @sqlFiles) {
    @pathParts = split(/\//,$sqlFile);
    $shortFileName = pop(@pathParts);
    if ( $shortFileName !~ /^[_,0-9]/ ) { # Skip special files
        $sqlFile =~ s/\.sql$//g;
        $classNameBase = ucfirst(classNameFromFileName($shortFileName));
        $className = $fileClassNameTemplate;
        $className =~ s/\@filename\@/$classNameBase/g;
        $className =~ s/\[filename\]/$classNameBase/g;
        if ($options{'print-class-name-only'} == 1) {
            print "$className\n";
        } else {
            if ($configuration{'print:verbose'} > 0) {
                print "$inputDirectory$sqlFile: class $className\n";
            }
            processFile("$inputDirectory$sqlFile",$className);
        }
    }
}

exit(0)