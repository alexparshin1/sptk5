<?php

header('Content-Type: text/json');
header("Access-Control-Allow-Origin: *");
header("Access-Control-Allow-Methods: POST, GET, OPTIONS");
header("Access-Control-Allow-Headers: Content-Type, Content-Length, Content-Encoding, Access-Control-Allow-Origin, Authorization");

function countDownloadFiles($sptkVersion, $directory)
{
    if (!file_exists($directory)) {
        return 0;
    }

    $files = scandir($directory);
    if ($files === false)
        return 0;

    $matchingFiles = array();

    return count($files);
}

function getDownloadFiles($sptkVersion, $directory, $os_dirname, $title)
{
    if (!file_exists($directory)) {
        return null;
    }

    $files = scandir($directory);
    if ($files === false)
        return null;

    $matchingFiles = array();

    if (count($files) == 0)
        return null;

    echo "        \"directory\": { \"title\": \"$title\", \"os_dir\": \"$os_dirname\" },\n";

    echo "        \"files\": [\n";

    $first = true;
    $fileCount = 0;
    foreach ($files as $file) {
        if ($file == "." || $file == "..")
            continue;
        if ($first) {
            $first = false;
        } else {
            echo ",\n";
        }
        echo "          { \"file\": \"$file\", ";
        echo "\"fdate\": \"" . date("d M Y", filemtime("$directory/$file")) . "\", ";
        echo "\"fsize\": \"" . (int) (filesize("$directory/$file") / 1024) . " Kb\" }";
        $fileCount++;
    }

    echo "\n        ]";

    return $fileCount;
}

function scanVersions($downloadDirectory)
{
    $versions = array();
    $files = scandir($downloadDirectory);
    foreach ($files as $file) {
        if (preg_match('/^SPTK\-\d\.\d+.\d+$/', $file)) {
            array_push($versions, $file);
        }
    }
    rsort($versions);
    return $versions;
}

function osVersionNames()
{
    return array(
        "ubuntu-jammy"   => "Ubuntu 22.04",
        "ubuntu-noble"  => "Ubuntu 24.04",
        "ubuntu-oracular"  => "Ubuntu 24.10",
        "debian-bookworm"  => "Debian Bookworm",
        "fedora-37"      => "Fedora 37",
        "fedora-38"      => "Fedora 38",
        "fedora-39"      => "Fedora 39",
        "fedora-40"      => "Fedora 40",
        "fedora-41"      => "Fedora 41",
        "oraclelinux9"   => "Oracle Linux 9",
        "tar" => "Source code (OS-independent)",
        "windows" => "Windows 10"
    );
}

function getSptkVersions($downloadDirectory)
{
    $versions = scanVersions($downloadDirectory);
    echo '"versions": [';
    $first = true;
    foreach ($versions as $version) {
        if ($first) {
            $first = false;
        }
        else {
            echo ", ";
        }
        echo "\"$version\"";
    }
    echo "]\n";
}

function getOsVersions()
{
    echo '"osVersions": {';
    foreach (osVersionNames() as $name => $version) {
        $first = true;
        if ($first) {
            $first = false;
        }
        else {
            echo ", ";
        }
        echo "\"$name\": \"$version\"";
    }
    echo "}\n";
}

function getAllDownloads($versions)
{
    $first = true;

    $sptkVersionFileCounter = [ "Test" => "Testic" ];
    foreach ($versions as $sptkVersion) {
        $versionDirectory = "download/$sptkVersion";

        if ($first) {
            $first = false;
        } else {
            echo ",\n";
        }

        echo "  {\n";
        echo "    \"sptk_version\": \"$sptkVersion\",\n";
        echo "    \"directories\": [\n";

        $firstDir = true;
        foreach(osVersionNames() as $dirname => $osVersion) {
            if (!file_exists("$versionDirectory/$dirname")) {
                continue;
            }
            if ($firstDir) {
                $firstDir = false;
            } else {
                echo ",\n";
            }
            echo "      {\n";
            getDownloadFiles($sptkVersion, "$versionDirectory/$dirname", $dirname, $osVersion);
            echo "\n      }";
        }

        echo "\n    ]\n  }";
    }

    echo "\n";

    return $sptkVersionFileCounter;
}

$versions = scanVersions("download");

//createVersionComboBox($versions);
//createOsComboBox();
echo "[\n";
$sptkVersionFileCounter = getAllDownloads($versions);
echo "]\n";

$sptkVersion = $versions[0];
//printDownloadFiles($sptkVersion, "download/dependencies", "windows", "Windows 10 dependencies (Open Source only)", false);
