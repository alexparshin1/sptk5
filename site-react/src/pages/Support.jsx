import React from "react";
import Seo from "../components/Seo";
import "../css/Documentation.css";

export default class Support extends React.Component
{
    state = {
        fileName: ""
    };

    render()
    {
        return <div className="Documentation">
            <Seo title="Support — XMQ MQTT Server and SPTK"
                 description="Commercial support and pricing for the XMQ MQTT server and the SPTK cross-platform C++ class library."
                 path="/support"/>
            <h1>XMQ and SPTK Support</h1>
            <div id="fadeout"/>
            <h3> Free Technical Support </h3>
            <p> Free technical support for the XMQ MQTT server and the SPTK library is provided by
                the development team.
                Since team members are extremely busy, we can no guarantee fast response.
                Any reported bugs are considered. If core functionality is broken, then bugs are fixed ASAP.
                Bugs with less importance are included into development plans.
                We also provide e-mail consultations on XMQ and SPTK usage.
            </p>
            <hr/>
            <h3> Commercial Technical Support </h3>
            <p> Commercial technical support for XMQ and SPTK is provided by Linotex Pty Ltd, Melbourne.
                This company is one of the project sponsors.
                All the services are e-mail based. Only Linux, Windows, and Solaris are presently supported.
                The following support services are currently offered:
            </p>
            <br/>
            <table style={{padding: 4, border: 1}}>
                <thead>
                <tr>
                    <th>Support Service</th>
                    <th width="400">Service includes</th>
                    <th>Price</th>
                </tr>
                </thead>
                <tbody>
                <tr bgcolor="#eee" valign="top">
                    <td> Regular Support (Linux)</td>
                    <td> Reported bugs are fixed within 72 hours since reported. Up to three incidents per
                        month.
                        Unlimited e-mail consultations on XMQ and SPTK usage.
                    </td>
                    <td> $50 US/mon, or $500 US/year</td>
                </tr>
                <tr bgcolor="#eee" valign="top">
                    <td> Regular Support (Windows,Solaris)</td>
                    <td> Reported bugs are fixed within 72 hours since reported. Up to three incidents per
                        month.
                        Unlimited e-mail consultations on XMQ and SPTK usage.
                    </td>
                    <td> $60 US/mon, or $600 US/year</td>
                </tr>
                <tr bgcolor="#eee" valign="top">
                    <td> Premium Support (Linux)</td>
                    <td> Reported bugs are fixed within 24 hours since reported. Up to five incidents per
                        month.
                        Unlimited e-mail consultations on XMQ and SPTK.
                    </td>
                    <td> $200 US/mon, or $1600 US/year</td>
                </tr>
                <tr bgcolor="#eee" valign="top">
                    <td> Premium Support (Windows,Solaris)</td>
                    <td> Reported bugs are fixed within 24 hours since reported. Up to five incidents per
                        month.
                        Unlimited e-mail consultations on XMQ and SPTK.
                    </td>
                    <td> $240 US/mon, or $1800 US/year</td>
                </tr>
                </tbody>
            </table>
            <br/>
            <p>
                If you need more information about XMQ or SPTK support, or different support options, please
                contact the <a href="mailto:alexeyp at gmail.com">project coordinator</a>.
            </p>
        </div>;
    }
}
