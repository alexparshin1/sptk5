import React from "react";
import "../css/Documentation.css";
import ComboBox from "../components/ComboBox";
import ControlAPI from "../ControlAPI";

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
        for (let directory of sptkVersion.directories)
        {
            let item = {value: directory.directory.os_dir, text: directory.directory.title};
            this.directories.push(item);
        }
    }

    render()
    {
        return <div className="Documentation">
            <div id="fadeout"/>
            <div>
                <label style={{padding: 16}}>SPTK version:</label>
                <ComboBox name="sptk_versions" style={{padding: 16}} items={this.sptkVersions}></ComboBox>

                <label style={{padding: 16}}>Operating System:</label>
                <ComboBox name="os_versions" style={{padding: 16}} items={this.directories}></ComboBox>
            </div>
        </div>;
    }
}
