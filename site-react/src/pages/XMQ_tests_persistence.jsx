import React, {useState} from "react";
import "../css/Documentation.css";
import persistenceUrl from "../xmq_test_results/Persistence.txt";

// One colour per scenario size, warm as the load grows.
const RUN_COLORS = ["#1f77b4", "#2ca02c", "#ff7f0e", "#d62728"];

function parsePersistenceResult(text)
{
    const runs = [];
    let current = null;

    for (const raw of text.split("\n")) {
        const line = raw.trim();
        if (line === "" || line.startsWith("Interval") || /^─+$/.test(line))
            continue;

        if (/^Server:/.test(line)) {
            current = {meta: {}, cpu: {}, dataPoints: []};
            runs.push(current);
            continue;
        }
        if (!current)
            continue;

        // "Average   5995002   249us   19983"
        const avgMatch = line.match(/^Average\s+(\d+)\s+(\d+)us\s+(\d+)$/);
        if (avgMatch) {
            current.averageCount = parseInt(avgMatch[1], 10);
            current.averageLatency = parseInt(avgMatch[2], 10);
            current.averageRate = parseInt(avgMatch[3], 10);
            continue;
        }

        // "30000ms   600000   244us   20000"
        const rowMatch = line.match(/^(\d+)ms\s+(\d+)\s+(\d+)us\s+(\d+)$/);
        if (rowMatch) {
            current.dataPoints.push({
                interval: parseInt(rowMatch[1], 10),
                count: parseInt(rowMatch[2], 10),
                latency: parseInt(rowMatch[3], 10),
                rate: parseInt(rowMatch[4], 10),
            });
            continue;
        }

        // Two processes are measured, so the CPU key appears twice per run and is told
        // apart by what follows it: "CPU Load: 415% (XMQ)" and "CPU Load: 87% (Redis)".
        const cpuMatch = line.match(/^CPU Load:\s+(\d+%)\s*\((\w+)\)$/);
        if (cpuMatch) {
            current.cpu[cpuMatch[2]] = cpuMatch[1];
            continue;
        }

        const kvMatch = line.match(/^([A-Za-z ]+?):\s+(.+)$/);
        if (kvMatch)
            current.meta[kvMatch[1].trim()] = kvMatch[2].trim();
    }

    for (const run of runs) {
        // "Point-To-Point 20K (persistent sessions)" -> 20K, used as the chart label.
        const sizeMatch = (run.meta["Scenario"] || "").match(/(\d+K)/);
        run.label = sizeMatch ? sizeMatch[1] : `${Math.round((run.averageRate || 0) / 1000)}K`;

        // A run whose latency climbs from start to finish has a backlog growing behind it:
        // the average then says when the run stopped, not what the broker sustains.
        const points = run.dataPoints;
        if (points.length >= 4) {
            const firstThird = points.slice(0, Math.ceil(points.length / 3));
            const lastThird = points.slice(-Math.ceil(points.length / 3));
            const mean = (list) => list.reduce((sum, p) => sum + p.latency, 0) / list.length;
            run.growth = mean(lastThird) / mean(firstThird);
            run.saturated = run.growth > 1.5;
        }
    }

    return runs;
}

function formatLatency(us)
{
    if (us >= 1000)
        return `${(us / 1000).toFixed(2)}ms`;
    return `${us}us`;
}

function formatInterval(ms)
{
    return `${Math.round(ms / 1000)}s`;
}

