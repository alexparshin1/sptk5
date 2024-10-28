import React from "react";
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

        this.downloads = ControlAPI.getRequest("site_downloads.php");

        this.makeSptkVersionList();
        this.state.sptkVersion = this.sptkVersions[0].value;

        this.makeOsVersionList(this.state.sptkVersion);
        this.state.osVersion = this.directories[0].value;
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
            "sptk-db_5",
            "sptk-db_mysql",
            "sptk-db_postgresql",
            "sptk-db_sqlite3",
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
            for (let file of directory.files) {
                let isFileRequiresForXMQ = this.isRequiredForXMQ(file.file);
                files.push(<tr key={file.file + "-info"}>
                    <td key={file.file + "-name"} className="FileInfo">
                        <a href={"download/" + this.state.sptkVersion + "/" + directory.directory.os_dir + "/" + file.file}>
                            {file.file}
                        </a>
                    </td>
                    <td key={file.file + "-date"} className="FileInfo">{file.fdate}</td>
                    <td key={file.file + "-size"} className="FileInfo">{file.fsize}</td>
                    <td key={file.file + "-required"} className="FileInfo">{isFileRequiresForXMQ ? "Yes" : "No"}</td>
                </tr>);
            }
        }
        return <div className="Downloads">
            <div>
                <label style={{padding: 16}}>SPTK version:</label>
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
                        <th style={{width: 100}}>Date</th>
                        <th style={{width: 100}}>Size</th>
                        <th style={{width: 150}}>Required for XMQ</th>
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
