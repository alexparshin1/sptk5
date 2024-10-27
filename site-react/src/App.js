import './App.css';
import Header from "./pages/Header";
import "./css/style.css";
import Menu from "./pages/Menu";
import {Route, Routes} from "react-router";
import Home from "./pages/Home";
import {BrowserRouter} from "react-router-dom";
import Screenshots from "./pages/Screenshots";
import Themes from "./pages/Themes";
import Documentation from "./pages/Documentation";
import Footer from "./pages/Footer";
import Support from "./pages/Support";
import XMQ_about from "./pages/XMQ_about";
import XMQ_tests from "./pages/XMQ_tests";
import XMQ_configuration from "./pages/XMQ_configuration";
import Downloads from "./pages/Downloads";

function App()
{
    return (
        <div className="App" style={{height: "100%", background: "#ddd"}}>
            <div className='content'>
                <Header/>
                <BrowserRouter>
                    <Menu key="main-menu"/>
                    <Routes>
                        <Route path="/" Component={Home}/>
                        <Route path="/screenshots" Component={Screenshots}/>
                        <Route path="/themes" Component={Themes}/>
                        <Route path="/documentation" Component={Documentation}/>
                        <Route path="/support" Component={Support}/>
                        <Route path="/xmq_about" Component={XMQ_about}/>
                        <Route path="/xmq_tests" Component={XMQ_tests}/>
                        <Route path="/xmq_configuration" Component={XMQ_configuration}/>
                        <Route path="/downloads" Component={Downloads}/>
                    </Routes>
                    <Footer/>
                </BrowserRouter>
            </div>
        </div>);
}

export default App;
