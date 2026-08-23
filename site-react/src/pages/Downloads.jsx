import React from "react";
import Seo from "../components/Seo";
import "../css/Documentation.css";
import ComboBox from "../components/ComboBox";
import ControlAPI from "../ControlAPI";
import {buildXmqReleases, sptkVersionNumber, xmqFileVersion} from "../downloadsCatalog";
import "../css/Downloads.css"
import "./UserManual.css";

function sortedFiles(files)
{
    return [...files].sort((left, right) => left.file.localeCompare(right.file));
}

export default class Downloads extends React.Component
{
    state = {
        xmqVersion: "",
        osVersion: ""
    };

    constructor()
    {
        super();

        this.xmqVersions = [];
        this.releaseIndex = {};
    }

    async componentDidMount()
    {
        this.downloads = await ControlAPI.getRequest("site_downloads.php");
        if (this.downloads && this.downloads.length > 0) {
            const catalog = buildXmqReleases(this.downloads);
            this.xmqVersions = catalog.xmqVersions;
            this.releaseIndex = catalog.releaseIndex;
            if (this.xmqVersions.length > 0) {
                const xmqVersion = this.xmqVersions[0].value;
                this.setState({xmqVersion: xmqVersion, osVersion: this.releaseIndex[xmqVersion].osList[0].value});
            }
        }
    }

    selectXmqVersion(xmqVersion)
    {
        if (xmqVersion !== this.state.xmqVersion) {
            const release = this.releaseIndex[xmqVersion];
            // The same operating system is kept selected when the new version is built for it
            const osVersion = release.osIndex[this.state.osVersion] ? this.state.osVersion : release.osList[0].value;
            this.setState({xmqVersion: xmqVersion, osVersion: osVersion});
        }
    }

    // The key is only for the source code tables, which are rendered from a list. Giving
    // one to the tables that always sit in the same place makes React add a new table on
    // every change of the selection instead of replacing the one that is there.
    fileTable(files, entry, key)
    {
        return <table key={key}>
            <thead>
            <tr>
                <th style={{width: 300}}>File</th>
                <th style={{width: 150}}>Date</th>
                <th style={{width: 100}}>Size</th>
            </tr>
            </thead>
            <tbody>
            {files.map((file) => <tr key={file.file + "-info"}>
                <td key={file.file + "-name"} className="FileInfo">
                    <a href={"download/" + entry.sptkVersion + "/" + entry.osDir + "/" + file.file}>
                        {file.file}
                    </a>
                </td>
                <td key={file.file + "-date"} className="FileInfo">{file.fdate}</td>
                <td key={file.file + "-size"} className="FileInfo">{file.fsize}</td>
            </tr>)}
            </tbody>
        </table>;
    }

