import React from "react";
import "../css/Header.css";

export default class Footer extends React.Component
{
    state = {
        fileName: ""
    };

    render()
    {
        var gaJsHost = (("https:" === document.location.protocol) ? "https://ssl." : "http://www.") + "google-analytics.com/ga.js";
        return <div className="Footer">

            <div id="bottom">
                If you have any questions or comments regarding this page feel free to drop a line
                to <a href="mailto:alexey.parshin at linotex.net">Alexey Parshin</a>.<br/>Design by <a
                href="mailto:perlovka at gmail.com">Michael Perlov</a>.
            </div>

            <script type='text/javascript' src={gaJsHost}></script>
            <script type="text/javascript">
                var pageTracker = _gat._getTracker("UA-5656238-1");
                pageTracker._trackPageview();
            </script>
        </div>;
    }
}
