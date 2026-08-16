import React, {useState} from "react";
import Seo from "../components/Seo";
import "../css/Documentation.css";
import p2pUrl from "../xmq_test_results/50K-Point-To-Point.txt";

const SERVER_COLORS = {
    XMQ: "#1f77b4",
    Mosquitto: "#2ca02c",
    EMQX: "#d62728",
    NanoMQ: "#ff7f0e",
};

// Per-run meta that duplicates the section heading rather than adding information.
const SKIPPED_META_KEYS = ["Scenario", "Rate"];

function parseP2PResult(text)
{
    const runs = [];
    let current = null;

    for (const raw of text.split("\n")) {
        const line = raw.trim();
        if (line === "" || line.startsWith("Interval") || /^─+$/.test(line))
            continue;

        const serverMatch = line.match(/^Server:\s*(.+)$/);
        if (serverMatch) {
            current = {name: serverMatch[1].trim(), meta: {}, dataPoints: []};
            runs.push(current);
            continue;
        }

        // "Average   89971693   215us   49984"
        const avgMatch = line.match(/^Average\s+(\d+)\s+(\d+)us\s+(\d+)$/);
        if (avgMatch && current) {
            current.averageCount = parseInt(avgMatch[1], 10);
            current.averageLatency = parseInt(avgMatch[2], 10);
            current.averageRate = parseInt(avgMatch[3], 10);
            continue;
        }

        // "0ms   8999988   213us   49999"
        const rowMatch = line.match(/^(\d+)ms\s+(\d+)\s+(\d+)us\s+(\d+)$/);
        if (rowMatch && current) {
            current.dataPoints.push({
                interval: parseInt(rowMatch[1], 10),
                count: parseInt(rowMatch[2], 10),
                latency: parseInt(rowMatch[3], 10),
                rate: parseInt(rowMatch[4], 10),
            });
            continue;
        }

        const kvMatch = line.match(/^([A-Za-z ]+?):\s+(.+)$/);
        if (kvMatch && current)
            current.meta[kvMatch[1].trim()] = kvMatch[2].trim();
    }

    // Group the runs by their publish rate, one group per chart.
    const groups = [];
    for (const run of runs) {
        const key = run.meta["Rate"] || "unknown";
        let group = groups.find((g) => g.key === key);
        if (!group) {
            group = {key, perPublisher: key, servers: []};
            groups.push(group);
        }
        group.servers.push(run);
    }

    // Label each group by the aggregate rate the scenario targets, taken from the
    // fastest server actually observed - a broker that falls behind under-reports it.
    for (const g of groups) {
        const best = Math.max(...g.servers.map((s) => s.averageRate || 0));
        g.targetRate = Math.round(best / 1000) * 1000;
    }
    return groups;
}

function formatLatency(us)
{
    if (us >= 1_000_000)
        return `${(us / 1_000_000).toFixed(us >= 10_000_000 ? 0 : 1)}s`;
    if (us >= 1000)
        return `${(us / 1000).toFixed(1)}ms`;
    return `${us}us`;
}

function formatRate(r)
{
    return r >= 1000 ? `${(r / 1000).toFixed(1)}K/s` : `${r}/s`;
}

function formatInterval(ms)
{
    return `${Math.round(ms / 1000)}s`;
}

