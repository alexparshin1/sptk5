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

    // A .sha256 is not a download of its own - it belongs to the file it is named after, and is
    // reported with it as "sha256" below. It stays on disk and reachable by URL, for anyone who
    // would rather run "shasum -a 256 -c" than compare by eye; it is only not listed as a package.
    $downloads = array();
    foreach ($files as $file) {
        if ($file == "." || $file == ".." || substr($file, -7) === ".sha256")
            continue;
        $downloads[] = $file;
    }

    // Only the newest release of each package is listed. XMQ and SPTK no longer move together, so
    // a directory named after an SPTK version can hold two XMQ releases - SPTK-5.6.6 holds 0.9.11
    // and 0.9.12 - and listing both side by side with nothing to say which is newer invites
    // someone to take the older one.
    //
    // The version is the first dotted number in the name, and what remains once it is taken out
    // identifies the package: xmq-server_0.9.16_amd64.deb and xmq-server_0.9.15_amd64.deb reduce
    // to the same thing, sptk-core_5.6.9_amd64.deb to something else. Nothing here needs to know
    // what any of the products are called. A file with no version in its name is always listed.
    //
    // The older files are not removed and stay reachable at their URL, which is how a previous
    // release can still be fetched; they are only not offered beside the current one.
    $newest = array();
    $unversioned = array();
    foreach ($downloads as $file) {
        if (!preg_match('/\d+\.\d+(?:\.\d+)*/', $file, $matched)) {
            $unversioned[] = $file;
            continue;
        }
        $version = $matched[0];
        $package = str_replace($version, "%V%", $file);
        if (!isset($newest[$package]) || version_compare($version, $newest[$package]["version"], ">")) {
            $newest[$package] = array("version" => $version, "file" => $file);
        }
    }
    $downloads = $unversioned;
    foreach ($newest as $entry) {
        $downloads[] = $entry["file"];
    }
    sort($downloads);

    $first = true;
    $fileCount = 0;
    foreach ($downloads as $file) {
        if ($first) {
            $first = false;
        } else {
            echo ",\n";
        }

        // The hash alone. The file holds it in the format shasum reads - the hash, two spaces and
        // the name - and the name is already known here.
        $checksum = "";
        $checksumFile = "$directory/$file.sha256";
        if (file_exists($checksumFile)) {
            $checksum = strtok(trim(file_get_contents($checksumFile)), " ");
        }

        echo "          { \"file\": \"$file\", ";
        echo "\"fdate\": \"" . date("d M Y", filemtime("$directory/$file")) . "\", ";
        echo "\"fsize\": \"" . (int) (filesize("$directory/$file") / 1024) . " Kb\", ";
        echo "\"sha256\": \"$checksum\" }";
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
        "ubuntu-plucky"  => "Ubuntu 25.04",
        "ubuntu-questing"  => "Ubuntu 25.10",
        "ubuntu-resolute"  => "Ubuntu 26.04",
        "debian-bookworm"  => "Debian Bookworm",
        "debian-trixie"  => "Debian Trixie",
        "fedora-37"      => "Fedora 37",
        "fedora-38"      => "Fedora 38",
        "fedora-39"      => "Fedora 39",
        "fedora-40"      => "Fedora 40",
        "fedora-41"      => "Fedora 41",
        "fedora-42"      => "Fedora 42",
        "fedora-43"      => "Fedora 43",
        "oraclelinux9"   => "Oracle Linux 9",
        "oraclelinux-9.5" => "Oracle Linux 9.5",
        // The major version alone: the package's ABI is FreeBSD:15:amd64, so it installs on any
        // 15.x. Without an entry here the directory would still be listed, under the name the
        // fallback makes of it - "Freebsd 15".
        "freebsd-15" => "FreeBSD 15",
        "tar" => "Source code (OS-independent)",
        "windows" => "Windows 10"
    );
}

// Fallback title for directories not listed in osVersionNames()
function prettyOsName($dirname)
{
    return ucwords(str_replace(array("-", "_"), " ", $dirname));
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

        // Known directories first, in the curated order of osVersionNames(),
        // then any other sub-directories with a title generated from their name
        $directories = array();
        foreach(osVersionNames() as $dirname => $osVersion) {
            if (file_exists("$versionDirectory/$dirname")) {
                $directories[$dirname] = $osVersion;
            }
        }
        foreach (scandir($versionDirectory) as $dirname) {
            if ($dirname[0] == "." || isset($directories[$dirname]) || !is_dir("$versionDirectory/$dirname")) {
                continue;
            }
            $directories[$dirname] = prettyOsName($dirname);
        }

        $firstDir = true;
        foreach($directories as $dirname => $osVersion) {
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
