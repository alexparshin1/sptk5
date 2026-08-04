import React from "react";
import "./UserManual.css";

/**
 * XMQ user manual.
 *
 * Served both from the server's own configuration interface and from the SPTK website, from
 * the same source: this file and UserManual.css are copied verbatim into the site's
 * site-react/src/pages/. It therefore carries no dependency on either application - no
 * ControlAPI, no shared components - and its wording names the configuration interface
 * rather than saying "this interface", which would be wrong in one of the two places.
 *
 * The XMQ repository is the original; the website copy is downstream of it.
 */
export default class UserManualScreen extends React.Component {

    renderContents() {
        const sections = [
            ["supported-oses", "Supported operating systems"],
            ["install", "Download and install binaries"],
            ["tuning", "Environment tuning"],
            ["basic-setup", "Basic MQTT setup"],
            ["advanced-setup", "Advanced setup"],
            ["testing-suite", "MQTT testing suite"],
            ["testing-setup", "Testing your setup"]
        ];

        return <div className="userManualContents">
            <ul>
                {sections.map(([anchor, title]) =>
                    <li key={anchor}><a href={"#" + anchor}>{title}</a></li>)}
            </ul>
        </div>;
    }

    renderSupportedOses() {
        return <div>
            <h4 id="supported-oses">Supported operating systems</h4>

            <p>
                XMQ is built and packaged for the systems below. Each is a 64-bit x86 build,
                packaged in the format native to that system.
            </p>

            <table className="userManualTable">
                <thead>
                <tr>
                    <th>System</th>
                    <th>Versions</th>
                    <th>Package</th>
                </tr>
                </thead>
                <tbody>
                <tr><td>Debian</td><td>trixie, forky</td><td><code>.deb</code></td></tr>
                <tr><td>Ubuntu</td><td>25.04, 25.10, 26.04</td><td><code>.deb</code></td></tr>
                <tr><td>Fedora</td><td>42, 43, 44</td><td><code>.rpm</code></td></tr>
                <tr><td>Oracle Linux</td><td>10</td><td><code>.rpm</code></td></tr>
                <tr><td>Windows</td><td>10, 11, Server</td><td>installer</td></tr>
                </tbody>
            </table>

            <p>
                Other Linux distributions of comparable vintage generally work when built from
                source, but only the versions listed here are tested and packaged.
            </p>
        </div>;
    }

