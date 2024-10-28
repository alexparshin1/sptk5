import React from "react";
import "../css/Documentation.css";

export default class SPTK_about extends React.Component
{
    render()
    {
        return <div className="SPTK" style={{textAlign: "left", padding: 8}}>
            <h3>Project Goals</h3>
            <p>
                SPTK is an Open Source cross-platform class library written in C++.
                It is implementing classes that are usually needed when a C++ application
                should work in more than one operating system.
                The range of classes includes GUI, threads, databases access, networking, etc.
                The aim of the project is to keep it as compact as possible.
            </p>

            <h3>Supported Operating Systems</h3>
            <p>
                The library supports Linux (.deb and .rpm flavours), BSD, and MS Windows.
                The binary files are available on Download page, along with source code downloads.
            </p>

        </div>;
    }
}