function LatencyChart({runs, width = 760, height = 360})
{
    const [hover, setHover] = useState(null);

    const margin = {top: 16, right: 16, bottom: 44, left: 78};
    const innerW = width - margin.left - margin.right;
    const innerH = height - margin.top - margin.bottom;

    const allPoints = runs.flatMap((r) => r.dataPoints);
    if (allPoints.length === 0)
        return null;

    const xMax = Math.max(...allPoints.map((p) => p.interval));
    const yMinRaw = Math.min(...allPoints.map((p) => p.latency));
    const yMaxRaw = Math.max(...allPoints.map((p) => p.latency));

    // The saturated run is an order of magnitude above the others; a linear axis would
    // press the three stable ones flat against the baseline.
    const useLog = yMinRaw > 0 && yMaxRaw / yMinRaw > 8;
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
    const yTicks = useLog
        ? (() => {
            const ticks = [];
            for (let p = Math.log10(yMin); p <= Math.log10(yMax) + 0.001; p++)
                ticks.push(Math.pow(10, p));
            return ticks;
        })()
        : Array.from({length: 6}, (_, i) => Math.round(yMin + ((yMax - yMin) / 5) * i));

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

    const hoverRows = hover === null ? [] : runs
        .map((r) => ({label: r.label, color: r.color, point: r.dataPoints.find((p) => p.interval === hover)}))
        .filter((r) => r.point);

    const tipW = 168;
    const tipH = 20 + hoverRows.length * 16;
    const tipX = hover === null ? 0 : Math.min(xScale(hover) + 10, width - margin.right - tipW);
    const tipY = margin.top + 6;

    return (
        <svg viewBox={`0 0 ${width} ${height}`}
             style={{width: "100%", maxWidth: width, height: "auto"}}
             onMouseMove={onMove} onMouseLeave={() => setHover(null)}>
            <title>Average latency per interval, by scenario size</title>

            {yTicks.map((t, idx) => (
                <g key={`y${idx}`}>
                    <line x1={margin.left} x2={width - margin.right} y1={yScale(t)} y2={yScale(t)} stroke="#eee"/>
                    <text x={margin.left - 8} y={yScale(t)} textAnchor="end" dominantBaseline="middle"
                          fontSize="11" fill="#555">
                        {formatLatency(Math.round(t))}
                    </text>
                </g>
            ))}
            {intervals.map((t, idx) => (
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

            {runs.map((r) => (
                <polyline key={r.label} fill="none" stroke={r.color} strokeWidth="2"
                          strokeDasharray={r.saturated ? "6 3" : undefined}
                          points={r.dataPoints.map((p) => `${xScale(p.interval)},${yScale(p.latency)}`).join(" ")}/>
            ))}

            {hover !== null && hoverRows.map((r) => (
                <circle key={r.label} cx={xScale(hover)} cy={yScale(r.point.latency)} r="4"
                        fill={r.color} stroke="#fff" strokeWidth="2"/>
            ))}

            {hover !== null && hoverRows.length > 0 && (
                <g>
                    <rect x={tipX} y={tipY} width={tipW} height={tipH} rx="4"
                          fill="#fff" stroke="#ccc" opacity="0.96"/>
                    <text x={tipX + 8} y={tipY + 14} fontSize="11" fill="#555">
                        {formatInterval(hover)} elapsed
                    </text>
                    {hoverRows.map((r, i) => (
                        <g key={r.label}>
                            <rect x={tipX + 8} y={tipY + 22 + i * 16} width="8" height="8" fill={r.color}/>
                            <text x={tipX + 22} y={tipY + 30 + i * 16} fontSize="11" fill="#333">
                                {r.label} &middot; {formatLatency(r.point.latency)}
                            </text>
                        </g>
                    ))}
                </g>
            )}
        </svg>
    );
}

function Legend({runs})
{
    return (
        <div style={{display: "flex", gap: 16, margin: "12px 0 8px", flexWrap: "wrap"}}>
            {runs.map((r) => (
                <div key={r.label} style={{display: "flex", alignItems: "center", gap: 4}}>
                    <span style={{width: 10, height: 10, display: "inline-block", backgroundColor: r.color}}/>
                    <span>{r.label} {r.saturated && <i>(beyond capacity)</i>}</span>
                </div>
            ))}
        </div>
    );
}

export default class XMQ_tests_persistence extends React.Component
{
    state = {runs: null, error: null};

    componentDidMount()
    {
        fetch(persistenceUrl)
            .then((response) => response.text())
            .then((text) => {
                const runs = parsePersistenceResult(text);
                runs.forEach((run, index) => { run.color = RUN_COLORS[index % RUN_COLORS.length]; });
                this.setState({runs});
            })
            .catch((error) => this.setState({error: String(error)}));
    }

    render()
    {
        const {runs, error} = this.state;
        const stable = runs ? runs.filter((r) => !r.saturated) : [];
        const best = stable.length > 0 ? stable[stable.length - 1] : null;

        return <div key="tests-persistence" className="XMQ" style={{textAlign: "left", padding: 8}}>
            <h3>Persistence</h3>

            <p>
                Everything on the other test pages runs in memory. This page measures what it
                costs to make delivery survive a broker restart: every session is persistent,
                every message is QoS&nbsp;1, and the record of each delivery is written to Redis
                before the message is handed on. Nothing is lost if the broker stops, and the
                question is what that guarantee is worth in throughput and latency.
            </p>

            <p>
                The scenario is point-to-point, the same shape as the in-memory point-to-point
                page: each publisher has its own subscriber on its own topic, so the broker does
                no fan-out and what is measured is the per-message cost of routing plus the cost
                of persisting it. Both client groups connect with <code>clean_session</code> off,
                so a run at 20K holds 40,000 persistent sessions. Each publisher sends one
                message per second with a 16-byte payload. Redis runs on the broker host, and the
                broker starts against an empty database.
            </p>

            <p>
                <b>This configuration answers a particular need, and is not the one most
                deployments want.</b> Persistence earns its cost where a message that never
                arrives is worse than a message that arrives late: commands to equipment that is
                intermittently connected, orders and transactions, anything a subscriber must
                receive even if it was offline when the message was sent, or if the broker
                restarted between the two. For live telemetry, metrics and status streams &mdash;
                where the next reading supersedes the last and a gap costs nothing &mdash;
                persistence buys nothing, and the in-memory figures on the other test pages are
                the ones that apply.
            </p>

            <p>
                It is worth setting the numbers below against that. Workloads that genuinely need
                durable delivery are usually counted in messages per device per minute rather
                than per second, so the rates measured here are well clear of what they ask for.
                The ceiling matters when durable and high-volume traffic share one broker, and
                the two can be separated: only sessions connecting with{" "}
                <code>clean_session</code> off and messages sent at QoS&nbsp;1 or above pay for
                persistence at all, so a broker can carry both without the fast traffic paying
                the durable traffic's cost.
            </p>

            {error && <p>Failed to load test results: {error}</p>}
            {!runs && !error && <p>Loading results&hellip;</p>}

            {runs && (
                <>
                    <table cellPadding="4" cellSpacing="4">
                        <thead>
                        <tr>
                            <th>Scenario</th>
                            <th>Sessions</th>
                            <th>Messages</th>
                            <th>Achieved rate</th>
                            <th>Avg latency</th>
                            <th>CPU (XMQ)</th>
                            <th>CPU (Redis)</th>
                            <th>Max RAM</th>
                        </tr>
                        </thead>
                        <tbody>
                        {runs.map((r) => (
                            <tr key={r.label}>
                                <td>
                                    <span style={{
                                        display: "inline-block", width: 10, height: 10,
                                        backgroundColor: r.color, marginRight: 6
                                    }}/>
                                    {r.label}
                                </td>
                                <td>{(parseInt(r.label, 10) * 2).toLocaleString()}K</td>
                                <td>{r.averageCount?.toLocaleString()}</td>
                                <td>{r.averageRate?.toLocaleString()}/s</td>
                                <td>
                                    {formatLatency(r.averageLatency)}
                                    {r.saturated && <span style={{color: "#d62728"}}> &nbsp;not a steady state</span>}
                                </td>
                                <td>{r.cpu["XMQ"] || "-"}</td>
                                <td>{r.cpu["Redis"] || "-"}</td>
                                <td>{r.meta["Max RAM"] || "-"}</td>
                            </tr>
                        ))}
                        </tbody>
                    </table>

                    <Legend runs={runs}/>
                    <LatencyChart runs={runs}/>
                    <p style={{fontSize: "0.9em", color: "#666"}}>
                        Latency uses a logarithmic axis. A flat line is a broker keeping up; a
                        line that climbs for the whole run is a backlog growing behind it, and is
                        drawn dashed. Hover the chart for per-interval values.
                    </p>

                    <h4>Reading the results</h4>
                    <ul>
                        {best && (
                            <li>
                                <b>Fully durable delivery holds up to {best.label} messages/second</b>
                                {" "}at {formatLatency(best.averageLatency)} average latency, with every
                                message recorded in Redis before it is sent. Latency is flat across the
                                run at 10K, 20K and 30K, which is what distinguishes a rate the broker
                                sustains from one it merely survives.
                            </li>
                        )}
                        {runs.filter((r) => r.saturated).map((r) => (
                            <li key={r.label}>
                                <b>At {r.label} the broker is past its limit.</b> The offered rate is
                                still met &mdash; the load generator paces itself &mdash; but latency
                                climbs from {formatLatency(r.dataPoints[0].latency)} at the first
                                interval to {formatLatency(r.dataPoints[r.dataPoints.length - 1].latency)}
                                {" "}at the last, without ever flattening. That is a queue filling faster
                                than it drains: the {formatLatency(r.averageLatency)} average describes
                                when the run stopped rather than any level the broker settles at, and a
                                longer run would report a worse figure. It is listed for completeness,
                                not as a result.
                            </li>
                        ))}
                        {runs.some((r) => r.saturated) && (
                            <li>
                                <b>Past the limit it degrades rather than collapses.</b> The offered
                                rate is still met, and the backlog costs single-digit milliseconds
                                across a five-minute run &mdash; not the seconds, or tens of seconds,
                                that an overloaded broker usually starts reporting. Being beyond the
                                sustainable rate is not the same as falling over, and a short burst
                                above it is absorbed rather than punished.
                            </li>
                        )}
                        <li>
                            <b>Memory is not the constraint.</b> Peak RSS grows with session count and
                            stays small in absolute terms &mdash; 0.13&nbsp;Gb at 10K rising to
                            0.45&nbsp;Gb at 40K &mdash; so the limit is reached in CPU and round trips
                            long before memory matters.
                        </li>
                        <li>
                            <b>Redis is close behind, but it is not what stops the broker first.</b>
                            {" "}Redis CPU plateaus at 85&ndash;94% of a single core from 20K onward
                            rather than rising with the load, so its cost per message is falling as the
                            rate climbs. It is single-threaded, so that plateau is near its ceiling and
                            it becomes the next limit as soon as the one below is lifted.
                        </li>
                    </ul>

                    <h4>Where the cost goes, and how to trade it</h4>

                    <p>
                        By default XMQ is fully durable: a QoS&nbsp;1 message for a persistent session
                        waits for its Redis record before it is delivered. Because the sending thread
                        is parked for that round trip, the writes never overlap &mdash; one message,
                        one round trip &mdash; and that, rather than the amount of work Redis does, is
                        what sets the ceiling above.
                    </p>

                    <p>
                        The <code>max_queued_writes</code> setting is the dial. Left at 0 it is the
                        fully durable behaviour measured here. Set above 0, deliveries proceed while
                        their records are written, so the writes pipeline and the ceiling moves up
                        considerably; the number is the count of messages that may be in flight
                        without a durable record, and so exactly how many could be lost if the broker
                        were killed at the wrong instant. It is a deliberate, bounded trade rather
                        than a hidden one, which is why the durable configuration is what gets
                        published here.
                    </p>

                    <p style={{fontSize: "0.9em", color: "#666"}}>
                        For comparison, the same hardware and the same point-to-point shape without
                        persistence sustains over 100,000 messages/second at 278us
                        (see <a href="/xmq_tests_p2p">Point-to-point</a>). Durability is therefore
                        worth roughly a threefold reduction in throughput &mdash; and that is a lower
                        bound, since the in-memory run had not reached its own limit.
                    </p>
                </>
            )}
        </div>;
    }
}
