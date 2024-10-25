import React from "react";
import ControlAPI from "../ControlAPI";

export default class Home extends React.Component
{
    state = {
        fileName: ""
    };

    renderNewsItem(item)
    {
        return <div>
            <div>{item.version_date} {item.version}</div>
            <div dangerouslySetInnerHTML={{__html: item.news}}/>
        </div>;
    }

    render()
    {
        let historyData = ControlAPI.getRequest("host_news_list.php");
        let history = [];
        for (let i = 0; i < historyData.length; i++) {
            history.push(this.renderNewsItem(historyData[i]));
        }
        return <div className="Page">
            <p>
            This website contains two main products:
            <ul>
                <li>XMQ cross-platform MQTT server.</li>
                <li>SPTK class library that is used for cross-platform development.</li>
            </ul>
            Both, XMQ and SPTK use GPL/LGPL License Agreement v2.0 that can be found <a href="https://opensource.org/license/gpl-2-0">here.</a>
            </p>
            <p>
                Below is the history of development, a kind of changelog and new version announcements.
            </p>
            {history}
        </div>;
    }
}
