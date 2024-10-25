import React from "react";
import "../css/Screenshots.css"

import cscroll from "../screenshots/cscroll.png";
import db_combo from "../screenshots/db_combo.png";
import dialog from "../screenshots/dialog.png";
import dir_ds from "../screenshots/dir_ds.png";
import imap_ds from "../screenshots/imap_ds.png";
import radio from "../screenshots/radio.png";
import save_as from "../screenshots/save_as.png";
import sptk1 from "../screenshots/sptk1.png";
import sptk2 from "../screenshots/sptk2.png";
import sptk3 from "../screenshots/sptk3.png";
import theme_blue from "../screenshots/theme_blue.png";
import theme_dark from "../screenshots/theme_dark.png";
import theme_default from "../screenshots/theme_default.png";
import theme_e17 from "../screenshots/theme_e17.png";
import theme_flat from "../screenshots/theme_flat.png";
import theme_keramic from "../screenshots/theme_keramic.png";
import theme_opera from "../screenshots/theme_opera.png";
import theme_osx from "../screenshots/theme_osx.png";
import tree from "../screenshots/tree.png";

export default class Screenshots extends React.Component
{
    render()
    {
        return <div style={{align: "left", background: "#ddd"}}>
            <div id="fadeout"></div>
            <table className="screenshot-table" style={{width:"100%"}}>
                <thead>
                    <th style={{width:200}}/>
                    <th/>
                </thead>
                <tbody>
                <tr>
                    <td>
                        <img src={cscroll} alt="" title=""/>
                    </td>
                    <td>
                        CScroll demo. Shows how the auto widget layout works even inside the scroll box.
                    </td>
                </tr>

                <tr>
                    <td>
                        <img src={db_combo} alt="" title=""/>
                    </td>
                    <td>
                        CComboBox, CDBListView and CDialog demo. Shows how to display and edit a simple database data.
                    </td>
                </tr>

                <tr>
                    <td>
                        <img src={dialog} alt="" title=""/>
                    </td>
                    <td>
                        CDialog demo. Shows how to pass and edit data in a dialog with multiple pages.
                    </td>
                </tr>

                <tr>
                    <td>
                        <img src={dir_ds} alt="" title=""/>
                    </td>
                    <td>
                        CDirectoryDS demo. Shows how to use a simple dataset with CListView.
                    </td>
                </tr>

                <tr>
                    <td>
                        <img src={imap_ds} alt="" title=""/></td>
                    <td>
                        CImapDS demo. Displays the list of e-mails in the mailbox in CListView.
                    </td>
                </tr>

                <tr>
                    <td>
                        <img src={radio} alt="" title=""/>
                    </td>
                    <td>
                        CRadioButtons demo. Demonstartes how to create and use a group of radio buttons with optional
                        'Other' entry.
                    </td>
                </tr>

                <tr>
                    <td>
                        <img src={save_as} alt="" title=""/>
                    </td>
                    <td>
                        CFileSaveDialog demo. An example for the SPTK 'Save As' dialog.
                    </td>
                </tr>

                <tr>
                    <td>
                        <img src={sptk1} alt="" title=""/>
                    </td>
                    <td>
                        General SPTK demo. Shows the available stock buttons inside CTabs.
                    </td>
                </tr>

                <tr>
                    <td>
                        <img src={sptk2} alt="" title=""/>
                    </td>
                    <td>
                        General SPTK demo. Shows different widgets inside CTabs.
                    </td>
                </tr>

                <tr>
                    <td>
                        <img src={sptk3} alt="" title=""/>
                    </td>
                    <td>
                        General SPTK demo. Shows database connected widgets inside CTabs.
                    </td>
                </tr>

                <tr>
                    <td>
                        <img src={tree} alt="" title=""/>
                    </td>
                    <td>
                        CTreeView demo. Shows SPTK Tree View widget.
                    </td>
                </tr>
                </tbody>
            </table>

        </div>;
    }
}
