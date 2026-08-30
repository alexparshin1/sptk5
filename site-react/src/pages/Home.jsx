import React from "react";
import ControlAPI from "../ControlAPI";
import {Link} from "react-router-dom";
import Seo from "../components/Seo";
import "../css/News.css";

export default class Home extends React.Component
{
    state = {
        fileName: "",
        historyData: []
    };

    async componentDidMount()
    {
        const historyData = await ControlAPI.getRequest("site_host_news_list.php");
        if (historyData) {
            this.setState({historyData});
        }
    }

    renderNewsItem(item)
    {
        return <div key={item.version + item.version_date}>
            <hr/>
            <h3>{item.version_date}: {item.version}</h3>
            <div className="news" dangerouslySetInnerHTML={{__html: item.news}}/>
        </div>;
    }

    render()
    {
        const {historyData} = this.state;
        let history = [];
        for (let i = 0; i < historyData.length; i++) {
            history.push(this.renderNewsItem(historyData[i]));
        }

        return <div key="home-page" className="Page">
            <Seo title="XMQ — Fast Free MQTT Server for Linux, FreeBSD and Windows"
                 description="XMQ is a fast, free MQTT server for Linux, FreeBSD and Windows. MQTT 3.1, 3.1.1, and 5.0, QoS 0/1/2, TLS, message persistence, MQTT bridge and up to 1 million concurrent clients, with an open MQTT test suite and published MQTT performance tests."
                 keywords="XMQ, fast MQTT server, free MQTT server, MQTT bridge, Linux MQTT server, FreeBSD MQTT server, MQTT broker FreeBSD, Windows MQTT server, MQTT test suite, MQTT performance tests"
                 path="/"/>

            <h1>XMQ — a fast, free MQTT server for Linux, FreeBSD and Windows</h1>
            <p>
                <Link to="/xmq_about">XMQ</Link> is a cross-platform MQTT server written in C++.
                It implements MQTT protocol versions 3.1, 3.1.1, and 5.0 with QoS levels 0, 1, and 2,
                TLS encryption, message persistence over database backend, and
                MQTT bridging between nodes. A single instance serves up to 1 million
                concurrent clients.
            </p>
            <ul>
                <li>
                    Runs as a <b>Linux MQTT server</b> (.deb and .rpm packages), a
                    <b> FreeBSD MQTT server</b> (pkg), and a <b>Windows MQTT server</b>.
                </li>
                <li>
                    Acts as an <b>MQTT bridge</b>, forwarding messages between XMQ nodes and
                    other MQTT servers.
                </li>
                <li>
                    Ships with an open <Link to="/xmq_mqtt_test_suite">MQTT test suite</Link> built
                    on standard MQTT protocols, so the
                    published <Link to="/xmq_tests_environment">MQTT performance tests</Link> can be
                    reproduced and compared against other MQTT servers.
                </li>
            </ul>
            <p>
                XMQ is <b>free</b>, released under the GPL/LGPL License Agreement v2.0 that can be
                found <a href="https://opensource.org/license/gpl-2-0">here</a>, and is distributed as
                binary packages from the <Link to="/downloads">Downloads</Link> page.
            </p>

            <h2>Built with SPTK</h2>
            <p>
                XMQ is built on top of <Link to="/sptk_about">SPTK</Link> (Simply Powerful Toolkit),
                the cross-platform C++20 class library developed alongside it. SPTK provides the
                networking, threading, database and XML/JSON layers that XMQ runs on, and is
                available separately in source code and binary packages from
                the <Link to="/downloads">Downloads</Link> page.
            </p>

            <h2>Release history</h2>
            <p>
                Below is the history of development for the last 12 months, a kind of changelog and
                new version announcements.
            </p>
            {history}
        </div>;
    }
}
