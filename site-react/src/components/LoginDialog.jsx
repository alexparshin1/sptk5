import React from "react";
import LocalAuth from "../LocalAuth";
import "./LoginDialog.css";

/**
 * @brief Login popup dialog
 * Props:
 * - onClose(success): called with true after a successful login,
 *   or false when the dialog is cancelled
 */
export default class LoginDialog extends React.Component
{
    state = {
        username: "",
        password: "",
        error: "",
        busy: false
    };

    async onSubmit(event)
    {
        event.preventDefault();
        if (this.state.busy) {
            return;
        }
        this.setState({busy: true});
        const result = await LocalAuth.login(this.state.username, this.state.password);
        if (result.success) {
            this.props.onClose(true);
        } else {
            this.setState({error: result.message, busy: false});
        }
    }

    render()
    {
        return <div className="LoginOverlay" onClick={() => this.props.onClose(false)}>
            <form className="LoginDialog" onClick={(e) => e.stopPropagation()}
                  onSubmit={(e) => this.onSubmit(e)}>
                <div className="LoginTitle">Login</div>
                <label htmlFor="login-username">Username</label>
                <input id="login-username" type="text" autoFocus autoComplete="username"
                       value={this.state.username}
                       onChange={(e) => this.setState({username: e.target.value, error: ""})}/>
                <label htmlFor="login-password">Password</label>
                <input id="login-password" type="password" autoComplete="current-password"
                       value={this.state.password}
                       onChange={(e) => this.setState({password: e.target.value, error: ""})}/>
                {this.state.error && <div className="LoginError">{this.state.error}</div>}
                <div className="LoginButtons">
                    <button type="submit" disabled={this.state.busy}>Login</button>
                    <button type="button" onClick={() => this.props.onClose(false)}>Cancel</button>
                </div>
            </form>
        </div>;
    }
}
