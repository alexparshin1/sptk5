import React from "react";
import "../css/Documentation.css";
import test100kUrl from "../xmq_test_results/100K-connections.txt";
import test500kUrl from "../xmq_test_results/500K-connections.txt";

const SERVER_COLORS = {
    XMQ: "#1f77b4",
    Mosquitto: "#2ca02c",
    EMQX: "#d62728",
    NanoMQ: "#ff7f0e",
};

// Meta keys that are per-server but not interesting as their own table column
// because they duplicate the test title (e.g. "Scenario: 100K Connection Test").
const SKIPPED_META_KEYS = ["Scenario"];

function parseConnectionsResult(text)
{
    const conditions = {};
    let note = null;
    const servers = [];
    let current = null;

    for (const raw of text.split("\n")) {
        const line = raw.trim();
        if (line === "" || line.startsWith("Conditions") || line.startsWith("Interval") || /^─+$/.test(line))
            continue;

        const noteMatch = line.match(/^Note:\s*(.+)$/);
        if (noteMatch) { note = noteMatch[1]; continue; }

        const serverMatch = line.match(/^Server:\s*(.+)$/);
        if (serverMatch) {
            current = {name: serverMatch[1].trim(), meta: {}, dataPoints: []};
            servers.push(current);
            continue;
        }

        const avgMatch = line.match(/^Average\s+(\d+)\s+(\d+)us$/);
        if (avgMatch && current) {
            current.averageCount = parseInt(avgMatch[1], 10);
            current.averageLatency = parseInt(avgMatch[2], 10);
            continue;
        }

        const rowMatch = line.match(/^(\d+)ms\s+(\d+)\s+(\d+)us$/);
        if (rowMatch && current) {
            current.dataPoints.push({
                interval: parseInt(rowMatch[1], 10),
                count: parseInt(rowMatch[2], 10),
                latency: parseInt(rowMatch[3], 10),
            });
            continue;
        }

        const kvMatch = line.match(/^([A-Za-z ]+?):\s+(.+)$/);
        if (kvMatch) {
            const key = kvMatch[1].trim();
            const value = kvMatch[2].trim();
            if (current)
                current.meta[key] = value;
            else
                conditions[key] = value;
        }
    }

    return {conditions, note, servers};
}

function formatLatency(us)
{
    if (us >= 1_000_000)
        return `${(us / 1_000_000).toFixed(2)}s`;
    if (us >= 1000)
        return `${(us / 1000).toFixed(1)}ms`;
    return `${us}us`;
}

function formatInterval(ms)
{
    return `${(ms / 1000).toFixed(0)}s`;
}

function niceTicks(min, max, count)
{
    if (min === max)
        return [min];
    const step = (max - min) / (count - 1);
    return Array.from({length: count}, (_, i) => Math.round(min + step * i));
}

