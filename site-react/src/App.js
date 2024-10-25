import './App.css';
import Header from "./pages/Header";
import "./css/style.css";
import MainMenu from "./pages/MainMenu";
import {Route, Routes} from "react-router";
import Home from "./pages/Home";
import {BrowserRouter} from "react-router-dom";
import Screenshots from "./pages/Screenshots";

function App() {
  return (
      <div className="App" style={{height: "100%"}}>
          <div className='content'>
              <Header/>
              <BrowserRouter>
                  <MainMenu/>
                  <Routes>
                      <Route path="/" Component={Home} />
                      <Route path="/screenshots" Component={Screenshots} />
                  </Routes>
              </BrowserRouter>
          </div>
      </div>);
}

export default App;