    renderInstall() {
        return <div>
            <h4 id="install">Download and install binaries</h4>

            <p>
                The only official download location is <a href="https://sptk.net/Downloads">here</a>.
                Select the latest SPTK version and your operating system, and download xmq_server package.
            </p>

            <p>
                Packages are named <code>XMQ-server</code> and install under <code>/usr/local</code>
                on Linux. The server, the utilities, the configuration interface, and the load-test scenarios all
                come from that one package.
            </p>

            <p>
                The dependencies for Linux include brotli and pcre2 packages that are included in all major 
                Linux distributions. The dependencies for Windows are installed with XMQ.
            </p>

            <p>
                The package file name carries the version and the architecture, so the commands
                below use a wildcard. Run them in the directory the package was downloaded to.
            </p>

            <h5>Debian and Ubuntu</h5>
            <pre className="userManualCode">{`sudo apt install ./xmq-server_*.deb`}</pre>

            <h5>Fedora and Oracle Linux</h5>
            <pre className="userManualCode">{`sudo dnf install ./XMQ-server-*.rpm`}</pre>

            <p>
                Installing through the package manager rather than <code>dpkg -i</code> or
                <code>rpm -i</code> lets it resolve the shared libraries the package depends on.
            </p>

            <h5>What gets installed</h5>
            <table className="userManualTable">
                <thead>
                <tr>
                    <th>Path</th>
                    <th>Contents</th>
                </tr>
                </thead>
                <tbody>
                <tr>
                    <td><code>/usr/local/bin</code></td>
                    <td><code>xmq_server</code> and the <code>xmq_pub</code>, <code>xmq_sub</code>,
                        <code>xmq_con</code>, <code>xmq_scn</code> utilities</td>
                </tr>
                <tr>
                    <td><code>/etc/xmq/xmq_server.conf</code></td>
                    <td>Server configuration. An existing file is never overwritten by an upgrade</td>
                </tr>
                <tr>
                    <td><code>/etc/xmq/xmq_server.conf.template</code></td>
                    <td>The shipped defaults, refreshed on every upgrade, for comparison</td>
                </tr>
                <tr>
                    <td><code>/etc/xmq/certs</code></td>
                    <td>TLS certificates and keys installed from the SSL Keys page</td>
                </tr>
                <tr>
                    <td><code>/var/log/xmq</code></td>
                    <td>Server log</td>
                </tr>
                <tr>
                    <td><code>/usr/local/share/xmq</code></td>
                    <td>Load-test scenarios, host tuning files, helper scripts</td>
                </tr>
                </tbody>
            </table>

            <p>
                On Windows the equivalents live under <code>C:\ProgramData\xmq</code>, with the
                program files in <code>C:\Program Files\xmq</code>.
            </p>

            <h5>Running the server</h5>
            <p>
                The package installs a systemd unit, so on Linux the server is managed the usual way:
            </p>
            <pre className="userManualCode">{`sudo systemctl enable --now xmq_server
sudo systemctl status xmq_server
sudo journalctl -u xmq_server -f`}</pre>

            <p>
                On Windows, <code>xmq_server --install-service</code> registers it as a Windows
                service, and <code>--uninstall-service</code> removes it.
            </p>

            <p>
                Useful options when running it by hand:
            </p>
            <table className="userManualTable">
                <thead>
                <tr>
                    <th>Option</th>
                    <th>Meaning</th>
                </tr>
                </thead>
                <tbody>
                <tr><td><code>-s</code>, <code>--console</code></td>
                    <td>Stay in the foreground instead of self-daemonizing. This is what the
                        systemd unit uses, so that systemd supervises the real process</td></tr>
                <tr><td><code>-c</code>, <code>--configuration-file</code></td>
                    <td>Use a configuration file other than the default</td></tr>
                <tr><td><code>-l</code>, <code>--log-file</code></td>
                    <td>Write the log somewhere other than the configured path</td></tr>
                <tr><td><code>--no-cpu-affinity</code></td>
                    <td>Stop pinning threads to physical cores. Affinity is the default and is
                        Linux-only; it understands CPU sets, so it cooperates with containers</td></tr>
                <tr><td><code>-d</code>, <code>--debug</code></td>
                    <td>Verbose logging, regardless of the configured levels</td></tr>
                <tr><td><code>--version</code></td>
                    <td>Print the version and exit</td></tr>
                </tbody>
            </table>

            <p>
                Once the server runs, the configuration interface is reachable on port 18883 by default:
                <code>http://&lt;server&gt;:18883</code>. Sign in with an administrative account;
                a fresh installation ships with <code>admin</code>.
            </p>

            <div className="userManualNote">
                Change the shipped passwords before putting the server on a network anyone else
                can reach. The defaults are published in the template file, so they are not secret.
            </div>
        </div>;
    }