function LatencyChart({servers, width = 760, height = 340})
{
    const [hover, setHover] = useState(null);

    const margin = {top: 16, right: 16, bottom: 44, left: 78};
    const innerW = width - margin.left - margin.right;
    const innerH = height - margin.top - margin.bottom;

    const allPoints = servers.flatMap((s) => s.dataPoints);
    if (allPoints.length === 0)
        return null;

    const xMax = Math.max(...allPoints.map((p) => p.interval));
    const yMinRaw = Math.min(...allPoints.map((p) => p.latency));
    const yMaxRaw = Math.max(...allPoints.map((p) => p.latency));

    // Latency here spans several orders of magnitude between brokers, so a linear
    // axis would flatten the faster one onto the baseline. Log unless the spread is small.
    const useLog = yMinRaw > 0 && yMaxRaw / yMinRaw > 50;
    const yMin = useLog ? Math.pow(10, Math.floor(Math.log10(yMinRaw))) : 0;
    const yMax = useLog ? Math.pow(10, Math.ceil(Math.log10(yMaxRaw))) : yMaxRaw;

    const xScale = (x) => margin.left + (xMax === 0 ? 0 : x / xMax) * innerW;
    const yScale = (y) => {
        const v = Math.max(y, yMin || 1);
        if (useLog) {
            const t = (Math.log10(v) - Math.log10(yMin)) / (Math.log10(yMax) - Math.log10(yMin) || 1);
            return margin.top + innerH - t * innerH;
        }
        const t = yMax === yMin ? 0 : (v - yMin) / (yMax - yMin);
        return margin.top + innerH - t * innerH;
    };

    const intervals = [...new Set(allPoints.map((p) => p.interval))].sort((a, b) => a - b);
    const xTicks = intervals.filter((_, i) => i % 2 === 0);
    const yTicks = useLog
        ? (() => {
            const ticks = [];
            for (let p = Math.log10(yMin); p <= Math.log10(yMax) + 0.001; p++)
                ticks.push(Math.pow(10, p));
            return ticks;
        })()
        : (() => {
            const step = (yMax - yMin) / 5;
            return Array.from({length: 6}, (_, i) => Math.round(yMin + step * i));
        })();

    const onMove = (event) => {
        const rect = event.currentTarget.getBoundingClientRect();
        const px = ((event.clientX - rect.left) / rect.width) * width;
        if (px < margin.left || px > width - margin.right) { setHover(null); return; }
        const value = ((px - margin.left) / innerW) * xMax;
        let nearest = intervals[0];
        for (const i of intervals)
            if (Math.abs(i - value) < Math.abs(nearest - value)) nearest = i;
        setHover(nearest);
    };

    const hoverRows = hover === null ? [] : servers
        .map((s) => ({name: s.name, point: s.dataPoints.find((p) => p.interval === hover)}))
        .filter((r) => r.point);

    const tipW = 168;
    const tipH = 20 + hoverRows.length * 16;
    const tipX = hover === null ? 0 : Math.min(xScale(hover) + 10, width - margin.right - tipW);
    const tipY = margin.top + 6;

    return (
        <svg viewBox={`0 0 ${width} ${height}`}
             style={{width: "100%", maxWidth: width, height: "auto"}}
             onMouseMove={onMove} onMouseLeave={() => setHover(null)}>
            <title>Average latency per interval, by broker</title>

            {yTicks.map((t, idx) => (
                <g key={`y${idx}`}>
                    <line x1={margin.left} x2={width - margin.right} y1={yScale(t)} y2={yScale(t)} stroke="#eee"/>
                    <text x={margin.left - 8} y={yScale(t)} textAnchor="end" dominantBaseline="middle"
                          fontSize="11" fill="#555">
                        {formatLatency(Math.round(t))}
                    </text>
                </g>
            ))}
            {xTicks.map((t, idx) => (
                <text key={`x${idx}`} x={xScale(t)} y={height - margin.bottom + 16} textAnchor="middle"
                      fontSize="11" fill="#555">
                    {formatInterval(t)}
                </text>
            ))}
            <text x={margin.left + innerW / 2} y={height - 6} textAnchor="middle" fontSize="11" fill="#777">
                elapsed
            </text>

            <line x1={margin.left} x2={width - margin.right} y1={height - margin.bottom}
                  y2={height - margin.bottom} stroke="#999"/>
            <line x1={margin.left} x2={margin.left} y1={margin.top} y2={height - margin.bottom} stroke="#999"/>

            {hover !== null && (
                <line x1={xScale(hover)} x2={xScale(hover)} y1={margin.top} y2={height - margin.bottom}
                      stroke="#bbb" strokeDasharray="3 3"/>
            )}

            {servers.map((s) => (
                <polyline key={s.name} fill="none" stroke={SERVER_COLORS[s.name] || "#333"} strokeWidth="2"
                          points={s.dataPoints.map((p) => `${xScale(p.interval)},${yScale(p.latency)}`).join(" ")}/>
            ))}

            {hover !== null && hoverRows.map((r) => (
                <circle key={r.name} cx={xScale(hover)} cy={yScale(r.point.latency)} r="4"
                        fill={SERVER_COLORS[r.name] || "#333"} stroke="#fff" strokeWidth="2"/>
            ))}

            {hover !== null && hoverRows.length > 0 && (
                <g>
                    <rect x={tipX} y={tipY} width={tipW} height={tipH} rx="4"
                          fill="#fff" stroke="#ccc" opacity="0.96"/>
                    <text x={tipX + 8} y={tipY + 14} fontSize="11" fill="#555">
                        {formatInterval(hover)} elapsed
                    </text>
                    {hoverRows.map((r, i) => (
                        <g key={r.name}>
                            <rect x={tipX + 8} y={tipY + 22 + i * 16} width="8" height="8"
                                  fill={SERVER_COLORS[r.name] || "#333"}/>
                            <text x={tipX + 22} y={tipY + 30 + i * 16} fontSize="11" fill="#333">
                                {r.name} {formatLatency(r.point.latency)} &middot; {formatRate(r.point.rate)}
                            </text>
                        </g>
                    ))}
                </g>
            )}
        </svg>
    );
}

