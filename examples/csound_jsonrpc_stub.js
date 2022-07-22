/**
 * This class provides a JSON-RPC interface to an already running instance of 
 * a csound_webserver opcode, and thus to Csound itself.
 */
const diagnostics_enabled = true;

class Csound {
    constructor(url) {
        this.url = url;
        this.id = 0;
    }
    async invoke_rpc(method, parameters) {
        let fetch_url = this.url + '/' + method;
        this.id = this.id + 1;
        let message_body = JSON.stringify({
              jsonrpc: '2.0',
              id: this.id,
              method: method,
              params: parameters
            });
        let fetch_request = {
            method: 'POST',
            headers: {
                'Accept': 'application/json',
                'Content-Type': 'application/json',
                'Content-Length': message_body.length
            },
            body: message_body
        };    
        if (diagnostics_enabled == true) console.log("\nfetch_request:\n" + JSON.stringify(fetch_request) + "\n");
        const jsonrpc_response = await fetch(fetch_url, fetch_request);
        if (diagnostics_enabled == true) console.log("\jsonrpc_response:\n" + JSON.stringify(jsonrpc_response) + "\n");
        const jsonrpc_result = jsonrpc_response.json();
        if (diagnostics_enabled == true) console.log("\jsonrpc_result:\n" + jsonrpc_result.result + "\n");
        // Returns not the JSON of the result, but the _value_ of the result.
        return jsonrpc_result.result;
    }
    invoke_notification(method, parameters) {
        let fetch_url = this.url + '/' + method;
        this.id = this.id + 1;
        let message_body = JSON.stringify({
              jsonrpc: '2.0',
              method: method,
              params: parameters
            });
        let fetch_request = {
            method: 'POST',
            headers: {
                'Accept': 'application/json',
                'Content-Type': 'application/json',
                'Content-Length': message_body.length
            },
            body: message_body
        };    
        if (diagnostics_enabled == true) console.log("\nfetch_request:\n" + JSON.stringify(fetch_request) + "\n");
        fetch(fetch_url, fetch_request);
    }
    CompileCsdText(csd_text) {
        let params = {csd_text : csd_text};
        return this.invoke_rpc("CompileCsdText", params);
    }
    CompileOrc(orc_code) {
        let params = {orc_code : orc_code};
        return this.invoke_rpc("CompileOrc", params);
    }
    EvalCode(orc_code) {
        let params = {orc_code : orc_code};
        return this.invoke_rpc("EvalCode", params);
    }
    Get0dBFS(callbackSuccess, callbackError) {
        let params = null;
        return this.invoke_rpc("Get0dBFS", params);
    }
    GetAudioChannel(channel_name) {
        let params = {channel_name : channel_name};
        return this.invoke_rpc("GetAudioChannel", params);
    };
    GetControlChannel(channel_name) {
        let params = {channel_name : channel_name};
        return this.invoke_rpc("GetControlChannel", params);
    };
    GetDebug(callbackSuccess, callbackError) {
        let params = null;
        return this.invoke_rpc("GetDebug", params);
    };
    GetKsmps(callbackSuccess, callbackError) {
        let params = null;
        return this.invoke_rpc("GetKsmps", params);
    };
    GetNchnls(callbackSuccess, callbackError) {
        let params = null;
        return this.invoke_rpc("GetNchnls", params);
    };
    GetNchnlsInput(callbackSuccess, callbackError) {
        let params = null;
        return this.invoke_rpc("GetNchnlsInput", params);
    };
    GetScoreOffsetSeconds(callbackSuccess, callbackError) {
        let params = null;
        return this.invoke_rpc("GetScoreOffsetSeconds", params);
    };
    GetScoreTime(callbackSuccess, callbackError) {
        let params = null;
        return this.invoke_rpc("GetScoreTime", params);
    };
    GetSr(callbackSuccess, callbackError) {
        let params = null;
        return this.invoke_rpc("GetSr", params);
    };
    GetStringChannel(channel_name) {
        let params = {channel_name : channel_name};
        return this.invoke_rpc("GetStringChannel", params);
    };
    InputMessage(sco_code) {
        let params = {sco_code : sco_code};
        return this.invoke_notification("InputMessage", params);
    };
    IsScorePending(callbackSuccess, callbackError) {
        let params = null;
        return this.invoke_rpc("IsScorePending", params);
    };
    Message(message) {
        let params = {message : message};
        this.invoke_notification("Message", params, false);
    };
    ReadScore(sco_code) {
        let params = {sco_code : sco_code};
        return this.invoke_notification("ReadScore", params);
    };
    RewindScore(callbackSuccess, callbackError) {
        let params = null;
        return this.invoke_rpc("RewindScore", params);
    };
    ScoreEvent(opcode_code, pfields) {
        let params = {opcode_code : opcode_code, pfields : pfields};
        return this.invoke_notification("ScoreEvent", params);
    };
    SetControlChannel(channel_name, channel_value) {
        let params = {channel_name : channel_name, channel_value : channel_value};
        return this.invoke_notification("SetControlChannel", params);
    };
    SetDebug(enabled) {
        let params = {enabled : enabled};
        return this.invoke_rpc("SetDebug", params);
    };
    /** 
     * To use this, the Csound orchestra must use the `webserver_send` opcode 
     * to create an event stream and send JSON-encoded messages in it. These 
     * messages will first be decoded, then received in the callback.
     */
    SetEventSourceCallback(event_stream_name, callback) {
        // The event source URL will become: origin + "/" + event_stream_name.
        let event_source = new EventSource(event_stream_name);
        event_source.onmessage = function(event) {
            try {
                let parsed_data = JSON.parse(event.data);
                callback(parsed_data);
            } catch (e) {
                console.log("Failed to parse: ");
                console.log(typeof event.data);
                console.log(event.data);
                console.log(e);
           };
        };
    };
    /** 
     * Implements csoundSetMessageCallback using server-sent events.
     */
    SetMessageCallback(callback) {
        var channel_name = "csound_message_callback";
        let params = {channel_name : channel_name};
        this.SetEventSourceCallback(channel_name, callback);
        return this.invoke_rpc("SetMessageCallback", params);
    };
    SetScoreOffsetSeconds(score_time) {
        let params = {score_time : score_time};
        return this.invoke_rpc("SetScoreOffsetSeconds", params);
    };
    SetScorePending(pending) {
        let params = {pending : pending};
        return this.invoke_rpc("SetScorePending", params);
    };
    SetStringChannel(channel_name, channel_value) {
        let params = {channel_name : channel_name, channel_value : channel_value};
        return this.invoke_notification("SetStringChannel", params, false);
    };
    TableLength(table_number) {
        let params = {table_number : table_number};
        return this.invoke_rpc("TableLength", params);
    };
    TableGet(index, table_number) {
        let params = {index : index, table_number : table_number};
        return this.invoke_rpc("TableGet", params);
    };
    TableSet(index, table_number, value) {
        let params = {index : index, table_number : table_number, value : value};
        return this.invoke_rpc("TableSet", params, false);
    };
};
