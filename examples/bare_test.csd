<CsoundSynthesizer>
<CsLicense>
A very basic test of the Csound webserver opcodes.
</CsLicense>
<CsOptions>
</CsOptions>
-m165 -odac0
<CsInstruments>
sr = 48000
nchnls = 2
ksmps = 100
0dbfs = 10

i_webserver webserver_create "/Users/michaelgogins/csound-webserver-opcodes/examples/", 8080, 1 

webserver_open_resource i_webserver, "bare_test.html", "open"

webserver_open_resource i_webserver, "https://csound.com/docs/manual/indexframes.html", "open"

gS_html init {{
<html>
<head>
</head>
<body>
Hello, world, from Csound's internal Web server, with embedded HTML!
</body>
</html>
}}

webserver_open_html i_webserver, gS_html, "open"

gS_html_rpc init {{
<html>
<script src="https://code.jquery.com/jquery-3.6.0.js" integrity="sha256-H+K7U5CnXl1h5ywQfKtSj8PCmoN9aaq30gDh27Xc0jk=" crossorigin="anonymous"></script>
<script src="csound.js"></script>
</script>
<head>
</head>
<body>
<p>
Hello, World, from Csound's internal Web server, with embedded HTML and RPC to Csound!
</p>
<textarea id='log_textarea' cols=80 rows=25>
</textarea>
<script>
function console_log(message) {
    var log_textarea = document.getElementById("log_textarea");
    var existing = log_textarea.value;
    log_textarea.value = existing + message;
    log_textarea.scrollTop = log_textarea.scrollHeight;   
};
console.log = console_log;
console.log("origin: " + origin);
csound = new Csound(origin);
var onSuccess = function(id, response) {
    console.info("\\nid: " + id + " response:" + response);
    return response;
};
var onError = function(id, error) {
    console.info("\\nid: " + id + " error:" + error);
    return error;
};

csound.EvalCode("prints \\"Hello from the browser (" + navigator.userAgent + ") via JSON-RPC!\\"\\n\\n", onSuccess, onError);
csound.Get0dBFS(onSuccess, onError);
// All Csound functions that return values are declared async. Here's how to 
// synchronously wait for the return value. Any number of `await` calls can be 
// made inside the anonymous async function.
(async () => {
    let zdbfs = await csound.Get0dBFS(onSuccess, onError);
    console.log("\\n0dBFS: " + zdbfs);
})();
</script>
</body>
</html>
}
}}

webserver_open_html i_webserver, gS_html_rpc, "open"

prints "Now starting to run..."

</CsInstruments>
<CsScore>
f 0 120
</CsScore>
</CsoundSynthesizer>
