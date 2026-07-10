import React from "react";
import LocalAuth from "../LocalAuth";
import "../css/AdminForm.css";

export default class AdminChangePassword extends React.Component
{
    state = {
        password: "",
        message: "",
        success: false,
        busy: false
    };

    async onSubmit(event)
    {
        event.preventDefault();
        if (this.state.busy) {
            return;
        }
        this.setState({busy: true});
        const result = await LocalAuth.changePassword(this.state.password);
        this.setState({
            message: result.message,
            success: result.success,
            password: result.success ? "" : this.state.password,
            busy: false
        });
    }

    render()
    {
        if (!LocalAuth.isLoggedIn) {
            return <div className="Page">Please login to access this page.</div>;
        }

        return <div className="Page">
            <h2>Change Password</h2>
            <form className="AdminForm" onSubmit={(e) => this.onSubmit(e)}>
                <label htmlFor="admin-username">Username</label>
                <input id="admin-username" type="text" value={LocalAuth.username} disabled/>
                <label htmlFor="admin-password">Password</label>
                <input id="admin-password" type="password" autoComplete="new-password"
                       value={this.state.password}
                       onChange={(e) => this.setState({password: e.target.value, message: ""})}/>
                {this.state.message &&
                    <div className={this.state.success ? "AdminMessage" : "AdminError"}>{this.state.message}</div>}
                <div className="AdminButtons">
                    <button type="submit" disabled={this.state.busy}>Submit</button>
                </div>
            </form>
        </div>;
    }
}
