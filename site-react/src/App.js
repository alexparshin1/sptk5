import './App.css';
import Header from "./pages/Header";
import "./css/style.css";
import {Route, Routes} from "react-router";
import Home from "./pages/Home";
import {BrowserRouter} from "react-router-dom";
import SPTK_screenshots from "./pages/SPTK_screenshots";
import SPTK_themes from "./pages/SPTK_themes";
import SPTK_documentation from "./pages/SPTK_documentation";
import Footer from "./pages/Footer";
import Support from "./pages/Support";
import XMQ_about from "./pages/XMQ_about";
import XMQ_tests_connections from "./pages/XMQ_tests_connections";
import XMQ_tests_environment from "./pages/XMQ_tests_environment";
import XMQ_tests_fanin from "./pages/XMQ_tests_fanin";
import XMQ_tests_fanout from "./pages/XMQ_tests_fanout";
import XMQ_tests_p2p from "./pages/XMQ_tests_p2p";
import XMQ_tests_persistence from "./pages/XMQ_tests_persistence";
import XMQ_mqtt_test_suite from "./pages/XMQ_mqtt_test_suite";
import XMQ_configuration from "./pages/XMQ_configuration";
import XMQ_documentation from "./pages/XMQ_documentation";
import Downloads from "./pages/Downloads";
import SPTK_about from "./pages/SPTK_about";
import AdminChangePassword from "./pages/AdminChangePassword";
import AdminAddNews from "./pages/AdminAddNews";
import Accordion from "./components/Accordion";
import {Component} from "react";
import ControlAPI from "./ControlAPI";
import LocalAuth from "./LocalAuth";

class App extends Component
{
    state = {
        counter: 0,
        loggedIn: LocalAuth.isLoggedIn
    }

    componentDidMount()
    {
        LocalAuth.onChanged = (isLoggedIn) => this.setState({loggedIn: isLoggedIn});
        this.getCounter(window.location.pathname).then(counter => {
            this.setState({counter: counter});
        });
    }

    async getCounter(activePage)
    {
        switch (activePage) {
            case "/":
            case "/home":
                activePage = "/index";
                break;
            default:
                break;
        }
        let sptkCounterName = "alexeyp" + activePage + ".php";
        const result = await ControlAPI.getRequest("counter.php", {countername: sptkCounterName});
        return result ? result.visitors : 0;
    }

    render()
    {
        let accordionMenu = [
            {
                title: "XMQ", items: [
                    {title: "Home", link: "/"},
                    {title: "About", link: "/xmq_about"},
                    {title: "Tests", items: [
                        {title: "Test Environment", link: "/xmq_tests_environment"},
                        {title: "Connections", link: "/xmq_tests_connections"},
                        {title: "Fan-in", link: "/xmq_tests_fanin"},
                        {title: "Fan-out", link: "/xmq_tests_fanout"},
                        {title: "Point-to-point", link: "/xmq_tests_p2p"},
                        {title: "Persistence", link: "/xmq_tests_persistence"},
                        {title: "MQTT Test Suite", link: "/xmq_mqtt_test_suite"},
                    ]},
                    {title: "Configuration", link: "/xmq_configuration"},
                    {title: "Documentation", link: "/xmq_documentation"},
                ]
            },
            {
                title: "SPTK", items: [
                    {title: "About", link: "/sptk_about"},
                    {title: "Screenshots", link: "/sptk_screenshots"},
                    {title: "Themes", link: "/sptk_themes"},
                    {title: "Documentation", link: "/sptk_documentation"},
                ]
            },
            {
                title: "Downloads", items: [
                    {title: "Files", link: "/downloads"},
                ]
            },
            {
                title: "Support", items: [
                    {title: "Prices", link: "/support"},
                ]
            },
        ];
        if (this.state.loggedIn) {
            accordionMenu.push({
                title: "Administration", items: [
                    {title: "Change Password", link: "/admin_change_password"},
                    {title: "Add News", link: "/admin_add_news"},
                ]
            });
        }
        return (
            <div className="App" style={{height: "100%", background: "#ddd"}}>
                <div className='content'>
                    <Header/>
                    <BrowserRouter>
                        {/* Sized in CSS units rather than from window.outerHeight: the
                            pre-rendered markup must not depend on the window it was
                            rendered in, or hydration would find a different height. */}
                        <table style={{width: "100%", minHeight: "calc(100vh - 100px)"}}>
                            <tbody>
                            <tr style={{height: "1.5em"}}>
                                <td></td>
                                <td align="right" style={{width: "100%"}}>
                                    There were {this.state.counter} unique visitors to this page
                                </td>
                            </tr>
                            <tr>
                                <td style={{
                                    verticalAlign: "top",
                                    width: 200,
                                    backgroundColor: "#ddd",
                                    paddingRight: 16
                                }}>
                                    <div style={{height: "1em"}}></div>
                                    <Accordion menu={accordionMenu} onChange={async (page) => {
                                        let counter = await this.getCounter(page);
                                        this.setState({counter: counter});
                                    }}/>
                                </td>
                                <td style={{verticalAlign: "top"}}>
                                    <Routes>
                                        <Route path="/" Component={Home}/>
                                        <Route path="/support" Component={Support}/>
                                        <Route path="/xmq_about" Component={XMQ_about}/>
                                        <Route path="/xmq_tests_environment" Component={XMQ_tests_environment}/>
                                        <Route path="/xmq_tests_connections" Component={XMQ_tests_connections}/>
                                        <Route path="/xmq_tests_fanin" Component={XMQ_tests_fanin}/>
                                        <Route path="/xmq_tests_fanout" Component={XMQ_tests_fanout}/>
                                        <Route path="/xmq_tests_p2p" Component={XMQ_tests_p2p}/>
                                        <Route path="/xmq_tests_persistence" Component={XMQ_tests_persistence}/>
                                        <Route path="/xmq_mqtt_test_suite" Component={XMQ_mqtt_test_suite}/>
                                        <Route path="/xmq_configuration" Component={XMQ_configuration}/>
                                        <Route path="/xmq_documentation" Component={XMQ_documentation}/>
                                        <Route path="/sptk_about" Component={SPTK_about}/>
                                        <Route path="/sptk_screenshots" Component={SPTK_screenshots}/>
                                        <Route path="/sptk_themes" Component={SPTK_themes}/>
                                        <Route path="/sptk_documentation" Component={SPTK_documentation}/>
                                        <Route path="/downloads" Component={Downloads}/>
                                        <Route path="/admin_change_password" Component={AdminChangePassword}/>
                                        <Route path="/admin_add_news" Component={AdminAddNews}/>
                                    </Routes>
                                </td>
                            </tr>
                            </tbody>
                        </table>
                        <Footer/>
                    </BrowserRouter>
                </div>
            </div>);
    }
}

export default App;