    renderTuning() {
        return <div>
            <h4 id="tuning">Environment tuning</h4>

            <p>
                A default Linux install is tuned for a few thousand sockets, not a few hundred
                thousand. Below roughly 10,000 concurrent connections the stock settings are fine
                and this section can be skipped. Above that, the limits below are the ones that
                bite, and each of them has been hit in real test runs.
            </p>

            <p>
                Ready-made files are installed with the package under
                <code>/usr/local/share/xmq/setup</code>, along with a README describing the symptom
                each one addresses:
            </p>

            <table className="userManualTable">
                <thead>
                <tr>
                    <th>File</th>
                    <th>Install to</th>
                    <th>Purpose</th>
                </tr>
                </thead>
                <tbody>
                <tr>
                    <td><code>sysctl.d/mqtt.conf</code></td>
                    <td><code>/etc/sysctl.d/</code></td>
                    <td>File descriptor ceiling, socket buffers, accept queue depth, TCP memory</td>
                </tr>
                <tr>
                    <td><code>sysctl.d/port_range.conf</code></td>
                    <td><code>/etc/sysctl.d/</code></td>
                    <td>Widen the ephemeral port range to 10000-65535</td>
                </tr>
                <tr>
                    <td><code>limits.d/mqtt.conf</code></td>
                    <td><code>/etc/security/limits.d/</code></td>
                    <td><code>nofile</code> for login sessions</td>
                </tr>
                <tr>
                    <td><code>modules-load.d/nf_conntrack.conf</code></td>
                    <td><code>/etc/modules-load.d/</code></td>
                    <td>Load <code>nf_conntrack</code> before <code>systemd-sysctl</code> runs</td>
                </tr>
                <tr>
                    <td><code>nftables.d/xmq-notrack.nft</code></td>
                    <td><code>/etc/nftables.d/</code></td>
                    <td>Exempt the MQTT ports from connection tracking</td>
                </tr>
                </tbody>
            </table>

            <pre className="userManualCode">{`cd /usr/local/share/xmq/setup
sudo install -m 0644 sysctl.d/mqtt.conf       /etc/sysctl.d/mqtt.conf
sudo install -m 0644 sysctl.d/port_range.conf /etc/sysctl.d/port_range.conf
sudo install -m 0644 limits.d/mqtt.conf       /etc/security/limits.d/mqtt.conf
sudo modprobe nf_conntrack
sudo sysctl --system`}</pre>

            <p>
                Then log out and back in, so the <code>nofile</code> limit applies to your shell.
            </p>

            <h5>The three that catch people out</h5>

            <ul>
                <li>
                    <b>One descriptor per connection, plus a handful.</b> The server idles at 17
                    descriptors, so a limit of exactly 1,000,000 fails at 999,983 connections. The
                    shipped files set 2,000,000, which leaves room.
                </li>
                <li>
                    <b>Services do not read <code>limits.d</code>.</b> That file applies to login
                    sessions only. A systemd service gets <code>DefaultLimitNOFILE</code> instead,
                    commonly 524288. The XMQ unit sets <code>LimitNOFILE</code> explicitly for this
                    reason; any unit you write yourself must do the same.
                </li>
                <li>
                    <b>A <code>ulimit -n</code> in your shell profile wins.</b> If
                    <code>~/.profile</code> or <code>~/.bashrc</code> lowers it, an unprivileged
                    process cannot raise it again, and the limit files appear to have no effect.
                </li>
            </ul>

            <p>
                Both ends need this. Every limit here is per host, and tuning only the server while
                leaving the client machine at defaults is the most common mistake. The
                <code>preflight.sh</code> script in <code>/usr/local/share/xmq</code> checks a host
                and exits non-zero if a required setting is missing:
            </p>

            <pre className="userManualCode">{`./preflight.sh                          # check this host as a client
./preflight.sh -H broker -r server -p 1883   # check a remote server host`}</pre>
        </div>;
    }