function Legend({servers})
{
    return (
        <div style={{display: "flex", gap: 16, margin: "12px 0 8px"}}>
            {servers.map((s) => (
                <div key={s.name} style={{display: "flex", alignItems: "center", gap: 4}}>
                    <span style={{
                        width: 10, height: 10, display: "inline-block",
                        backgroundColor: SERVER_COLORS[s.name] || "#333"
                    }}/>
                    <span>{s.name}</span>
                </div>
            ))}
        </div>
    );
}

function RateGroup({group})
{
    const {servers, perPublisher, targetRate} = group;
    const extraColumns = ["Version", "QOS", "CPU Load"].filter((key) =>
        servers.some((s) => s.meta[key] !== undefined && !SKIPPED_META_KEYS.includes(key))
    );

    return (
        <div style={{marginBottom: 40}}>
            <h4>{formatRate(targetRate).replace("/s", "")} messages/second &mdash; {perPublisher}</h4>

            <table cellPadding="4" cellSpacing="4">
                <thead>
                <tr>
                    <th>Server</th>
                    {extraColumns.map((c) => <th key={c}>{c}</th>)}
                    <th>Messages</th>
                    <th>Achieved rate</th>
                    <th>Avg latency</th>
                </tr>
                </thead>
                <tbody>
                {servers.map((s) => (
                    <tr key={s.name}>
                        <td>
                            <span style={{
                                display: "inline-block", width: 10, height: 10,
                                backgroundColor: SERVER_COLORS[s.name] || "#333", marginRight: 6
                            }}/>
                            {s.name}
                        </td>
                        {extraColumns.map((c) => <td key={c}>{s.meta[c] || "-"}</td>)}
                        <td>{s.averageCount?.toLocaleString()}</td>
                        <td>{s.averageRate?.toLocaleString()}/s</td>
                        <td>{formatLatency(s.averageLatency)}</td>
                    </tr>
                ))}
                </tbody>
            </table>

            <Legend servers={servers}/>
            <LatencyChart servers={servers}/>
            <p style={{fontSize: "0.9em", color: "#666"}}>
                Latency uses a logarithmic axis: the brokers differ by several orders of
                magnitude, and a linear axis would flatten the faster one onto the baseline.
                Hover the chart for per-interval values.
            </p>
        </div>
    );
}

