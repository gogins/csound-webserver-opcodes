/**
 * All methods that return values are declared "async" so that they can be 
 * called either asynchronously or synchronously.
 */
class Csound {
    constructor(url) {
        this.url = url;
        this.id = 0;
    }
    async invoke_rpc(method, parameters, is_method, on_success, on_error) {
        let fetch_url = this.url + '/' + method;
        this.id = this.id + 1;
        let fetch_request = {
            method: 'POST',
            headers: {
                'Accept': 'application/json',
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
              jsonrpc: '2.0',
              id: this.id,
              method: method,
              params: parameters
            })
        };    
        console.log("\nfetch_request:\n" + JSON.stringify(fetch_request) + "\n");
        const jsonrpc_response = await fetch(fetch_url, fetch_request);
        console.log("\jsonrpc_response:\n" + JSON.stringify(jsonrpc_response) + "\n");
        const jayson = await jsonrpc_response.json();
        console.log("\jayson:\n" + JSON.stringify(jayson) + "\n");
        return jayson;
    }
    async CompileCsdText(csd_text, callbackSuccess, callbackError) {
        var params = {csd_text : csd_text};
        return this.invoke_rpc("CompileCsdText", params, true, callbackSuccess, callbackError);
    }
    async CompileOrc(orc_code, callbackSuccess, callbackError) {
        var params = {orc_code : orc_code};
        return this.invoke_rpc("CompileOrc", params, true, callbackSuccess, callbackError);
    }
    async EvalCode(orc_code, callbackSuccess, callbackError) {
        var params = {orc_code : orc_code};
        return this.invoke_rpc("EvalCode", params, true, callbackSuccess, callbackError);
    }
    async Get0dBFS(callbackSuccess, callbackError) {
        var params = null;
        return this.invoke_rpc("Get0dBFS", params, true, callbackSuccess, callbackError);
    }
    async GetAudioChannel(channel_name, callbackSuccess, callbackError) {
        var params = {channel_name : channel_name};
        return this.invoke_rpc("GetAudioChannel", params, true, callbackSuccess, callbackError);
    };
    async GetControlChannel(channel_name, callbackSuccess, callbackError) {
        var params = {channel_name : channel_name};
        return this.invoke_rpc("GetControlChannel", params, true, callbackSuccess, callbackError);
    };
    async GetDebug(callbackSuccess, callbackError) {
        var params = null;
        return this.invoke_rpc("GetDebug", params, true, callbackSuccess, callbackError);
    };
    async GetKsmps(callbackSuccess, callbackError) {
        var params = null;
        return this.invoke_rpc("GetKsmps", params, true, callbackSuccess, callbackError);
    };
    async GetNchnls(callbackSuccess, callbackError) {
        var params = null;
        return this.invoke_rpc("GetNchnls", params, true, callbackSuccess, callbackError);
    };
    async GetNchnlsInput(callbackSuccess, callbackError) {
        var params = null;
        return this.invoke_rpc("GetNchnlsInput", params, true, callbackSuccess, callbackError);
    };
    async GetScoreOffsetSeconds(callbackSuccess, callbackError) {
        var params = null;
        return this.invoke_rpc("GetScoreOffsetSeconds", params, true, callbackSuccess, callbackError);
    };
    async GetScoreTime(callbackSuccess, callbackError) {
        var params = null;
        return this.invoke_rpc("GetScoreTime", params, true, callbackSuccess, callbackError);
    };
    async GetSr(callbackSuccess, callbackError) {
        var params = null;
        return this.invoke_rpc("GetSr", params, true, callbackSuccess, callbackError);
    };
    async GetStringChannel(channel_name, callbackSuccess, callbackError) {
        var params = {channel_name : channel_name};
        return this.invoke_rpc("GetStringChannel", params, true, callbackSuccess, callbackError);
    };
    async InputMessage(sco_code, callbackSuccess, callbackError) {
        var params = {sco_code : sco_code};
        return this.invoke_rpc("InputMessage", params, true, callbackSuccess, callbackError);
    };
    async IsScorePending(callbackSuccess, callbackError) {
        var params = null;
        return this.invoke_rpc("IsScorePending", params, true, callbackSuccess, callbackError);
    };
    async Message(message, callbackSuccess, callbackError) {
        var params = {message : message};
        this.invoke_rpc("Message", params, false, callbackSuccess, callbackError);
    };
    async ReadScore(sco_code, callbackSuccess, callbackError) {
        var params = {sco_code : sco_code};
        return this.invoke_rpc("ReadScore", params, true, callbackSuccess, callbackError);
    };
    async RewindScore(callbackSuccess, callbackError) {
        var params = null;
        return this.invoke_rpc("RewindScore", params, true, callbackSuccess, callbackError);
    };
    async ScoreEvent(opcode_code, pfields, callbackSuccess, callbackError) {
        var params = {opcode_code : opcode_code, pfields : pfields};
        return this.invoke_rpc("ScoreEvent", params, true, callbackSuccess, callbackError);
    };
    async SetControlChannel(channel_name, channel_value, callbackSuccess, callbackError) {
        var params = {channel_name : channel_name, channel_value : channel_value};
        return this.invoke_rpc("SetControlChannel", params, true, callbackSuccess, callbackError);
    };
    async SetDebug(enabled, callbackSuccess, callbackError) {
        var params = {enabled : enabled};
        return this.invoke_rpc("SetDebug", params, true, callbackSuccess, callbackError);
    };
    async SetMessageCallback(callback, callbackSuccess, callbackError) {
        var params = {callback : callback};
        return this.invoke_rpc("SetMessageCallback", params, true, callbackSuccess, callbackError);
    };
    async SetScoreOffsetSeconds(score_time, callbackSuccess, callbackError) {
        var params = {score_time : score_time};
        return this.invoke_rpc("SetScoreOffsetSeconds", params, true, callbackSuccess, callbackError);
    };
    async SetScorePending(pending, callbackSuccess, callbackError) {
        var params = {pending : pending};
        return this.invoke_rpc("SetScorePending", params, true, callbackSuccess, callbackError);
    };
    async SetStringChannel(channel_name, channel_value, callbackSuccess, callbackError) {
        var params = {channel_name : channel_name, channel_value : channel_value};
        return this.invoke_rpc("SetStringChannel", params, true, callbackSuccess, callbackError);
    };
    async TableLength(table_number, callbackSuccess, callbackError) {
        var params = {table_number : table_number};
        return this.invoke_rpc("TableLength", params, true, callbackSuccess, callbackError);
    };
    async TableGet(index, table_number, callbackSuccess, callbackError) {
        var params = {index : index, table_number : table_number};
        return this.invoke_rpc("TableGet", params, true, callbackSuccess, callbackError);
    };
    async TableSet(index, table_number, value, callbackSuccess, callbackError) {
        var params = {index : index, table_number : table_number, value : value};
        return this.invoke_rpc("TableSet", params, true, callbackSuccess, callbackError);
    };
};