    renderBasicSetup() {
        return <div>
            <h4 id="basic-setup">Basic MQTT setup</h4>

            <p>
                Everything in this section can be set from the configuration interface, and is written back to
                <code>/etc/xmq/xmq_server.conf</code>. Hand-editing that file is still possible;
                if you do, restart the server afterwards, since a running server rewrites the file
                when settings change and would overwrite your edit.
            </p>

            <h5>Port numbers</h5>

            <p>
                Listeners are configured on the <b>Listeners</b> page. Each one is a port, a
                protocol, and a thread count. The shipped configuration defines two:
            </p>

            <table className="userManualTable">
                <thead>
                <tr>
                    <th>Port</th>
                    <th>Protocol</th>
                    <th>Purpose</th>
                </tr>
                </thead>
                <tbody>
                <tr><td>1883</td><td>MQTT</td><td>Plain TCP, the standard MQTT port</td></tr>
                <tr><td>8883</td><td>MQTT+SSL</td><td>TLS, the standard secure MQTT port</td></tr>
                <tr><td>18883</td><td>HTTP</td><td>The configuration interface and its control API</td></tr>
                </tbody>
            </table>

            <p>
                These are the standard MQTT ports, so most clients need no port setting at all.
                If another broker on the same host already holds them, either stop it or move one
                of the two: the Listeners page changes XMQ's ports, and the change takes effect as
                soon as the listener is saved.
            </p>

            <p>
                A listener also carries a bind address. <code>0.0.0.0</code> accepts on every
                interface; naming one address restricts the listener to it, which is a simple way
                to keep a port on an internal network. Disabling a listener keeps its settings but
                closes its port.
            </p>

            <p>
                The service port itself is set on the <b>Service</b> page. Changing it takes effect
                after a restart, and the interface then answers on the new port only.
            </p>

            <h5>SSL certificates</h5>

            <p>
                An <code>MQTT+SSL</code> listener needs a server key and certificate. Install them
                from the <b>SSL Keys</b> page: choosing a file uploads its contents, and the server
                writes it into <code>/etc/xmq/certs</code>
                (<code>C:\ProgramData\xmq\certs</code> on Windows) and records the path. A previous
                file of the same name is kept beside the new one with an <code>.old</code> suffix.
            </p>

            <ul>
                <li><b>Server key</b> and <b>server certificate</b> are required for TLS.</li>
                <li><b>CA certificate</b> is needed only to verify client certificates.</li>
                <li>
                    <b>Verify depth</b> of 0 means clients are not asked for a certificate: the TLS
                    listener proves the server's identity and no more. Above 0, a client must
                    present a certificate that the CA certificate validates, within that many
                    intermediate signers.
                </li>
            </ul>

            <p>
                If the keys are missing or fail to load, the server logs an error, skips the
                MQTT+SSL listener, and carries on serving the plain one. That is worth checking in
                the log after installing certificates for the first time.
            </p>

            <h5>Logging</h5>

            <p>
                The <b>Logging</b> page sets the log file and the level per subject. The
                <b>minimum log level</b> is a ceiling rather than a floor: each subject has its own
                level, and the server clips it to this one. Turning the ceiling down to
                <code>INFO</code> quietens everything without touching the individual subjects,
                and turning it up to <code>DEBUG</code> lets them speak at their configured level.
            </p>

            <p>
                Levels are <code>PANIC</code>, <code>ERROR</code>, <code>WARNING</code>,
                <code>NOTICE</code>, <code>INFO</code>, <code>DEBUG</code>, from quietest to most
                verbose. Two subjects deserve care: <b>Publish</b> and <b>Acks</b> produce a line
                per message, not per client. At the rates XMQ is built for, <code>DEBUG</code>
                there can outweigh everything else the server logs and cost real throughput. Leave
                them at <code>ERROR</code> unless you are chasing something specific.
            </p>

            <p>
                Log rotation is left to the system. Session errors are always logged at
                <code>ERROR</code> and are not configurable.
            </p>

            <h5>Users and anonymous access</h5>

            <p>
                Accounts on the <b>Users</b> page serve both MQTT clients and the configuration interface. An
                administrator may change the configuration; a non-administrator can connect but not
                administer. <b>Allow anonymous</b> lets MQTT clients connect with no credentials at
                all; it does not affect the configuration interface, which always requires signing in.
            </p>

            <h5>Persistence</h5>

            <p>
                Without persistence the server keeps everything in memory and nothing survives a
                restart. With it, client sessions, their subscriptions, and undelivered QoS 1 and
                QoS 2 messages are stored in Redis. Redis is a separate service, not part of XMQ:
                install and start it yourself, then enable persistence on the <b>Persistence</b>
                page.
            </p>

            <p>
                If the server cannot reach Redis at startup it logs the error and keeps running
                with in-memory storage. The server stays up and nothing is persisted, so the log is
                worth a look after enabling it.
            </p>

            <p>
                One setting there is a genuine trade-off. <b>Max queued writes</b> at 0 makes every
                message wait for its own record to become durable before it is sent: the safest
                setting, and the slowest. Above 0, record writes pipeline instead, roughly doubling
                throughput, and the value is approximately how many messages could be lost if the
                server were killed outright.
            </p>
        </div>;
    }

