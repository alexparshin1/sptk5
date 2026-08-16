import React from "react";
import Seo from "../components/Seo";
import {Link} from "react-router-dom";
import "../css/Documentation.css";
import {BrokerBenchmark, parseBrokerResult} from "../components/BrokerBenchmark";
import fanInUrl from "../xmq_test_results/Fan-In-50K.txt";

export default class XMQ_tests_fanin extends React.Component
{
    state = {servers: null, error: null};

    componentDidMount()
    {
        fetch(fanInUrl)
            .then((r) => r.text())
            .then((text) => this.setState({servers: parseBrokerResult(text)}))
            .catch((err) => this.setState({error: String(err)}));
    }

    render()
    {
        const {servers, error} = this.state;
        const mono = {fontFamily: "monospace", fontSize: "0.92em"};

        return <div key="tests-fanin" className="XMQ" style={{textAlign: "left", padding: 8}}>
            <Seo title="MQTT Performance Tests — Fan-In"
                 description="Fan-in MQTT performance tests with 50,000 publishers: throughput and latency of the XMQ MQTT server compared with other MQTT servers."
                 keywords="MQTT performance tests, fast MQTT server, MQTT benchmark, XMQ"
                 path="/xmq_tests_fanin"/>
            <h1>MQTT Performance Tests: Fan-In</h1>
            <h3>Fan-in</h3>

            <p>
                Fan-in is the many-to-few case: a large number of publishers, each on its own
                topic, feeding a small pool of subscribers that share the load through a shared
                subscription. Every message is delivered to exactly one subscriber, so the broker
                does no delivery amplification &mdash; what this measures is ingest at scale plus
                the cost of dispatching across a shared subscription group.
            </p>

            <p>
                50,000 publishers on 50,000 topics at one message per second, so 50,000
                messages/second in aggregate, consumed by 500 subscribers sharing
                {" "}<span style={mono}>$share/benchmark/test/#</span>. QoS&nbsp;1, 16-byte
                payload, 30 minutes. The scenario mirrors the Open MQTT Benchmark Suite's
                {" "}<span style={mono}>singlenode-sharesub-50K-500-50K-50K</span> case so the
                figures can be read against those published there; see the
                {" "}<Link to="/xmq_tests_environment">Test Environment</Link> page for hardware,
                tuning and method.
            </p>

            {error && <p>Failed to load test results: {error}</p>}
            {!servers && !error && <p>Loading results&hellip;</p>}

            <BrokerBenchmark servers={servers} targetRate={50000}/>

            {servers && (
                <>
                    <h4>Reading the results</h4>
                    <ul>
                        <li>
                            <b>XMQ holds the full rate at 217&micro;s</b>, and holds it flat: the
                            ten interval averages span 215&ndash;218&micro;s across the whole
                            thirty minutes. It does so on 2.7 of the 8 vCPUs it is pinned to
                            &mdash; a third of its own allowance, an eighth of the machine &mdash;
                            in 242&nbsp;MB.
                        </li>
                        <li>
                            <b>Mosquitto also holds the rate</b>, at 379&nbsp;ms, and is the one
                            overloaded case here whose latency <i>improves</i> over the run rather
                            than diverging (946&nbsp;ms down to 145&nbsp;ms), on a single core. The
                            cost is memory: 31.4&nbsp;GB peak on a 40&nbsp;GB host. This
                            configuration leaves the in-flight and queued message limits unbounded,
                            so the early backlog is absorbed as heap rather than as dropped
                            messages.
                        </li>
                        <li>
                            <b>EMQX did not reach the offered rate</b> here: 41,958/s of 50,000/s,
                            with latency climbing from 226&nbsp;ms to 260&nbsp;s, while saturating
                            all 16 vCPUs. That does not match EMQX's own published result for this
                            scenario (50,000/s at 2.51&nbsp;ms), and the explanations within our
                            control were checked and ruled out: its configuration follows EMQX's
                            performance tuning guide, the load generator sat at 275% of the 1600%
                            available, memory was never constrained, and the hardware is
                            equivalent. What remains is the broker version (5.8.9 here against
                            5.0.21 there) and the load generator. It is reported as a result for
                            this version under this harness, not as a general statement about EMQX
                            &mdash; particularly since Mosquitto held the same rate on one core
                            while EMQX saturated thirteen, which points at something specific to
                            shared-subscription dispatch in this build.
                        </li>
                    </ul>
                </>
            )}
        </div>;
    }
}
