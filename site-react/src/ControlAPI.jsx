export default class ControlAPI {

    static serviceURL = "http://sptk.net";

    static getRequest(method, methodParams) {
        const params = new URLSearchParams(methodParams);
        const url = this.serviceURL + "/" + method + "?" + params.toString();
        try {
            var xhr = new XMLHttpRequest();
            xhr.open('GET', url, false);
            xhr.send(null);
            return JSON.parse(xhr.responseText);
        } catch (error) {
            console.error(error.message);
        }
    }

    /**
     * Performs an API call to the specified `apiMethod` with the given `content` object.
     * @param {string} apiMethod - The name of the API call to make
     * @param {Object} content - The JSON object to pass as the body of the request
     * @return {Promise<Object|Error>} A promise that resolves to the JSON response
     *  from the API, or rejects with an error if the call fails
     */
    static async asyncMakeAPICall(apiMethod, content) {
        try {
            let headers = {'Content-Type': 'application/json'};
            if (ControlAPI.connected()) {
                headers['Authorization'] = 'Bearer ' + ControlAPI.token;
            }
            const response = await fetch(this.serviceURL + '/' + apiMethod,
                {
                    body: JSON.stringify(content),
                    headers: headers,
                    method: 'POST',
                    mode: 'cors'
                });
            return await response.json();
        } catch (e) {
            alert("XMQ server is offline");
            console.log(e);
            return null;
        }
    }

    static login(username, password) {
        ControlAPI.asyncMakeAPICall("Login", {"username": username, "password": password})
            .then(result => {
                if (result === null || result.result === null) {
                    // XMQ server is offline
                    return false;
                }
                if (result.result.success) {
                    ControlAPI.token = result.token;
                    ControlAPI.onConnectedChanged(true);
                    return true;
                } else {
                    alert("Login failed: " + result.result.description);
                    return false;
                }
            });
    }

    static logout() {
        ControlAPI.token = "";
        ControlAPI.onConnectedChanged(false);
    }

    static setPersistence(persistence) {
        ControlAPI.asyncMakeAPICall("PersistenceControl", {action: "set", persistence: persistence})
            .then(result => {
                if (result === null) {
                    // XMQ server is offline
                    return false;
                }
                if (result.result.success) {
                    alert("Persistence parameters changed, please restart XMQ to take effect");
                    return true;
                }
            });
    }

    static installKeys(keys) {
        ControlAPI.asyncMakeAPICall("SSLKeysControl", {action: "set", keys: keys})
            .then(result => {
                if (result === null) {
                    // XMQ server is offline
                    return false;
                }
                if (result.result.success) {
                    return true;
                }
            });
    }
}
