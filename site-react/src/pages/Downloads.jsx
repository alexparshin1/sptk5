import React from "react";
import Seo from "../components/Seo";
import "../css/Documentation.css";
import ComboBox from "../components/ComboBox";
import ControlAPI from "../ControlAPI";
import "../css/Downloads.css"

export default class Downloads extends React.Component
{
    state = {
        sptkVersion: "",
        osVersion: ""
    };

    constructor()
    {
        super();

        this.sptkVersions = [];
        this.sptkVersionIndex = {};
        this.directories = [];
        this.directoryIndex = {};
    }

    async componentDidMount()
    {
        this.downloads = await ControlAPI.getRequest("site_downloads.php");
        if (this.downloads) {
            this.makeSptkVersionList();
            const sptkVersion = this.sptkVersions[0].value;
            const firstDirectory = this.makeOsVersionList(sptkVersion);
            this.setState({sptkVersion: sptkVersion, osVersion: firstDirectory.os_dir});
        }
    }

    makeSptkVersionList()
    {
        this.sptkVersions = [];
        this.sptkVersionIndex = {};
        for (let version of this.downloads) {
            this.sptkVersions.push({value: version.sptk_version, text: version.sptk_version});
            this.sptkVersionIndex[version.sptk_version] = version;
        }
    }

    makeOsVersionList(sptkVersionNumber)
    {
        let sptkVersion = this.sptkVersionIndex[sptkVersionNumber];

        this.directories = [];
        this.directoryIndex = {};
        for (let directory of sptkVersion.directories) {
            let item = {value: directory.directory.os_dir, text: directory.directory.title};
            this.directories.push(item);
            this.directoryIndex[directory.directory.os_dir] = directory;
        }

        return sptkVersion.directories[0].directory;
    }

    selectSptkVersion(sptkVersion)
    {
        if (sptkVersion !== this.state.sptkVersion) {
            let firstDirectory = this.makeOsVersionList(sptkVersion);
            this.setState({sptkVersion: sptkVersion, osVersion: firstDirectory.os_dir});
        }
    }

    isRequiredForXMQ(fileName)
    {
        const requeredForXMQ = [
            "sptk-core",
            "xmq-server"
        ];

        for (let required of requeredForXMQ) {
            if (fileName.indexOf(required) === 0) {
                return true;
            }
        }

        return false;
    }

    render()
    {
        let directory = this.directoryIndex[this.state.osVersion];
        let files = [];
        if (directory) {
            for (let file of directory.files.sort()) {
                files.push(<tr key={file.file + "-info"}>
                    <td key={file.file + "-name"} className="FileInfo">
                        <a href={"download/" + this.state.sptkVersion + "/" + directory.directory.os_dir + "/" + file.file}>
                            {file.file}
                        </a>
                    </td>
                    <td key={file.file + "-date"} className="FileInfo">{file.fdate}</td>
                    <td key={file.file + "-size"} className="FileInfo">{file.fsize}</td>
                </tr>);
            }
        }
        return <div className="Downloads">
            <Seo title="Downloads — XMQ MQTT Server and SPTK Library"
                 description="Download the free XMQ MQTT server for Linux (.deb and .rpm) and Windows, and the SPTK C++ class library in source code and binary packages."
                 keywords="download MQTT server, free MQTT server, Linux MQTT server, Windows MQTT server, XMQ, SPTK"
                 path="/downloads"/>
            <div style={{textAlign: "left", padding: 16}}>
                <h1>Download the XMQ MQTT server</h1>
                <p>
                    The free XMQ MQTT server is distributed as binary packages for Linux
                    (.deb and .rpm) and Windows. The same release directories hold the SPTK
                    class library that XMQ is built with, available in both source code and
                    binary packages.
                </p>
            </div>
            <div>
                <label style={{padding: 16}}>Release version:</label>
                <ComboBox name="sptk_versions" style={{padding: 16}} items={this.sptkVersions}
                          onChange={(sptkVersion) =>
                              this.selectSptkVersion(sptkVersion)
                          }></ComboBox>

                <label style={{padding: 16}}>Operating System:</label>
                <ComboBox name="os_versions" style={{padding: 16}} items={this.directories}
                          onChange={(osVersion) => this.setState({osVersion: osVersion})}></ComboBox>

            </div>
            <div style={{textAlign: "left", padding: 16}}>
                <table>
                    <thead>
                    <tr>
                        <th style={{width: 300}}>File</th>
                        <th style={{width: 150}}>Date</th>
                        <th style={{width: 100}}>Size</th>
                    </tr>
                    </thead>
                    <tbody>
                    {files}
                    </tbody>
                </table>
            </div>
        </div>;
    }
}