    renderAdvancedSetup() {
        return <div>
            <h4 id="advanced-setup">Advanced setup</h4>

            <h5>MQTT bridges</h5>

            <p>
                A bridge is a client connection XMQ makes to another MQTT broker, carrying an
                agreed set of topics. The remote broker does not have to be XMQ: bridging to
                Mosquitto, EMQX, or anything else speaking MQTT works the same way. Bridges are
                configured on the <b>Bridges</b> page.
            </p>

            <p>
                A bridge has a direction, and so does each of its topics:
            </p>

            <table className="userManualTable">
                <thead>
                <tr>
                    <th>Mode</th>
                    <th>Meaning</th>
                </tr>
                </thead>
                <tbody>
                <tr><td><code>out</code></td>
                    <td>Messages published on this server are forwarded to the remote one</td></tr>
                <tr><td><code>in</code></td>
                    <td>This server subscribes to the remote one and republishes what it sends</td></tr>
                <tr><td><code>inout</code></td>
                    <td>Both directions</td></tr>
                </tbody>
            </table>

            <p>
                Each topic entry carries a pattern, a direction of its own, and the QoS to
                subscribe with. A topic's direction narrows the bridge's: a topic marked
                <code>out</code> on an <code>inout</code> bridge travels outbound only. Topics keep
                the name they were published with; there is no rewriting between the two servers.
                A bridge with no topics carries nothing.
            </p>

            <h5>Things worth getting right</h5>

            <ul>
                <li>
                    <b>Client ID must be stable.</b> The remote broker keys the bridge's stored
                    subscriptions and queued messages by it. An id that changes between reconnects
                    leaves orphaned sessions accumulating on the remote. Left empty, XMQ derives one
                    from the two node names, which is stable.
                </li>
                <li>
                    <b>Leave clean session off.</b> The remote then keeps the subscriptions and
                    queues messages while the link is down, so a brief outage does not lose them.
                    With it on, the session starts empty each time and anything published while
                    disconnected is gone.
                </li>
                <li>
                    <b>Loops.</b> XMQ marks its bridge connections with an origin node, so it will
                    not echo back a message that arrived over the bridge. Bridging through a broker
                    that does not do the same, in both directions, on overlapping topics, can still
                    produce a loop.
                </li>
                <li>
                    <b>Encrypted bridges</b> use key files already present on the XMQ host, given by
                    path. That is the opposite of the SSL Keys page, which uploads content. The
                    remote broker must be listening on its TLS port.
                </li>
            </ul>

            <h5>Server limits</h5>

            <p>
                The <b>Server Limits</b> page holds the thread counts and the per-session bounds.
                The rule for threads is short: XMQ pins its threads to physical cores, and no thread
                group should be larger than the host's physical core count. The groups share those
                cores, so count the send threads, the receive threads, the persistence threads and
                the per-listener threads together rather than each on its own. Adding threads past
                that buys contention, not throughput.
            </p>
        </div>;
    }

    renderTestingSuite() {
        return <div>
            <h4 id="testing-suite">MQTT testing suite</h4>

            <p>
                <code>xmq_scn</code> runs load-test scenarios described by JSON files. It is
                broker-neutral: it speaks plain MQTT and can be pointed at Mosquitto, EMQX, NanoMQ
                or anything else, which is what makes its numbers comparable across brokers.
            </p>

            <p>
                Scenarios ship in <code>/usr/local/share/xmq</code>. List them with:
            </p>

            <pre className="userManualCode">{`xmq_scn --list-scenarios`}</pre>

            <p>
                A scenario file names the client groups, their counts, the topics, and the pacing.
                The shipped set covers connection ramps, fan-in, fan-out, and point-to-point, at
                sizes from 10,000 to 1,000,000 connections. Writing a new one is a matter of copying
                the closest and changing the numbers.
            </p>

            <pre className="userManualCode">{`xmq_scn -s Point-To-Point-50K-50K-50K-50K.json --progress
xmq_scn -s 1M-Connections-5K-rate.json -h broker -p 1883`}</pre>

            <table className="userManualTable">
                <thead>
                <tr>
                    <th>Option</th>
                    <th>Meaning</th>
                </tr>
                </thead>
                <tbody>
                <tr><td><code>-s</code>, <code>--scenario</code></td>
                    <td>Scenario file. A relative path is looked up in the current directory first,
                        then in the installed scenario directory</td></tr>
                <tr><td><code>-m</code>, <code>--payload-size</code></td>
                    <td>Message size, overriding the scenario</td></tr>
                <tr><td><code>-r</code>, <code>--publish-rate</code></td>
                    <td>Messages per second per publisher. 0 means unpaced: as fast as possible</td></tr>
                <tr><td><code>-d</code>, <code>--duration</code></td>
                    <td>How long to run</td></tr>
                <tr><td><code>-q</code>, <code>--qos</code></td>
                    <td>Quality of service to publish with</td></tr>
                <tr><td><code>--max-inflight</code></td>
                    <td>Cap on a publisher's un-acknowledged QoS 1 backlog, so it throttles against
                        its own round-trip time rather than publishing blindly at the configured rate</td></tr>
                <tr><td><code>--id-prefix</code></td>
                    <td>Client id prefix, so several client hosts don't collide</td></tr>
                <tr><td><code>--progress</code></td>
                    <td>Show a progress bar</td></tr>
                </tbody>
            </table>

            <p>
                Reaching the larger sizes needs the tuning above on both hosts, and enough source
                addresses on the client: a connection is identified by its source address and port,
                so one address runs out of ports long before a million connections. The tested
                1,000,000-connection runs used 30 client addresses, about 33,000 connections each.
                The <code>make_ip_addresses.sh</code> helper in the same directory adds them, and
                <code>remove_ip_addresses.sh</code> takes them away again:
            </p>

            <pre className="userManualCode">{`./make_ip_addresses.sh -s <first three octets> -p <prefix> -f <first host octet> -c 29`}</pre>

            <p>
                Scenarios spread their clients across whatever addresses the host has, so the
                <code>--id-prefix</code> option keeps client ids distinct when several client
                machines drive the same broker.
            </p>

            <div className="userManualNote">
                Run the load generator on a different machine from the broker. Sharing a host means
                measuring the two competing for the same cores, which flatters neither.
            </div>
        </div>;
    }

