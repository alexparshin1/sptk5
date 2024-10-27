import './App.css';
import Header from "./pages/Header";
import "./css/style.css";
import Menu from "./components/Menu";
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
                    <Footer/>
                </BrowserRouter>
            </div>
        </div>);
}

export default App;
