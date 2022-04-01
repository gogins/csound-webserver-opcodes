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
0dbfs = 2

i_webserver webserver_create "/Users/michaelgogins/csound-webserver/examples/", 8080, 1 

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
Hello, world, from Csound's internal Web server, with embedded HTML and RPC to Csound!
<script>
document.write("\\norigin: " + origin);
csound = new Csound(origin);
var onSuccess = function(id, response) {
    document.write("\\nid: " + id + " response:" + response);
    return response;
};
var onError = function(id, error) {
    document.write("\\nid: " + id + " error:" + error);
    return error;
}
csound.CompileCsdText("2 + 2", onSuccess, onError);
</script>
</body>
</html>
}}

webserver_open_html i_webserver, gS_html_rpc, "open"

prints "Now starting to run..."

</CsInstruments>
<CsScore>
f 0 120
</CsScore>
</CsoundSynthesizer>
