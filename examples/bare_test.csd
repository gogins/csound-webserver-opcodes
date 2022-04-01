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

gS_html init {{<html>
<head>
</head>
<body>
Hello, world, from Csound's internal Web server, with embedded HTML!
</body>
</html>
}}

webserver_open_html i_webserver, gS_html, "open"

prints "Now starting to run..."

</CsInstruments>
<CsScore>
f 0 120
</CsScore>
</CsoundSynthesizer>