export default class XMQ_tests_p2p extends React.Component
{
    state = {groups: null, error: null};

    componentDidMount()
    {
        fetch(p2pUrl)
            .then((r) => r.text())
            .then((text) => this.setState({groups: parseP2PResult(text)}))
            .catch((err) => this.setState({error: String(err)}));
    }

    render()
    {
        const {groups, error} = this.state;

        return <div key="tests-p2p" className="XMQ" style={{textAlign: "left", padding: 8}}>
            <Seo title="MQTT Performance Tests — Point-to-Point"
                 description="Point-to-point MQTT performance tests with 50,000 client pairs: end-to-end latency and throughput of the XMQ MQTT server compared with other MQTT servers."
                 keywords="MQTT performance tests, fast MQTT server, MQTT benchmark, XMQ"
                 path="/xmq_tests_p2p"/>
            <h1>MQTT Performance Tests: Point-to-Point</h1>
            <h3>Point-to-point</h3>

            <p>
                Point-to-point pairs every publisher with its own subscriber on its own topic,
                so each message has exactly one recipient and the broker does no fan-out. It
                isolates the per-message cost of routing: latency is measured end to end,
                publisher to subscriber, and reported as an average per interval so that
                latency drifting upward over a run is visible rather than averaged away.
            </p>

            <p>
                50,000 publisher/subscriber pairs, QoS&nbsp;1, run at two publish rates: one and
                two messages per second per publisher, giving 50K and 100K messages/second in
                aggregate. Every broker sees an identical client and scenario; only the broker
                changes between runs.
            </p>

            <p>
                <b>Mosquitto is absent from these results because it could not complete the
                test.</b> It fails to sustain rates above roughly 40K messages/second: its
                single event-loop thread saturates one core, and past that point the delivery
                backlog grows without bound rather than settling. Since it cannot hold the 50K
                rate for the duration of a run, there is no comparable figure to publish here.
                See the fan-in and fan-out pages for rates it does sustain.
            </p>

            <p style={{fontSize: "0.9em", color: "#666"}}>
                This is not specific to our setup: EMQX's own
                {" "}<a href="https://github.com/emqx/mqttbs/tree/main/results"
                        target="_blank" rel="noreferrer">published results</a> for the same
                scenario record Mosquitto missing the target as well, settling at 37.3K
                messages/second against the 50K offered.
            </p>

            {error && <p>Failed to load test results: {error}</p>}
            {!groups && !error && <p>Loading results&hellip;</p>}

            {groups && groups.map((g) => <RateGroup key={g.key} group={g}/>)}

            {groups && (
                <>
                    <h4>Reading the results</h4>
                    <ul>
                        <li>
                            <b>XMQ holds sub-millisecond latency at both rates.</b> Doubling the
                            load from 50K to 100K messages/second moves the average from 215us to
                            278us &mdash; a 29% rise for twice the work &mdash; while CPU goes from
                            364% to 636%, slightly better than linear.
                        </li>
                        <li>
                            <b>EMQX completes the 50K run but two orders of magnitude slower</b>,
                            at 53.0ms against XMQ's 215us, and needs 1071% CPU against 364% to do it.
                        </li>
                        <li>
                            <b>At 100K messages/second EMQX does not keep up.</b> It achieves
                            84.6K/s against the 100K/s offered, and its latency climbs steadily
                            through the whole run &mdash; 17.6s at the first interval to 263s at the
                            last, rising by roughly 25 seconds every interval. A latency that grows
                            linearly with elapsed time is a queue filling faster than it drains, so
                            the figure is bounded only by run length and would keep climbing. Its
                            149s average is a property of when the run stopped, not a steady state.
                        </li>
                    </ul>
                </>
            )}
        </div>;
    }
}
