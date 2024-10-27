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

        this.makeOsVersionList();
        this.state.osVersion = this.directories[0].value;
    }

    makeSptkVersionList()
    {
        this.sptkVersions = [];
        this.sptkVersionIndex = {};
        for (let version of this.downloads)
        {
            this.sptkVersions.push({value: version.sptk_version, text: version.sptk_version});
            this.sptkVersionIndex[version.sptk_version] = version;
        }
    }

    makeOsVersionList()
    {
        let sptkVersion = this.sptkVersionIndex[this.state.sptkVersion];

        this.directories = [];
        this.directoryIndex = {};
        for (let directory of sptkVersion.directories)
        {
            let item = {value: directory.directory.os_dir, text: directory.directory.title};
            this.directories.push(item);
            this.directoryIndex[directory.directory.os_dir] = directory;
        }
    }

    render()
    {
        let directory = this.directoryIndex[this.state.osVersion];
        let files = [];
        for (let file of directory.files)
        {
            //files.push(<a href={file.file.link} key={file.file.link}>{file.file.title}</a>);
            files.push(<tr key={file.file + "-info"}>
                <td key={file.file + "-name"} className="FileInfo"><a href={"download/" + this.state.sptkVersion + "/" + directory.directory.os_dir + "/" + file.file}>{file.file}</a></td>
                <td key={file.file + "-date"} className="FileInfo">{file.fdate}</td>
                <td key={file.file + "-size"} className="FileInfo">{file.fsize}</td>
            </tr>);
        }

        return <div className="Documentation">
            <div id="fadeout"/>
            <div>
                <label style={{padding: 16}}>SPTK version:</label>
                <ComboBox name="sptk_versions" style={{padding: 16}} items={this.sptkVersions}
                          onChange={(sptkVersion) => this.setState({sptkVersion: sptkVersion})}></ComboBox>

                <label style={{padding: 16}}>Operating System:</label>
                <ComboBox name="os_versions" style={{padding: 16}} items={this.directories}
                          onChange={(osVersion) => this.setState({osVersion: osVersion})}></ComboBox>

            </div>
            <div style={{textAlign: "left", padding: 16}}>
            <table>
                <thead>
                <tr>
                    <th>File</th>
                    <th>Date</th>
                    <th>Size</th>
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
