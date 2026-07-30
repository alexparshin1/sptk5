import React from "react";
import "../css/Documentation.css";

export default class XMQ_mqtt_test_suite extends React.Component
{
    render()
    {
        return <div key="mqtt-test-suite-menu" className="XMQ" style={{textAlign: "left", padding: 8}}>
            <p>
                XMQ is distributed with a set of command-line utilities used to load-test and
                functionally test MQTT brokers, XMQ included. They implement MQTT protocol
                versions 3.1, 3.1.1, and 5.0, so they can be pointed at any compliant broker
                (e.g. XMQ, eMQX, Mosquitto) for an apples-to-apples comparison. All of them share
                a common set of connection options, such as <code>--host</code>/<code>--port</code>,
                {" "}<code>--username</code>/<code>--password</code>, <code>--cafile</code>/<code>--cert</code>/<code>--key</code> for
                TLS, <code>--client-id</code>, <code>--protocol-version</code>, and <code>--sessions</code> for
                opening several client sessions from a single process.
            </p>

            <h3>xmq_con &mdash; connection tester</h3>
            <p>
                Opens a number of client connections to the broker, either as fast as possible or
                at a controlled rate, and reports how quickly the connections are established.
                It doesn't publish or subscribe to anything &mdash; it is a pure connect/disconnect
                load generator used to find out how many clients per second a broker can accept,
                and to compare that rate across brokers.
            </p>
            <ul>
                <li><code>--sessions</code>/<code>-n</code> &mdash; number of client sessions to connect.</li>
                <li><code>--connect-rate</code>/<code>-R</code> &mdash; connections per second; unpaced when omitted.</li>
                <li><code>--show-counters</code> / <code>--show-counters-csv</code> &mdash; print progress counters
                    as plain text or CSV while the test is running.
                </li>
            </ul>

            <h3>xmq_pub &mdash; publisher</h3>
            <p>
                Connects one or more publishing clients and sends messages to one or more topics.
                It is used both to generate publish load and to functionally verify that a broker
                accepts and routes messages correctly.
            </p>
            <ul>
                <li><code>--topic</code>/<code>-t</code> &mdash; one or more topics (separated by &lsquo;;&rsquo;); a topic
                    may contain <code>%ClientIndex%</code>, substituted with the client's index.
                </li>
                <li>Message source: <code>--message</code>/<code>-m</code> (inline text), <code>--message-file</code>/<code>-f</code> (file
                    contents), or <code>--stdin</code>/<code>-s</code> (one message per line).
                </li>
                <li><code>--qos</code>/<code>-q</code>, <code>--repeat</code>/<code>-r</code> &mdash; QoS level and how many times
                    the message is sent.
                </li>
                <li><code>--message-rate</code>/<code>-g</code> &mdash; publish rate in messages per second; unpaced
                    when omitted.
                </li>
                <li><code>--max-inflight</code>/<code>-M</code> &mdash; maximum in-flight messages for QoS 1/2.</li>
                <li><code>--send-timestamp</code>/<code>-l</code> &mdash; injects a send timestamp at the start of each
                    message, used by xmq_sub to measure end-to-end latency.
                </li>
                <li><code>--session-expiry-interval</code>/<code>-x</code>, <code>--disable-clean-session</code>/<code>-c</code> &mdash; MQTT
                    session persistence controls.
                </li>
            </ul>
            <p>
                MQTT 5 CONNECT and PUBLISH properties (such as <code>message-expiry-interval</code> or
                {" "}<code>user-property</code>) can be set with repeated <code>-D</code> arguments; the full
                property list is printed by <code>xmq_pub --help</code>.
            </p>

            <h3>xmq_sub &mdash; subscriber</h3>
            <p>
                Connects one or more subscribing clients to one or more topics and receives
                messages, used to verify delivery and, together with xmq_pub's
                {" "}<code>--send-timestamp</code>, to measure publish-to-receive latency.
            </p>
            <ul>
                <li><code>--topic</code>/<code>-t</code> &mdash; one or more topics to subscribe to, with the same
                    &lsquo;;&rsquo; and <code>%ClientIndex%</code> support as xmq_pub.
                </li>
                <li><code>--qos</code>/<code>-q</code> &mdash; subscription QoS level.</li>
                <li><code>--receive-count</code>/<code>-C</code> &mdash; disconnect and exit after receiving this many
                    messages.
                </li>
                <li><code>--disconnect-after</code>/<code>-W</code> &mdash; disconnect and exit after waiting this many
                    seconds for messages.
                </li>
                <li><code>--print-messages</code>/<code>-v</code> &mdash; print each received message to stdout.</li>
            </ul>

            <h3>xmq_scn &mdash; scenario runner</h3>
            <p>
                Runs a complete multi-client test &mdash; a mix of publishers, subscribers, and a
                target broker &mdash; described by a single JSON scenario file, instead of requiring
                separate xmq_con/xmq_pub/xmq_sub processes to be started and coordinated by hand.
                Any value from the scenario file (rate, duration, QoS, broker address, and so on)
                can be overridden from the command line for a one-off run.
            </p>
            <ul>
                <li><code>--scenario</code>/<code>-s</code> &mdash; scenario JSON file. An absolute path is used as
                    is; a relative path is looked up in the current directory first, then in the
                    installed scenario directory (<code>share/xmq</code> next to the xmq_scn executable).
                </li>
                <li><code>--list-scenarios</code> &mdash; list the scenario files installed under
                    {" "}<code>share/xmq</code>, grouped by sub-directory, and exit.
                </li>
                <li><code>--publish-rate</code>/<code>-r</code>, <code>--publish-count</code>/<code>-C</code>,
                    {" "}<code>--duration</code>/<code>-d</code>, <code>--connection-rate</code>/<code>-R</code>,
                    {" "}<code>--payload-size</code>/<code>-m</code>, <code>--qos</code>/<code>-q</code> &mdash; override the matching
                    scenario parameters.
                </li>
                <li><code>--bind-to-interfaces</code>/<code>-I</code> &mdash; spread clients across several local
                    network interfaces (a comma-separated IP list, or a mask such as
                    {" "}<code>192.0.2.1/24</code>), needed to run more than 64K clients from one host.
                </li>
                <li><code>--id-prefix</code> &mdash; prepended to the publishers' and subscribers'
                    {" "}<code>id_prefix</code> from the scenario file, so client IDs don't collide when
                    the same scenario is run from several hosts against one broker.
                </li>
            </ul>

            <h4>Scenario file format</h4>
            <p>
                A scenario file is a JSON document with the following top-level sections:
            </p>
            <ul>
                <li><code>name</code> &mdash; scenario name, used in reports and by <code>--list-scenarios</code>.</li>
                <li><code>type</code> &mdash; one of <code>Connections</code>, <code>Point-To-Point</code>, <code>Fan-Out</code>,
                    {" "}<code>Fan-In</code>; selects the traffic pattern the scenario exercises.
                </li>
                <li><code>publishers</code> &mdash; the publishing client group: <code>id_prefix</code>,
                    {" "}<code>protocol_version</code>, <code>client_count</code>, <code>qos</code>, <code>clean_session</code>, and
                    {" "}<code>topics</code> (which may use <code>$clientid</code>/<code>$clientindex</code> substitution).
                </li>
                <li><code>subscribers</code> &mdash; the subscribing client group, same shape as
                    {" "}<code>publishers</code>; omitted for scenarios that don't subscribe (e.g. a pure
                    connection test). Its <code>topics</code> can use the shared-subscription syntax
                    {" "}<code>$share/&lt;group&gt;/&lt;filter&gt;</code> so several subscribers split one topic's
                    messages.
                </li>
                <li><code>server</code> &mdash; the broker under test: <code>hostname</code>, <code>port</code>,
                    {" "}<code>username</code>, <code>password</code>.
                </li>
                <li><code>parameters</code> &mdash; test sizing and pass/fail thresholds:
                    {" "}<code>message_count</code> or <code>duration_sec</code> (how long/how much to publish),
                    {" "}<code>payload_size</code>, <code>publish_rate</code>, <code>connection_rate</code>, and the
                    expected results <code>expected_connect_latency</code>/<code>expected_message_latency</code>
                    the run is compared against.
                </li>
            </ul>

