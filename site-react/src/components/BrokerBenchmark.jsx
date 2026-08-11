import React, {useState} from "react";

// Shared rendering for the per-broker benchmark result pages (fan-in, fan-out, and any
// later scenario). The results files under xmq_test_results/ all share one format:
// a preamble, then one block per broker with "Server:"/"Version:"/"CPU Load:"/"Max RAM:"
// meta followed by an interval table. Keeping the parser and chart here means a new
// scenario page is a few lines rather than another copy of this file.

export const SERVER_COLORS = {
    XMQ: "#1f77b4",
    Mosquitto: "#2ca02c",
    EMQX: "#d62728",
    NanoMQ: "#ff7f0e",
    FlashMQ: "#9467bd",
};

const KNOWN_BROKERS = /^(XMQ|EMQX|Mosquitto|NanoMQ|FlashMQ)$/;

// "Server:" also appears in the preamble describing the host, so only lines naming a
// known broker start a result block; anything else closes the current one.
export function parseBrokerResult(text)
{
    const servers = [];
    let current = null;

    for (const raw of text.split("\n")) {
        const line = raw.trim();
        if (line === "" || line.startsWith("Interval") || /^─+$/.test(line))
            continue;

        let m;
        if ((m = line.match(/^Server:\s*(.+)$/))) {
            const name = m[1].trim();
            if (KNOWN_BROKERS.test(name)) {
                current = {name, meta: {}, dataPoints: []};
                servers.push(current);
            } else {
                current = null;
            }
            continue;
        }

        if ((m = line.match(/^Average\s+(\d+)\s+(\d+)us\s+(\d+)$/)) && current) {
            current.averageCount = parseInt(m[1], 10);
            current.averageLatency = parseInt(m[2], 10);
            current.averageRate = parseInt(m[3], 10);
            continue;
        }

        if ((m = line.match(/^(\d+)ms\s+(\d+)\s+(\d+)us\s+(\d+)$/)) && current) {
            current.dataPoints.push({
                interval: parseInt(m[1], 10),
                count: parseInt(m[2], 10),
                latency: parseInt(m[3], 10),
                rate: parseInt(m[4], 10),
            });
            continue;
        }

        if ((m = line.match(/^([A-Za-z ]+?):\s+(.+)$/)) && current)
            current.meta[m[1].trim()] = m[2].trim();
    }
    return servers;
}

export function formatLatency(us)
{
    if (us >= 1_000_000)
        return `${(us / 1_000_000).toFixed(us >= 10_000_000 ? 0 : 1)}s`;
    if (us >= 1000)
        return `${(us / 1000).toFixed(1)}ms`;
    return `${us}us`;
}

