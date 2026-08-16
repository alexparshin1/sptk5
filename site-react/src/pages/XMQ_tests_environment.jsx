import React from "react";
import Seo from "../components/Seo";
import {Link} from "react-router-dom";
import "../css/Documentation.css";

export default class XMQ_tests_environment extends React.Component
{
    render()
    {
        const cell = {border: "1px solid #ccc", padding: "4px 8px", verticalAlign: "top"};
        const head = {...cell, background: "#f0f0f0", fontWeight: "bold", textAlign: "left"};
        const mono = {fontFamily: "monospace", fontSize: "0.92em"};
        const table = {borderCollapse: "collapse", margin: "8px 0 16px 0"};

        return <div key="tests-environment" className="XMQ" style={{textAlign: "left", padding: 8}}>
            <Seo title="MQTT Performance Tests — Test Environment"
                 description="The hardware, network and software environment used for the XMQ MQTT performance tests, so that the published MQTT benchmark results can be reproduced."
                 keywords="MQTT performance tests, MQTT test suite, MQTT benchmark, XMQ"
                 path="/xmq_tests_environment"/>
            <h1>MQTT Performance Tests: Test Environment</h1>
            <h3>Test Environment</h3>

            <p>
                All published XMQ benchmark results are produced on AWS EC2, using a dedicated
                server instance and a separate client instance. Load generators are never run on
                the broker host: a client competing for the same cores inflates latency and makes
                the result impossible to attribute. Every figure quoted in the test pages was
                measured with the client on its own instance.
            </p>

            <h4>AWS configuration</h4>

            <table style={table}>
                <tbody>
                <tr><td style={head}>Role</td><td style={head}>Instance</td><td style={head}>vCPU</td><td style={head}>Network</td></tr>
                <tr>
                    <td style={cell}>Broker</td>
                    <td style={cell}>c5n.4xlarge</td>
                    <td style={cell}>16 (8 physical cores, 2 threads each)</td>
                    <td style={cell}>Up to 25 Gbps, ENA enabled</td>
                </tr>
                <tr>
                    <td style={cell}>Load generator</td>
                    <td style={cell}>c5n.4xlarge</td>
                    <td style={cell}>16 (8 physical cores, 2 threads each)</td>
                    <td style={cell}>Up to 25 Gbps, ENA enabled</td>
                </tr>
                </tbody>
            </table>

            <ul>
                <li>
                    <b>Single availability zone.</b> Both instances run in <span style={mono}>us-east-1a</span>.
                    Cross-AZ traffic would add roughly a millisecond of round trip on its own, which
                    is several times the latency being measured.
                </li>
                <li>
                    <b>Cluster placement group.</b> Both instances are members of the
                    <span style={mono}> xmq-load-test </span> cluster placement group, packing them
                    onto the same low-latency network segment for consistent, high bandwidth between them.
                </li>
                <li>
                    <b>Processor.</b> Intel Xeon Platinum 8124M (Skylake-SP) at 3.0 GHz base.
                </li>
                <li>
                    <b>Multiple source addresses.</b> The client binds connections across 30 secondary
                    private IP addresses. A single address exhausts its ~64K ephemeral ports well
                    before the connection counts these tests use, and port allocation slows sharply
                    long before that limit is reached.
                </li>
                <li>
                    <b>Operating system.</b> Ubuntu 26.04 LTS, kernel 7.0.0-aws.
                </li>
            </ul>

            <h4>Broker tuning</h4>

            <p>
                Every broker is tuned before measurement, not run at its packaged defaults. A
                benchmark that leaves any broker misconfigured proves nothing.
            </p>

            <ul>
                <li>
                    <b>EMQX</b> is configured following the official
                    <a href="https://docs.emqx.com/en/emqx/latest/performance/tune.html"
                       target="_blank" rel="noreferrer"> EMQX performance tuning guide</a>, and the
                    OS settings below are taken from it. Its Erlang VM settings were left at the
                    shipped values after testing confirmed they are the right choice at these message
                    rates: raising the scheduler busy-wait threshold
                    (<span style={mono}>+sbwt</span>) improves latency below roughly 20K messages per
                    second but costs throughput above it, and reducing the scheduler count to match
                    physical cores was substantially worse.
                </li>
                <li>
                    <b>Mosquitto</b> runs with <span style={mono}>set_tcp_nodelay true</span>,
                    unlimited <span style={mono}>max_inflight_messages</span> and
                    <span style={mono}> max_queued_messages</span>, persistence disabled, and a raised
                    file descriptor limit. Mosquitto's defaults for the two queue limits throttle
                    delivery at high publisher counts, and its default leaves Nagle's algorithm enabled.
                </li>
                <li>
                    <b>FlashMQ</b> runs with <span style={mono}>tcp_nodelay true</span>,
                    <span style={mono}> thread_count 16</span> (one worker per vCPU, which is also what
                    its auto-detection chooses on this host), and
                    <span style={mono}> log_level warning</span> with subscription and publish logging
                    off &mdash; at the lower levels it logs every connection and every publish, which at
                    these rates measures the log rather than the broker. Its
                    <span style={mono}> max_qos_msg_pending_per_client</span> and
                    <span style={mono}> max_qos_bytes_pending_per_client</span> are raised from their
                    512-message and 64&nbsp;kB defaults to 65535 and 256&nbsp;MB: at the defaults QoS 1
                    traffic is dropped when a subscriber falls behind, which would be recorded as
                    throughput the broker never delivered. This is the same reasoning behind the
                    unbounded queues given to Mosquitto.
                </li>
            </ul>

            <h4>Kernel settings</h4>

            <p>
                Applied identically to broker and client. The system tuning is similar to that
                described in the
                <a href="https://docs.emqx.com/en/emqx/latest/performance/tune.html"
                   target="_blank" rel="noreferrer"> EMQX performance tuning guide</a>, and the same
                host configuration is used for every broker tested, so none is advantaged by it.
            </p>

            <table style={table}>
                <tbody>
                <tr><td style={head}>Setting</td><td style={head}>Value</td><td style={head}>Purpose</td></tr>
                <tr><td style={{...cell, ...mono}}>fs.file-max</td><td style={{...cell, ...mono}}>2097152</td>
                    <td style={cell} rowSpan="2">One file descriptor per connection, plus headroom.</td></tr>
                <tr><td style={{...cell, ...mono}}>fs.nr_open</td><td style={{...cell, ...mono}}>2097152</td></tr>
                <tr><td style={{...cell, ...mono}}>net.core.somaxconn</td><td style={{...cell, ...mono}}>65535</td>
                    <td style={cell} rowSpan="3">Accept and receive queues deep enough that a burst of
                        connections is not dropped before the broker accepts them.</td></tr>
                <tr><td style={{...cell, ...mono}}>net.ipv4.tcp_max_syn_backlog</td><td style={{...cell, ...mono}}>65535</td></tr>
                <tr><td style={{...cell, ...mono}}>net.core.netdev_max_backlog</td><td style={{...cell, ...mono}}>16384</td></tr>
                <tr><td style={{...cell, ...mono}}>net.ipv4.ip_local_port_range</td><td style={{...cell, ...mono}}>1024 65535</td>
                    <td style={cell}>Full ephemeral range on the client, used together with the
                        secondary addresses above.</td></tr>
                <tr><td style={{...cell, ...mono}}>net.core.rmem_max</td><td style={{...cell, ...mono}}>16777216</td>
                    <td style={cell} rowSpan="4">Socket buffer ceilings. The minimum stays small so
                        that hundreds of thousands of mostly idle sockets do not consume the memory
                        the ceiling permits.</td></tr>
                <tr><td style={{...cell, ...mono}}>net.core.wmem_max</td><td style={{...cell, ...mono}}>16777216</td></tr>
                <tr><td style={{...cell, ...mono}}>net.ipv4.tcp_rmem</td><td style={{...cell, ...mono}}>1024 4096 16777216</td></tr>
                <tr><td style={{...cell, ...mono}}>net.ipv4.tcp_wmem</td><td style={{...cell, ...mono}}>1024 4096 16777216</td></tr>
                <tr><td style={{...cell, ...mono}}>net.ipv4.tcp_mem</td>
                    <td style={{...cell, ...mono}}>1048576 1572864 2097152</td>
                    <td style={cell}>Global ceiling on TCP buffer memory, in 4&nbsp;KB pages
                        (4/6/8&nbsp;GB). The kernel derives its default from installed RAM, which lands
                        near 2.8&nbsp;GB on a 30&nbsp;GB host — low enough that a message-passing test at
                        high connection counts can reach the pressure threshold and start throttling.
                        Connection-count tests do not come close: idle sockets hold almost no buffer
                        memory, and measured use stayed under 6&nbsp;MB during a 1M-connection run.</td></tr>
                <tr><td style={{...cell, ...mono}}>net.ipv4.tcp_max_tw_buckets</td><td style={{...cell, ...mono}}>2000000</td>
                    <td style={cell} rowSpan="3">Closing a million connections at the end of a run
                        leaves an enormous number of sockets in TIME_WAIT; without these, the next run
                        cannot allocate source ports for several minutes.
                        <span style={mono}> tcp_tw_reuse </span> must be 1 rather than 2 — on kernels
                        5.10 and later the value 2 restricts reuse to loopback traffic, which does
                        nothing for a client connecting to a separate broker host.</td></tr>
                <tr><td style={{...cell, ...mono}}>net.ipv4.tcp_fin_timeout</td><td style={{...cell, ...mono}}>15</td></tr>
                <tr><td style={{...cell, ...mono}}>net.ipv4.tcp_tw_reuse</td><td style={{...cell, ...mono}}>1</td></tr>
                </tbody>
            </table>

            <p>
                Transparent huge pages are left at <span style={mono}>madvise</span>, so they are used
                only where explicitly requested rather than assembled in the background, and swap is
                disabled. Both avoid latency spikes that would otherwise appear as unexplained outliers.
            </p>

            <h4>Process limits</h4>

            <p>
                The <span style={mono}>fs.nr_open</span> and <span style={mono}>fs.file-max</span>
                settings above are only the system-wide ceiling. The per-process limit is separate, and
                it is the one a run actually hits — a connection costs one descriptor, so the limit has
                to exceed the connection count with room for listeners, the event reactor and any
                database sockets. Both hosts set:
            </p>

            <table style={table}>
                <tbody>
                <tr><td style={head}>Mechanism</td><td style={head}>Value</td><td style={head}>Applies to</td></tr>
                <tr><td style={{...cell, ...mono}}>/etc/security/limits.d/mqtt.conf</td>
                    <td style={{...cell, ...mono}}>nofile 2000000</td>
                    <td style={cell}>Login sessions, and therefore any broker or load generator
                        started from a shell.</td></tr>
                <tr><td style={{...cell, ...mono}}>LimitNOFILE</td>
                    <td style={{...cell, ...mono}}>2000000</td>
                    <td style={cell}>Set explicitly on any systemd unit. The
                        <span style={mono}> DefaultLimitNOFILE </span> a distribution ships is
                        typically well below 1M, and services do not inherit the limits above.</td></tr>
                </tbody>
            </table>

            <p>
                Two details are worth stating because both produce failures that look like broker
                faults rather than host misconfiguration. A per-user
                <span style={mono}> ulimit -n </span> in a shell profile sets the <i>hard</i> limit as
                well as the soft one, so a value below the figures above silently caps every
                subsequent run and cannot be raised again by an unprivileged process. And a limit
                equal to the connection count is not enough: the broker's own listener and reactor
                descriptors have to fit too, so a run targeting exactly the limit stops a few
                connections short and reports them as stalled handshakes.
            </p>

            <p>
                One further setting applies only where connection tracking is active — a host running
                Docker or libvirt, which load <span style={mono}>nf_conntrack</span> whether or not
                anything is being filtered. Each connection then consumes a tracking entry against
                <span style={mono}> net.netfilter.nf_conntrack_max</span>, whose RAM-derived default is
                around 262K. Beyond it the kernel drops packets silently, which presents as connect
                timeouts and unreachable-server errors from the client while the broker itself sits
                idle. Where this applies, the MQTT ports are exempted from tracking outright rather
                than the maximum simply raised, which also keeps the per-packet tracking lookup out of
                the path being measured.
            </p>

            <h4>Load generator and scenarios</h4>

            <p>
                Every figure on these pages is produced by <span style={mono}>xmq_scn</span>, the
                scenario runner shipped as part of the XMQ server installation &mdash; the same
                binary that is on any machine where XMQ is installed, not a private harness. It
                drives every broker identically: nothing in it is XMQ-specific, it speaks plain
                MQTT 3.1.1/5.0, and the broker under test is chosen with nothing more than a host
                and port. See the <Link to="/xmq_mqtt_test_suite">MQTT Test Suite</Link> page for
                a full description of <span style={mono}>xmq_scn</span> &mdash; its scenario
                format, command-line options and output.
            </p>

            <p>
                A test is a JSON scenario file: a type
                (<span style={mono}>Point-To-Point</span>, <span style={mono}>Fan-In</span>,
                {" "}<span style={mono}>Fan-Out</span> or <span style={mono}>Connections</span>),
                publisher and subscriber counts, topic count, QoS, payload size, publish or
                connection rate, and duration. Because the broker is only a host and port, the
                identical file runs against each broker in turn.
            </p>

            <p>
                The supplied scenarios deliberately reproduce the Basic and Enterprise sets from
                EMQX's published broker benchmark, so results here can be read against theirs
                rather than only against each other. The file names encode the parameters:
            </p>

            <table style={table}>
                <tbody>
                <tr><td style={head}>EMQX scenario</td><td style={head}>Parameters</td><td style={head}>Scenario file</td></tr>
                <tr><td style={cell} rowSpan="4"><b>Basic</b></td>
                    <td style={cell}>Point-to-point, 1K publishers / 1K subscribers / 1K topics, 1 msg/s each</td>
                    <td style={{...cell, ...mono}}>Point-To-Point-1K-1K-1K-1K.json</td></tr>
                <tr><td style={cell}>Fan-out, 1 publisher / 1 topic / 1000 subscribers, 1 msg/s</td>
                    <td style={{...cell, ...mono}}>Fan-Out-1-1k-1-1K.json</td></tr>
                <tr><td style={cell}>Fan-in, 1K publishers / 1K topics / 5 shared subscribers, 1K msg/s</td>
                    <td style={{...cell, ...mono}}>Fan-In-1K-5-1K-1K.json</td></tr>
                <tr><td style={cell}>Connections, 10K connections at 100/s</td>
                    <td style={{...cell, ...mono}}>Connections-10k-100.json</td></tr>
                <tr><td style={cell} rowSpan="4"><b>Enterprise</b></td>
                    <td style={cell}>Point-to-point, 50K publishers / 50K subscribers / 50K topics, 1 msg/s each</td>
                    <td style={{...cell, ...mono}}>Point-To-Point-50K-50K-50K-50K.json</td></tr>
                <tr><td style={cell}>Fan-out, 5 publishers / 5 topics / 1000 subscribers, 250 msg/s each</td>
                    <td style={{...cell, ...mono}}>Fan-Out-5-1000-5-250K.json</td></tr>
                <tr><td style={cell}>Fan-in, 50K publishers / 50K topics / 500 shared subscribers, 50K msg/s</td>
                    <td style={{...cell, ...mono}}>Fan-In-50K-500-50K-50K.json</td></tr>
                <tr><td style={cell}>Connections, 1M connections at 5000/s</td>
                    <td style={{...cell, ...mono}}>Connections-1M-5k.json</td></tr>
                </tbody>
            </table>

            <p>
                Additional intermediate scenarios (for example point-to-point at 10K, 20K and 35K
                pairs, and connection tests at 100K and 500K) fill in the gap between the two sets,
                where a broker's behaviour often changes.
            </p>

            <p>
                Around <span style={mono}>xmq_scn</span> sit three shell scripts, so a run is
                reproducible rather than a remembered command line.
                {" "}<span style={mono}>run_load_test.sh</span> is the entry point: it confirms the
                broker is reachable, optionally applies the client-side kernel tuning above, brings
                up the secondary IP addresses, then invokes <span style={mono}>xmq_scn</span> with
                the scenario and any overrides. <span style={mono}>make_ip_addresses.sh</span>
                assigns the secondary addresses, and a per-environment
                {" "}<span style={mono}>init_environment_*.sh</span> supplies the broker host, port
                and client subnet through environment variables &mdash; so the scenario files
                themselves stay free of any site-specific addressing and are used unmodified
                everywhere.
            </p>

            <p>
                For the fuller statement of the conditions these scenarios come from, see EMQX's
                {" "}<a href="https://www.emqx.com/en/blog/open-mqtt-benchmarking-comparison-mqtt-brokers-in-2023"
                        target="_blank" rel="noreferrer">open MQTT benchmark</a> and the
                {" "}<a href="https://github.com/emqx/mqttbs/tree/main/results"
                        target="_blank" rel="noreferrer">Open MQTT Benchmark Suite results</a> the
                scenario definitions are taken from &mdash; the file names above follow that
                repository's own naming, so each scenario here has a direct counterpart there.
                Both describe a single c5.4xlarge (16 cores, 32&nbsp;GB, Ubuntu 22.04) driven by
                XMeter, comparing EMQX 4.4.16 and 5.0.21, Mosquitto 2.0.15 and NanoMQ 0.17.0.
            </p>

            <p>
                <b>Those results are from 2023 and should be read as a definition of the
                scenarios, not as a current baseline.</b> Every broker version in them is by now
                several releases old, which is why the brokers compared on these pages are re-run
                here at current versions rather than quoted from that publication.
            </p>

            <p>
                <b>Their published latencies could not be reproduced here.</b> For the 50K
                point-to-point scenario the suite reports 1.68&nbsp;ms average for EMQX&nbsp;4.4.16;
                the same scenario, re-run for these pages, puts EMQX at 53&nbsp;ms &mdash; roughly
                thirty times higher.
            </p>

            <p>
                The hardware is not the explanation. Their <span style={mono}>c5.4xlarge</span> and
                the <span style={mono}>c5n.4xlarge</span> used here are both 16&nbsp;vCPU Xeon
                Platinum 8124M (Skylake-SP); the <span style={mono}>n</span> variant differs only in
                carrying more memory and 25&nbsp;Gbps of network against 10, so the runs here had, if
                anything, the more capable machine. What does differ is the broker build &mdash;
                their EMQX 4.4.16 and 5.0.21 against a later 5.x release, installed from the
                vendor's own package repository &mdash; and the load generator, XMeter against
                {" "}<span style={mono}>xmq_scn</span>, which also means a different point at which
                latency is timestamped. Either could account for some of a gap this size; neither is
                verifiable from the published material, which does not state where its load
                generator ran.
            </p>

            <p>
                This is not offered as a correction to their figures, and the scenario definitions
                remain useful regardless. It is the reason every number on these pages is a
                measurement taken here, with its conditions stated, rather than a citation of
                someone else's. Where the two do agree is on Mosquitto: their results likewise
                record it failing to reach the target rate in this scenario, settling at 37.3K
                messages/second against the 50K offered.
            </p>

            <p>
                Two further differences are worth keeping in mind when reading the two side by
                side: the results here use a separate instance for the load generator, and they are
                reported as per-interval averages across the run rather than as one figure per test.
            </p>

            <h4>Method</h4>

            <ul>
                <li>
                    <b>Identical client and scenario for every broker.</b> Only the broker changes
                    between runs. Where a broker's own published figures differ from those measured
                    here, this is the difference most likely to explain it: published benchmarks
                    frequently omit how load was generated.
                </li>
                <li>
                    <b>Results reported as interval averages</b> across the whole run, not as a single
                    figure, so that latency drifting upward over time is visible rather than averaged
                    away. Run duration is stated on each test page, since it varies by test.
                </li>
                <li>
                    <b>Brokers restarted between runs</b> so that session state from a previous run
                    cannot carry into the next.
                </li>
            </ul>
        </div>;
    }
}
