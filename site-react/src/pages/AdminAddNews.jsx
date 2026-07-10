import React from "react";
import ControlAPI from "../ControlAPI";
import LocalAuth from "../LocalAuth";
import "../css/AdminForm.css";

export default class AdminAddNews extends React.Component
{
    state = {
        date: new Date().toISOString().slice(0, 10),
        title: "",
        news: "",
        message: "",
        success: false,
        busy: false
    };

    async onSubmit(event)
    {
        event.preventDefault();

        if (!this.state.title || !this.state.news) {
            this.setState({message: "Please enter title and news text", success: false});
            return;
        }

        this.setState({busy: true, message: ""});
        const response = await ControlAPI.asyncMakeAPICall("site_add_news.php", {
            date: this.state.date,
            title: this.state.title,
            news: this.state.news
        });

        if (response && response.result && response.result.success) {
            this.setState({busy: false, message: "News added", success: true, title: "", news: ""});
        } else {
            const description = response && response.result ? response.result.description : "Server is not available";
            this.setState({busy: false, message: "Failed to add news: " + description, success: false});
        }
    }

    render()
    {
        if (!LocalAuth.isLoggedIn) {
            return <div className="Page">Please login to access this page.</div>;
        }

        return <div className="Page">
            <h2>Add News</h2>
            <form className="AdminForm" onSubmit={(e) => this.onSubmit(e)}>
                <label htmlFor="news-date">Date</label>
                <input id="news-date" type="date" value={this.state.date}
                       onChange={(e) => this.setState({date: e.target.value, message: ""})}/>
                <label htmlFor="news-title">Title</label>
                <input id="news-title" type="text" value={this.state.title}
                       onChange={(e) => this.setState({title: e.target.value, message: ""})}/>
                <label htmlFor="news-text">News</label>
                <textarea id="news-text" value={this.state.news}
                          onChange={(e) => this.setState({news: e.target.value, message: ""})}/>
                {this.state.message &&
                    <div className={this.state.success ? "AdminMessage" : "AdminError"}>{this.state.message}</div>}
                <div className="AdminButtons">
                    <button type="submit" disabled={this.state.busy}>Submit</button>
                </div>
            </form>
        </div>;
    }
}
