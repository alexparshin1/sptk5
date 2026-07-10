import {sha256} from "js-sha256";
import ControlAPI from "./ControlAPI";

/**
 * Site login backed by the users table on the server.
 *
 * The raw password never leaves the browser: the client computes
 * secret = SHA-256(username + ":" + password) and sends that to
 * site_login.php, which stores it bcrypt-hashed (password_hash) in the
 * users table and verifies it with password_verify on later logins.
 *
 * On the very first login of an unknown username the server accepts and
 * stores the entered credentials.
 *
 * The secret is kept in memory for the session so that
 * site_change_password.php can verify the current password before
 * replacing it.
 */
export default class LocalAuth {

    static isLoggedIn = false;
    static username = "";
    static secret = "";
    static onChanged = (isLoggedIn) => {};

    static computeSecret(username, password) {
        return sha256(username + ":" + password);
    }

    /**
     * Verifies the credentials against the users table, or stores them
     * if the username isn't known yet.
     * @return {Promise<Object>} {success: boolean, message: string}
     */
    static async login(username, password) {
        if (!username || !password) {
            return {success: false, message: "Please enter username and password"};
        }

        const secret = LocalAuth.computeSecret(username, password);
        const response = await ControlAPI.asyncMakeAPICall("site_login.php",
            {username: username, secret: secret});
        if (!response || !response.result) {
            return {success: false, message: "Server is not available"};
        }
        if (!response.result.success) {
            return {success: false, message: response.result.description};
        }

        LocalAuth.isLoggedIn = true;
        LocalAuth.username = username;
        LocalAuth.secret = secret;
        LocalAuth.onChanged(true);
        return {success: true, message: response.result.description};
    }

    /**
     * Replaces the current user's password in the users table,
     * keeping the username.
     * @return {Promise<Object>} {success: boolean, message: string}
     */
    static async changePassword(password) {
        if (!LocalAuth.isLoggedIn) {
            return {success: false, message: "Not logged in"};
        }
        if (!password) {
            return {success: false, message: "Please enter a password"};
        }

        const newSecret = LocalAuth.computeSecret(LocalAuth.username, password);
        const response = await ControlAPI.asyncMakeAPICall("site_change_password.php",
            {username: LocalAuth.username, secret: LocalAuth.secret, newSecret: newSecret});
        if (!response || !response.result) {
            return {success: false, message: "Server is not available"};
        }
        if (!response.result.success) {
            return {success: false, message: response.result.description};
        }

        LocalAuth.secret = newSecret;
        return {success: true, message: response.result.description};
    }

    static logout() {
        LocalAuth.isLoggedIn = false;
        LocalAuth.username = "";
        LocalAuth.secret = "";
        LocalAuth.onChanged(false);
    }
}