function LatencyChart({servers, width = 760, height = 340})
{
    const margin = {top: 16, right: 16, bottom: 40, left: 70};
    const innerW = width - margin.left - margin.right;
    const innerH = height - margin.top - margin.bottom;

    const chartServers = servers.map((s) => {
        const points = s.dataPoints;
        const last = points[points.length - 1];
        const dataPoints = last && last.count < 10 ? points.slice(0, -1) : points;
        return {...s, dataPoints};
    });

    const allPoints = chartServers.flatMap((s) => s.dataPoints);
    if (allPoints.length === 0)
        return null;

    const xMax = Math.max(...allPoints.map((p) => p.interval));
    const yMinRaw = Math.min(...allPoints.map((p) => p.latency));
    const yMaxRaw = Math.max(...allPoints.map((p) => p.latency));
    const useLog = yMinRaw > 0 && yMaxRaw / yMinRaw > 50;
    const yMin = useLog ? yMinRaw : 0;
    const yMax = yMaxRaw;

    const xScale = (x) => margin.left + (xMax === 0 ? 0 : x / xMax) * innerW;
    const yScale = (y) => {
        if (useLog) {
            const t = (Math.log10(y) - Math.log10(yMin)) / (Math.log10(yMax) - Math.log10(yMin) || 1);
            return margin.top + innerH - t * innerH;
        }
        const t = yMax === yMin ? 0 : (y - yMin) / (yMax - yMin);
        return margin.top + innerH - t * innerH;
    };

    const xTicks = niceTicks(0, xMax, 6);
    const yTicks = useLog
        ? (() => {
            const lo = Math.floor(Math.log10(yMin));
            const hi = Math.ceil(Math.log10(yMax));
            const ticks = [];
            for (let p = lo; p <= hi; p++)
                ticks.push(Math.pow(10, p));
            return ticks;
        })()
        : niceTicks(yMin, yMax, 6);

    return (
        <svg viewBox={`0 0 ${width} ${height}`} style={{width: "100%", maxWidth: width, height: "auto"}}>
            {yTicks.map((t, idx) => (
                <g key={`y${idx}`}>
                    <line x1={margin.left} x2={width - margin.right} y1={yScale(t)} y2={yScale(t)} stroke="#eee"/>
                    <text x={margin.left - 8} y={yScale(t)} textAnchor="end" dominantBaseline="middle" fontSize="11">
                        {formatLatency(Math.round(t))}
                    </text>
                </g>
            ))}
            {xTicks.map((t, idx) => (
                <g key={`x${idx}`}>
                    <text x={xScale(t)} y={height - margin.bottom + 16} textAnchor="middle" fontSize="11">
                        {formatInterval(t)}
                    </text>
                </g>
            ))}
            <line x1={margin.left} x2={width - margin.right} y1={height - margin.bottom} y2={height - margin.bottom}
                  stroke="#999"/>
            <line x1={margin.left} x2={margin.left} y1={margin.top} y2={height - margin.bottom} stroke="#999"/>

            {chartServers.map((s) => (
                <polyline
                    key={s.name}
                    fill="none"
                    stroke={SERVER_COLORS[s.name] || "#333"}
                    strokeWidth="2"
                    points={s.dataPoints
                        .map((p) => `${xScale(p.interval)},${yScale(Math.max(p.latency, yMin || 1))}`)
                        .join(" ")}
                />
            ))}
        </svg>
    );
}

function ConnectionsTest({title, test})
{
    if (!test)
        return null;

    const {conditions, note, servers} = test;
    const extraColumns = ["Port", "Max CPU", "Max RAM"].filter((key) =>
        servers.some((s) => s.meta[key] !== undefined && !SKIPPED_META_KEYS.includes(key))
    );

    return (
        <div style={{marginBottom: 40}}>
            <h3>{title}</h3>
            <ul>
                {Object.entries(conditions).map(([key, value]) => (
                    <li key={key}><b>{key}:</b> {value}</li>
                ))}
            </ul>
            {note && <p><i>{note}</i></p>}

            <table cellPadding="4" cellSpacing="4">
                <thead>
                <tr>
                    <th>Server</th>
                    {extraColumns.map((c) => <th key={c}>{c}</th>)}
                    <th>Avg Connections</th>
                    <th>Avg Latency</th>
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
                        <td>{formatLatency(s.averageLatency)}</td>
                    </tr>
                ))}
                </tbody>
            </table>

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
            <LatencyChart servers={servers}/>
        </div>
    );
}

export default class XMQ_tests_connections extends React.Component
{
    state = {test100k: null, test500k: null, error: null};

    componentDidMount()
    {
        Promise.all([
            fetch(test100kUrl).then((r) => r.text()),
            fetch(test500kUrl).then((r) => r.text()),
        ])
            .then(([text100k, text500k]) => {
                this.setState({
                    test100k: parseConnectionsResult(text100k),
                    test500k: parseConnectionsResult(text500k),
                });
            })
            .catch((err) => this.setState({error: String(err)}));
    }

    render()
    {
        const {test100k, test500k, error} = this.state;
        return <div key="tests-connections" className="XMQ" style={{textAlign: "left", padding: 8}}>
            <h3>Connections</h3>
            <p>
                Connection tests measure how many client connections a broker can accept and how
                connect latency behaves as the number of concurrently connected clients grows.
                Each scenario connects clients in batches and records, per interval, how many
                connections completed and their average latency.
            </p>
            {error && <p>Failed to load test results: {error}</p>}
            <ConnectionsTest title="100K Connections" test={test100k}/>
            <ConnectionsTest title="500K Connections" test={test500k}/>
        </div>;
    }
}