const formatRate = (r) => (r >= 1000 ? `${(r / 1000).toFixed(1)}K/s` : `${r}/s`);
const formatInterval = (ms) => `${Math.round(ms / 1000)}s`;

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

    // Brokers here can differ by five orders of magnitude in the same scenario; a linear
    // axis would flatten the faster one onto the baseline and show nothing.
    const useLog = yMinRaw > 0 && yMaxRaw / yMinRaw > 50;
    const yMin = useLog ? Math.pow(10, Math.floor(Math.log10(yMinRaw))) : 0;
    const yMax = useLog ? Math.pow(10, Math.ceil(Math.log10(yMaxRaw))) : yMaxRaw;

    const xScale = (x) => margin.left + (xMax === 0 ? 0 : x / xMax) * innerW;
    const yScale = (y) => {
        const v = Math.max(y, yMin || 1);
        const t = useLog
            ? (Math.log10(v) - Math.log10(yMin)) / (Math.log10(yMax) - Math.log10(yMin) || 1)
            : (v - yMin) / (yMax - yMin || 1);
        return margin.top + innerH - t * innerH;
    };

    const intervals = [...new Set(allPoints.map((p) => p.interval))].sort((a, b) => a - b);
    const xTicks = intervals.filter((_, i) => i % 2 === 0);
    const yTicks = [];
    if (useLog) {
        for (let p = Math.log10(yMin); p <= Math.log10(yMax) + 0.001; p++)
            yTicks.push(Math.pow(10, p));
    } else {
        const step = (yMax - yMin) / 5;
        for (let i = 0; i <= 5; i++) yTicks.push(Math.round(yMin + step * i));
    }

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

    const tipW = 190;
    const tipH = 20 + hoverRows.length * 16;
    const tipX = hover === null ? 0 : Math.min(xScale(hover) + 10, width - margin.right - tipW);
    const tipY = margin.top + 6;

    return (
        <svg viewBox={`0 0 ${width} ${height}`}
             style={{width: "100%", maxWidth: width, height: "auto"}}
             onMouseMove={onMove} onMouseLeave={() => setHover(null)}>
            <title>Average latency per interval, by broker</title>

            {yTicks.map((t, i) => (
                <g key={`y${i}`}>
                    <line x1={margin.left} x2={width - margin.right} y1={yScale(t)} y2={yScale(t)} stroke="#eee"/>
                    <text x={margin.left - 8} y={yScale(t)} textAnchor="end" dominantBaseline="middle"
                          fontSize="11" fill="#555">{formatLatency(Math.round(t))}</text>
                </g>
            ))}
            {xTicks.map((t, i) => (
                <text key={`x${i}`} x={xScale(t)} y={height - margin.bottom + 16} textAnchor="middle"
                      fontSize="11" fill="#555">{formatInterval(t)}</text>
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
                    <rect x={tipX} y={tipY} width={tipW} height={tipH} rx="4" fill="#fff" stroke="#ccc" opacity="0.96"/>
                    <text x={tipX + 8} y={tipY + 14} fontSize="11" fill="#555">{formatInterval(hover)} elapsed</text>
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

export function BrokerBenchmark({servers, targetRate})
{
    if (!servers || servers.length === 0)
        return null;

    const cell = {border: "1px solid #ccc", padding: "4px 8px", verticalAlign: "top"};
    const head = {...cell, background: "#f0f0f0", fontWeight: "bold", textAlign: "left"};
    const table = {borderCollapse: "collapse", margin: "8px 0 16px 0"};

    return (
        <div>
            <table style={table}>
                <tbody>
                <tr>
                    <td style={head}>Server</td>
                    <td style={head}>Version</td>
                    <td style={head}>Messages</td>
                    <td style={head}>Achieved rate</td>
                    <td style={head}>Avg latency</td>
                    <td style={head}>CPU</td>
                    <td style={head}>Peak RAM</td>
                </tr>
                {servers.map((s) => {
                    const missed = targetRate && s.averageRate && s.averageRate < targetRate * 0.97;
                    return (
                        <tr key={s.name}>
                            <td style={cell}>
                                <span style={{
                                    display: "inline-block", width: 10, height: 10,
                                    backgroundColor: SERVER_COLORS[s.name] || "#333", marginRight: 6
                                }}/>
                                {s.name}
                            </td>
                            <td style={cell}>{s.meta["Version"] || "-"}</td>
                            <td style={cell}>{s.averageCount?.toLocaleString()}</td>
                            <td style={cell}>
                                {s.averageRate?.toLocaleString()}/s
                                {missed && <span style={{color: "#b00", marginLeft: 6}}>(below target)</span>}
                            </td>
                            <td style={cell}>{formatLatency(s.averageLatency)}</td>
                            <td style={cell}>{s.meta["CPU Load"] || "-"}</td>
                            <td style={cell}>{s.meta["Max RAM"] || "-"}</td>
                        </tr>
                    );
                })}
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
            <p style={{fontSize: "0.9em", color: "#666"}}>
                Latency uses a logarithmic axis: the brokers differ by several orders of magnitude
                in this scenario, and a linear axis would flatten the faster ones onto the baseline.
                Hover the chart for per-interval values.
            </p>
        </div>
    );
}
