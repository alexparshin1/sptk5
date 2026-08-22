// The download area is laid out by SPTK version on disk, and every release directory holds the
// XMQ packages built with that SPTK version. The Downloads page turns that inside out: XMQ is
// what is downloaded, so its version is picked first and the SPTK version follows from it.
// This module does the regrouping, from the JSON of site_downloads.php.

const XMQ_FILE = /xmq/i;

// The version in an XMQ package name: xmq-server_0.9.14_amd64.deb, xmq-server-0.9.14.rpm,
// XMQ-0.9.13.msi, XMQ-0.9.11.2.msi. The fourth component of the Windows installer name is a
// build number, so only the three that make the release are kept.
export function xmqFileVersion(fileName)
{
    if (!XMQ_FILE.test(fileName)) {
        return null;
    }
    const version = fileName.match(/\d+\.\d+\.\d+/);
    return version ? version[0] : null;
}

export function sptkVersionNumber(sptkVersion)
{
    return sptkVersion.replace(/^SPTK-/, "");
}

function compareVersionsDescending(left, right)
{
    const leftParts = left.split(".").map(Number);
    const rightParts = right.split(".").map(Number);
    for (let i = 0; i < Math.max(leftParts.length, rightParts.length); i++) {
        const difference = (rightParts[i] || 0) - (leftParts[i] || 0);
        if (difference !== 0) {
            return difference;
        }
    }
    return 0;
}

// A release directory that the download endpoint could not read comes back as an empty object
function validDirectories(sptkVersion)
{
    if (!sptkVersion || !Array.isArray(sptkVersion.directories)) {
        return [];
    }
    return sptkVersion.directories.filter((entry) => entry && entry.directory && Array.isArray(entry.files));
}

// Directories that never hold an XMQ package - the source code archives - belong to SPTK
// alone, and are listed with every XMQ version built from that SPTK release
function operatingSystemsWithXmq(downloads)
{
    const operatingSystems = new Set();
    for (const sptkVersion of downloads) {
        for (const entry of validDirectories(sptkVersion)) {
            if (entry.files.some((file) => xmqFileVersion(file.file))) {
                operatingSystems.add(entry.directory.os_dir);
            }
        }
    }
    return operatingSystems;
}

// Keeps the operating system order of the download endpoint, which lists the known systems in
// a curated order and appends the rest
function makeOsList(release, sptkVersion)
{
    const osList = [];
    const listed = new Set();
    for (const entry of validDirectories(sptkVersion)) {
        const osDir = entry.directory.os_dir;
        if (release.osIndex[osDir]) {
            osList.push({value: osDir, text: release.osIndex[osDir].title});
            listed.add(osDir);
        }
    }
    for (const [osDir, entry] of Object.entries(release.osIndex)) {
        if (!listed.has(osDir)) {
            osList.push({value: osDir, text: entry.title});
        }
    }
    return osList;
}

// Groups every download directory by the XMQ version its packages carry. The SPTK versions
// arrive newest first, so the first directory found for an operating system is the one from
// the newest release, and the rest are older copies of the same XMQ packages.
export function buildXmqReleases(downloads)
{
    if (!Array.isArray(downloads)) {
        return {xmqVersions: [], releaseIndex: {}};
    }

    const sptkVersionIndex = {};
    for (const sptkVersion of downloads) {
        sptkVersionIndex[sptkVersion.sptk_version] = sptkVersion;
    }
    const osWithXmq = operatingSystemsWithXmq(downloads);

    const releaseIndex = {};
    for (const sptkVersion of downloads) {
        for (const entry of validDirectories(sptkVersion)) {
            const osDir = entry.directory.os_dir;
            const fileVersions = new Set(entry.files.map((file) => xmqFileVersion(file.file)).filter(Boolean));
            for (const xmqVersion of fileVersions) {
                if (!releaseIndex[xmqVersion]) {
                    // The newest SPTK version holding this XMQ version is the one it is shown with
                    releaseIndex[xmqVersion] = {sptkVersion: sptkVersion.sptk_version, osIndex: {}, osList: []};
                }
                const release = releaseIndex[xmqVersion];
                if (!release.osIndex[osDir]) {
                    release.osIndex[osDir] = {
                        sptkVersion: sptkVersion.sptk_version,
                        title: entry.directory.title,
                        files: entry.files
                    };
                }
            }
        }
    }

    for (const release of Object.values(releaseIndex)) {
        const primaryVersion = sptkVersionIndex[release.sptkVersion];
        for (const entry of validDirectories(primaryVersion)) {
            const osDir = entry.directory.os_dir;
            if (!osWithXmq.has(osDir) && !release.osIndex[osDir]) {
                release.osIndex[osDir] = {
                    sptkVersion: release.sptkVersion,
                    title: entry.directory.title,
                    files: entry.files
                };
            }
        }
        release.osList = makeOsList(release, primaryVersion);
    }

    const xmqVersions = Object.keys(releaseIndex)
        .sort(compareVersionsDescending)
        .map((xmqVersion) => ({value: xmqVersion, text: xmqVersion}));

    return {xmqVersions: xmqVersions, releaseIndex: releaseIndex};
}