    render()
    {
        const release = this.releaseIndex[this.state.xmqVersion];
        const entry = release ? release.osIndex[this.state.osVersion] : null;

        const xmqFiles = [];
        const sptkFiles = [];
        if (entry) {
            for (const file of sortedFiles(entry.files)) {
                const fileVersion = xmqFileVersion(file.file);
                if (fileVersion === null) {
                    sptkFiles.push(file);
                } else if (fileVersion === this.state.xmqVersion) {
                    // Older XMQ packages left in the same directory are not shown
                    xmqFiles.push(file);
                }
            }
        }

        return <div className="Downloads">
            <Seo title="Downloads — XMQ MQTT Server and SPTK Library"
                 description="Download the free XMQ MQTT server for Linux (.deb and .rpm), Windows, and as a Docker image, and the SPTK C++ class library in source code and binary packages."
                 keywords="download MQTT server, free MQTT server, MQTT Docker image, MQTT server Docker, Linux MQTT server, Windows MQTT server, XMQ, SPTK"
                 path="/downloads"/>
            <div style={{textAlign: "left", padding: 16}}>
                <h1>Download the XMQ MQTT server</h1>
                <p>
                    The free XMQ MQTT server is distributed as binary packages for Linux
                    (.deb and .rpm) and Windows, and as a <a href="#docker">Docker image</a>. Pick
                    the XMQ version and the operating system; each release also lists the SPTK
                    class library it was built with, in source code and binary packages.
                </p>
            </div>
            <div>
                <label style={{padding: 16}}>XMQ version:</label>
                <ComboBox name="xmq_versions" style={{padding: 16}} items={this.xmqVersions}
                          value={this.state.xmqVersion}
                          onChange={(xmqVersion) =>
                              this.selectXmqVersion(xmqVersion)
                          }></ComboBox>

                <label style={{padding: 16}}>Operating System:</label>
                <ComboBox name="os_versions" style={{padding: 16}} items={release ? release.osList : []}
                          value={this.state.osVersion}
                          onChange={(osVersion) => this.setState({osVersion: osVersion})}></ComboBox>

            </div>
            <div style={{textAlign: "left", padding: 16}}>
                {xmqFiles.length > 0
                    ? this.fileTable(xmqFiles, entry)
                    : <p className="DownloadsNote">There is no XMQ package for this selection.</p>}
                {sptkFiles.length > 0 && entry &&
                    <p className="DownloadsNote">
                        The SPTK {sptkVersionNumber(entry.sptkVersion)} packages below were used to
                        build XMQ {this.state.xmqVersion}, and are not needed for the XMQ download.
                    </p>}
                {sptkFiles.length > 0 && this.fileTable(sptkFiles, entry)}
                {release && release.sourceEntries.length > 0 &&
                    <p className="DownloadsNote">
                        The source code of SPTK {sptkVersionNumber(release.sptkVersion)}, the same
                        for every operating system, and not needed for the XMQ download either.
                    </p>}
                {release && release.sourceEntries.map((sourceEntry) =>
                    this.fileTable(sortedFiles(sourceEntry.files), sourceEntry,
                                   sourceEntry.sptkVersion + "-" + sourceEntry.osDir))}
            </div>
            <div id="docker" style={{textAlign: "left", padding: 16}}>
                <h2>Run it in Docker</h2>
                <p>
                    The same release is published as a Docker image,{" "}
                    <a href="https://hub.docker.com/r/alexeyparshin/xmq">alexeyparshin/xmq</a>. The
                    image installs the .deb from this page rather than building from source, so a
                    container holds exactly the binaries listed above. Tags are the version number
                    and <code>latest</code>; the image is amd64, 36 MB to download.
                </p>
                <pre className="userManualCode">{`docker run --rm -p 1883:1883 alexeyparshin/xmq`}</pre>
                <p>
                    The broker takes clients with no credentials out of the box, so nothing has to
                    be configured before the first publish:
                </p>
                <pre className="userManualCode">{`mosquitto_pub -h localhost -t test/hello -m 'first message'`}</pre>
                <p>
                    That default suits trying the broker out and nothing else. Add{" "}
                    <code>-e XMQ_ALLOW_ANONYMOUS=false</code> before the container is reachable
                    from a network you do not control, and create accounts in the configuration
                    interface on <code>https://localhost:18883</code> — published with{" "}
                    <code>-p 18883:18883</code>, first sign-in <b>admin / admin</b>, to be changed
                    at once. MQTT over TLS is on 8883, with a self-signed certificate the container
                    generates on first start.
                </p>
                <p>
                    The configuration, the accounts and the certificate live in{" "}
                    <code>/etc/xmq</code>. Mount it, or every restart is a fresh install:
                </p>
                <pre className="userManualCode">{`docker run -d -p 1883:1883 -p 8883:8883 -p 18883:18883 -v xmq-config:/etc/xmq --ulimit nofile=1048576:1048576 alexeyparshin/xmq`}</pre>
                <p>
                    The descriptor limit belongs there too: Docker gives a container 1024 open
                    files, which caps the broker at about a thousand connections. Sessions, queued
                    messages and retained messages are kept in Redis when{" "}
                    <code>XMQ_PERSISTENCE=true</code> is set; the <code>docker-compose.yml</code>{" "}
                    in the source tree starts both containers and wires them together. The full
                    list of settings is on the{" "}
                    <a href="https://hub.docker.com/r/alexeyparshin/xmq">image page</a>.
                </p>
            </div>
        </div>;
    }
}