    renderTestingSetup() {
        return <div>
            <h4 id="testing-setup">Testing your setup</h4>

            <p>
                Three small utilities are enough to prove an installation works. All of them take
                <code>-h</code> host, <code>-p</code> port, <code>-u</code> username,
                <code>--password</code>, and <code>--version</code>.
            </p>

            <h5>Is the server up?</h5>
            <pre className="userManualCode">{`systemctl status xmq_server
tail -f /var/log/xmq/xmq_server.log`}</pre>

            <p>
                The log names each listener it opened at startup, and says which storage it is
                using: a Redis address, or <code>memory</code> when persistence is off or Redis
                could not be reached.
            </p>

            <h5>Subscribe and publish</h5>

            <p>
                In one terminal, subscribe. In another, publish, and watch it arrive:
            </p>

            <pre className="userManualCode">{`xmq_sub -h localhost -p 1883 -u user --password secret -t "test/#" -q 1 -v

xmq_pub -h localhost -p 1883 -u user --password secret -t test/hello -m "it works"`}</pre>

            <p>
                Useful <code>xmq_pub</code> options: <code>-r</code> repeats the message a number of
                times, <code>-g</code> paces it to a rate, <code>-f</code> takes the payload from a
                file, <code>-s</code> reads it from standard input, and <code>-l</code> stamps a
                send timestamp into the payload so latency can be measured at the far end.
            </p>

            <h5>Testing TLS</h5>
            <pre className="userManualCode">{`xmq_sub -h localhost -p 8883 -u user --password secret \\
        -t "test/#" -q 1 -v --cafile /etc/xmq/certs/ca.crt`}</pre>

            <p>
                If this fails while the plain port works, the certificates are the place to look:
                the server log says whether it managed to load them at startup.
            </p>

            <h5>Many connections at once</h5>

            <p>
                <code>xmq_con</code> opens a number of sessions and holds them, which is the quickest
                way to see whether the host limits are in place:
            </p>

            <pre className="userManualCode">{`xmq_con -h localhost -p 1883 -u user --password secret -n 10000 --show-counters`}</pre>

            <p>
                If it stops short of the number asked for, the file descriptor limit is the usual
                cause, on whichever host ran out first. Check both, then see the tuning section.
            </p>

            <h5>Checking a bridge</h5>

            <p>
                Subscribe on the remote broker, publish on XMQ, and confirm the message crosses:
            </p>

            <pre className="userManualCode">{`xmq_sub -h remote-broker -p 1883 -t "bridged/#" -q 1 -v

xmq_pub -h localhost -p 1883 -u user --password secret -t bridged/test -m "over the bridge"`}</pre>

            <p>
                Nothing arriving usually means the bridge is disabled, its topic pattern does not
                cover the topic, or its direction excludes it. The server log, with
                <b>Connections</b> at <code>DEBUG</code>, shows the bridge connecting and how many
                topics it subscribed to.
            </p>
        </div>;
    }

    render() {
        return <div className="userManual">
            <h3 style={{textAlign: 'Left'}}>XMQ User Manual</h3>

            <p>
                XMQ is an MQTT 5, 3.1.1, and 3.1 server. This manual covers installing it, tuning
                the host it runs on, configuring it from the web interface, and proving that the
                result works.
            </p>

            {this.renderContents()}
            {this.renderSupportedOses()}
            {this.renderInstall()}
            {this.renderTuning()}
            {this.renderBasicSetup()}
            {this.renderAdvancedSetup()}
            {this.renderTestingSuite()}
            {this.renderTestingSetup()}
        </div>;
    }
}