            <p>The four scenario types model different delivery topologies:</p>
            <ul>
                <li><b>Connections</b> &mdash; clients only connect (and stay connected) at a given
                    connection rate; there are no publishers or subscribers, so it is a pure
                    connect-scaling test.
                </li>
                <li><b>Point-To-Point</b> &mdash; matching numbers of publishers and subscribers, each
                    pair using its own topic; models one-to-one message delivery.
                </li>
                <li><b>Fan-Out</b> &mdash; one (or a few) publisher(s) send to a single topic that many
                    subscribers listen to; models broadcast-style delivery.
                </li>
                <li><b>Fan-In</b> &mdash; many publishers, each on its own topic, consumed by a small
                    number of subscribers through a shared subscription; models fan-in
                    aggregation load.
                </li>
            </ul>

            <p>A Point-To-Point example, 1,000 publishers and 1,000 subscribers over 1,000 topics:</p>
            <pre style={{
                backgroundColor: "#eee",
                border: "1px solid #ccc",
                padding: 8,
                overflowX: "auto",
                fontSize: "0.85em"
            }}>{`{
  "name": "Point-To-Point-1K-1K-1K-1K",
  "type": "Point-To-Point",
  "publishers": {
    "id_prefix": "test-publisher-",
    "protocol_version": 3,
    "client_count": 1000,
    "qos": 1,
    "clean_session": true,
    "topics": "test/topic$clientindex"
  },
  "subscribers": {
    "id_prefix": "test-subscriber-",
    "protocol_version": 3,
    "client_count": 1000,
    "qos": 1,
    "clean_session": true,
    "topics": "test/topic$clientindex"
  },
  "server": {
    "hostname": "localhost",
    "port": 1880,
    "username": "user",
    "password": "secret"
  },
  "parameters": {
    "duration_sec": 10,
    "payload_size": 16,
    "publish_rate": 1,
    "expected_message_latency": 2
  }
}`}</pre>
            <p>
                Run it with <code>xmq_scn --scenario Basic/Point-To-Point-1K-1K-1K-1K.json</code>, or
                override just the publish rate for one run with{" "}
                <code>xmq_scn --scenario Basic/Point-To-Point-1K-1K-1K-1K.json --publish-rate 10</code>.
            </p>
        </div>;
    }
}
