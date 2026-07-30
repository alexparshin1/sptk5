import React from "react";
import {Link} from "react-router-dom";
import "../css/Documentation.css";
import {BrokerBenchmark, parseBrokerResult} from "../components/BrokerBenchmark";
import fanOutUrl from "../xmq_test_results/Fan-Out-250K.txt";

export default class XMQ_tests_fanout extends React.Component
{
    state = {servers: null, error: null};

    componentDidMount()
    {
        fetch(fanOutUrl)
            .then((r) => r.text())
            .then((text) => this.setState({servers: parseBrokerResult(text)}))
            .catch((err) => this.setState({error: String(err)}));
    }

    render()
    {
        const {servers, error} = this.state;
        const mono = {fontFamily: "monospace", fontSize: "0.92em"};

        return <div key="tests-fanout" className="XMQ" style={{textAlign: "left", padding: 8}}>
            <h3>Fan-out</h3>

            <p>
                Fan-out is the few-to-many case, and the one that isolates delivery cost: a
                handful of publishers on a handful of topics, with a thousand subscribers on each.
                One received message becomes a thousand sends, so the inbound side is negligible
                and what is being measured is almost entirely the broker's delivery path.
            </p>

            <p>
                5 publishers on 5 topics at 50 messages/second each &mdash; 250 messages/second
                inbound &mdash; and 1,000 subscribers each subscribed to all five topics, giving
                250,000 messages/second delivered. QoS&nbsp;1, 16-byte payload, 5 minutes. Only
                1,005 connections are involved, so this is a throughput test rather than a
                connection-scale one. The scenario mirrors the Open MQTT Benchmark Suite's
                {" "}<span style={mono}>singlenode-fanout-5-1000-5-250K</span> case; see the
                {" "}<Link to="/xmq_tests_environment">Test Environment</Link> page for hardware,
                tuning and method.
            </p>

            {error && <p>Failed to load test results: {error}</p>}
            {!servers && !error && <p>Loading results&hellip;</p>}

            <BrokerBenchmark servers={servers} targetRate={250000}/>

            {servers && (
                <>
                    <h4>Reading the results</h4>
                    <ul>
                        <li>
                            <b>XMQ and EMQX both hold the full 250,000 messages/second</b>, which
                            makes this a clean latency and efficiency comparison rather than a
                            question of capacity. XMQ averages 2.48&nbsp;ms against EMQX's
                            4.04&nbsp;ms, using 2.6&times; less CPU and 19&times; less memory
                            &mdash; and doing it confined to 8 of the 16 vCPUs, while EMQX had all
                            of them.
                        </li>
                        <li>
                            <b>XMQ's 22&nbsp;MB peak is the figure most worth noting.</b> Fan-out
                            holds only 1,005 connections, so almost none of that memory is session
                            state; it reflects how little the delivery path buffers when it keeps
                            up with the send rate.
                        </li>
                        <li>
                            <b>Mosquitto reached 104,604/s of the 250,000 offered</b>, with latency
                            growing linearly from 8.5&nbsp;s to 165.9&nbsp;s &mdash; a fixed
                            deficit converting straight into backlog, so the average is bounded by
                            run length rather than settling. Its single event-loop thread is the
                            constraint, and unlike the other limits in this suite that one does not
                            move with better hardware: one thread is one thread on any machine. It
                            did beat its own published 81,000/s for this scenario, which is
                            consistent with faster cores, but the shape is unchanged.
                        </li>
                        <li>
                            <b>The load generator was never the limit.</b> At 250,000
                            messages/second the client has to receive and timestamp every message,
                            so it could plausibly have been the bottleneck rather than the broker.
                            It peaked at 442% of the 1600% available &mdash; about 28% of the
                            client machine &mdash; so both 250K results measure the broker.
                        </li>
                    </ul>
                </>
            )}
        </div>;
    }
}
