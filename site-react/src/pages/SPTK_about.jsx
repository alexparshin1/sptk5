import React from "react";
import Seo from "../components/Seo";
import {Link} from "react-router-dom";
import "../css/Documentation.css";

export default class SPTK_about extends React.Component
{
    render()
    {
        return <div className="SPTK" style={{textAlign: "left", padding: 8}}>
            <Seo title="SPTK — the C++ Class Library Behind XMQ"
                 description="SPTK (Simply Powerful Toolkit) is the cross-platform C++20 class library used to build the XMQ MQTT server: networking, threading, database access, XML/JSON and GUI."
                 path="/sptk_about"/>
            <h1>SPTK — the library behind XMQ</h1>
            <p>
                SPTK (Simply Powerful Toolkit) is the Open Source cross-platform C++20 class library
                that the <Link to="/xmq_about">XMQ MQTT server</Link> is built with. Everything XMQ
                needs below the MQTT protocol itself — non-blocking sockets and TLS, thread pools
                and timers, database access for message persistence, XML/JSON handling and logging —
                comes from SPTK. The MQTT server is, in that sense, the largest application built on
                the library and its primary proving ground.
            </p>

            <h3>Project Goals</h3>
            <p>
                SPTK implements the classes that are usually needed when a C++ application
                should work in more than one operating system.
                The range of classes includes networking, threads, database access, XML/JSON
                documents, web services and GUI.
                The aim of the project is to keep it as compact as possible.
            </p>

            <h3>Supported Operating Systems</h3>
            <p>
                The library supports Linux (.deb and .rpm flavours), BSD, and MS Windows.
                Unlike XMQ, which is distributed as binary packages only, SPTK is available
                in both source code and binary packages from
                the <Link to="/downloads">Downloads</Link> page.
            </p>

        </div>;
    }
}
