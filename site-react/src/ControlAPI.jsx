export default class ControlAPI {

    static serviceURL = "http://www.sptk.net";
    static isConnected = false;
    static token = "";

    static connected() { return ControlAPI.isConnected; }
    static onConnectedChanged = (isConnected) => {};

    static async getRequest(method, methodParams) {
        const params = new URLSearchParams(methodParams);
        const url = `${ControlAPI.serviceURL}/${method}?${params.toString()}`;
        try {
            const response = await fetch(url);
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            return await response.json();
        } catch (error) {
            console.error(error.message);
            return null;
        }
    }

    /**
     * Performs an API call to the specified `apiMethod` with the given `content` object.
     * @param {string} apiMethod - The name of the API call to make
     * @param {Object} content - The JSON object to pass as the body of the request
     * @return {Promise<Object|null>} A promise that resolves to the JSON response
     *  from the API, or null if the call fails
     */
    static async asyncMakeAPICall(apiMethod, content) {
        try {
            let headers = {'Content-Type': 'application/json'};
            if (ControlAPI.connected()) {
                headers['Authorization'] = 'Bearer ' + ControlAPI.token;
            }
            const response = await fetch(`${ControlAPI.serviceURL}/${apiMethod}`,
                {
                    body: JSON.stringify(content),
                    headers: headers,
                    method: 'POST',
                    mode: 'cors'
                });

            if (!response.ok) {
                const errorData = await response.json().catch(() => ({}));
                const message = errorData?.result?.description || `HTTP error! status: ${response.status}`;
                throw new Error(message);
            }

            return await response.json();
        } catch (e) {
            console.error("API Call failed:", e);
            // alert("XMQ server is offline"); // Keeping it for now as per original logic, but switching to console.error
            return null;
        }
    }

    static async login(username, password) {
        try {
            const result = await ControlAPI.asyncMakeAPICall("Login", {"username": username, "password": password});
            ControlAPI.isConnected = false;

            if (result === null || result.result === null) {
                // XMQ server is offline
                return false;
            }

            if (result.result.success) {
                ControlAPI.token = result.token;
                ControlAPI.isConnected = true;
                ControlAPI.onConnectedChanged(true);
                return true;
            } else {
                alert("Login failed: " + result.result.description);
                return false;
            }
        } catch (e) {
            console.error("Login exception:", e);
            return false;
        }
    }

    static logout() {
        ControlAPI.token = "";
        ControlAPI.isConnected = false;
        ControlAPI.onConnectedChanged(false);
    }

    static async setPersistence(persistence) {
        const result = await ControlAPI.asyncMakeAPICall("PersistenceControl", {action: "set", persistence: persistence});
        if (result === null) {
            // XMQ server is offline
            return false;
        }
        if (result.result.success) {
            alert("Persistence parameters changed, please restart XMQ to take effect");
            return true;
        }
        return false;
    }

    static async installKeys(keys) {
        const result = await ControlAPI.asyncMakeAPICall("SSLKeysControl", {action: "set", keys: keys});
        if (result === null) {
            // XMQ server is offline
            return false;
        }
        if (result.result.success) {
            return true;
        }
        return false;
    }
}
