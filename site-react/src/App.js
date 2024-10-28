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
import XMQ_tests from "./pages/XMQ_tests";
import XMQ_configuration from "./pages/XMQ_configuration";
import Downloads from "./pages/Downloads";
import SPTK_about from "./pages/SPTK_about";
import Accordion from "./components/Accordion";
import {Component} from "react";
import ControlAPI from "./ControlAPI";

class App extends Component
{
    state = {
        counter: 0
    }

    constructor()
    {
        super();
        this.state.counter = this.getCounter("/");
    }

    getCounter(activePage)
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
        let counter = ControlAPI.getRequest("counter.php", {countername: sptkCounterName}).visitors;
        return counter;
    }

    render()
    {
        let accordionMenu = [
            {
                title: "SPTK", items: [
                    {title: "Home", link: "/"},
                    {title: "About", link: "/sptk_about"},
                    {title: "Screenshots", link: "/sptk_screenshots"},
                    {title: "Themes", link: "/sptk_themes"},
                    {title: "Documentation", link: "/sptk_documentation"},
                ]
            },
            {
                title: "XMQ", items: [
                    {title: "About", link: "/xmq_about"},
                    {title: "Tests", link: "/xmq_tests"},
                    {title: "Configuration", link: "/xmq_configuration"},
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
        return (
            <div className="App" style={{height: "100%", background: "#ddd"}}>
                <div className='content'>
                    <Header/>
                    <BrowserRouter>
                        <table style={{width: "100%", height: window.outerHeight - 100}}>
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
                                    <Accordion menu={accordionMenu} onChange={(page) => {
                                        let counter = this.getCounter(page);
                                        this.setState({counter: counter});
                                    }}/>
                                </td>
                                <td style={{verticalAlign: "top"}}>
                                    <Routes>
                                        <Route path="/" Component={Home}/>
                                        <Route path="/support" Component={Support}/>
                                        <Route path="/xmq_about" Component={XMQ_about}/>
                                        <Route path="/xmq_tests" Component={XMQ_tests}/>
                                        <Route path="/xmq_configuration" Component={XMQ_configuration}/>
                                        <Route path="/sptk_about" Component={SPTK_about}/>
                                        <Route path="/sptk_screenshots" Component={SPTK_screenshots}/>
                                        <Route path="/sptk_themes" Component={SPTK_themes}/>
                                        <Route path="/sptk_documentation" Component={SPTK_documentation}/>
                                        <Route path="/downloads" Component={Downloads}/>
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
