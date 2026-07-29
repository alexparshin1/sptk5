import React from "react";
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

            <h4>Competitor tuning</h4>

            <p>
                Competing brokers are tuned before measurement, not run at their packaged defaults.
                A benchmark that leaves a competitor misconfigured proves nothing.
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
            </ul>

            <h4>Kernel settings</h4>

            <p>
                Applied identically to broker and client. These come from the EMQX tuning guide and
                are used for every broker tested, so no broker is advantaged by the host configuration.
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
                <tr><td style={{...cell, ...mono}}>net.ipv4.tcp_max_tw_buckets</td><td style={{...cell, ...mono}}>1048576</td>
                    <td style={cell} rowSpan="3">Connections closed at the end of a run leave large
                        numbers of sockets in TIME_WAIT; these keep that from blocking the next run.</td></tr>
                <tr><td style={{...cell, ...mono}}>net.ipv4.tcp_fin_timeout</td><td style={{...cell, ...mono}}>15</td></tr>
                <tr><td style={{...cell, ...mono}}>net.ipv4.tcp_tw_reuse</td><td style={{...cell, ...mono}}>2</td></tr>
                </tbody>
            </table>

            <p>
                Transparent huge pages are left at <span style={mono}>madvise</span>, so they are used
                only where explicitly requested rather than assembled in the background, and swap is
                disabled. Both avoid latency spikes that would otherwise appear as unexplained outliers.
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
