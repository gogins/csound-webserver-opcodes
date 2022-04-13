<CsoundSynthesizer>
<CsLicense>
A very basic test of the Csound webserver opcodes.
</CsLicense>
<CsOptions>
-d -m165 -odac0
</CsOptions>
<CsInstruments>
sr = 48000
nchnls = 2
ksmps = 100
// Doesn't work in this context:
// 0dbfs = 5
// Multiply final output by:
gi_1odbfs init (1. / 5.)
print gi_1odbfs

gS_open init "open"

// i_webserver webserver_create "/Users/michaelgogins/csound-webserver-opcodes/examples/", 8080, 1
i_webserver webserver_create "/home/mkg/csound-webserver-opcodes/examples/", 8080, 1

webserver_open_resource i_webserver, "bare_test.html", gS_open

webserver_open_resource i_webserver, "https://csound.com/docs/manual/indexframes.html", gS_open

gS_html_rpc init {{
<html>
<script src="https://code.jquery.com/jquery-3.6.0.js" integrity="sha256-H+K7U5CnXl1h5ywQfKtSj8PCmoN9aaq30gDh27Xc0jk=" crossorigin="anonymous"></script>
<script src="csound.js"></script>
</script>
<head>
</head>
<body>
<p>
Hello, World, from Csound's internal Web server, with embedded HTML and JSON-RPC!
</p>
<textarea id='log_textarea' cols=80 rows=25>
</textarea>
<textarea id='orc' cols=80 rows=25 style="visibility:hidden;">
; Coded by Hans Mikelson October 1999

        zakinit  50,50                      ; May need this later

;---------------------------------------------------------
; Terrain Lead 1
;---------------------------------------------------------
       instr     1

idur   =         p3            ; Duration
iamp   =         p4            ; Amplitude
ifqc   =         cpspch(p5)    ; Pitch to frequency
ipanl  =         sqrt(p6)      ; Pan left
ipanr  =         sqrt(1-p6)    ; Pan right
iplsf  =         p7            ; Frequency modifier
iwahrt =         p8            ; Wah rate
iwahtb =         p9            ; Wah table
ipbnd  =         p10           ; Pitch Bend Table

kpbnd  oscili    1, 1/idur, ipbnd                      ; Pitch bend
kwrt   linseg    1, idur*.3, 1.5, idur*.7, .2          ; Pulse width rate
kdclk  linseg    0, idur*.3, 1, idur*.4, 1, idur*.3, 0 ; Declick

asin   oscili    1, iplsf, 1         ; Sine wave pulse frequency
ar     =         asin*asin           ; Make it positive

armp   oscili    100, iwahrt*kwrt, iwahtb ; PWM
ayi    oscili    ar*armp, iplsf, 1   ; Y
ayf    =         .1/(.1+ayi*ayi)     ; Sort of a square pulse wave at this point

asig   oscili    1, ifqc*kpbnd, 1    ; Sine oscillator with pitch bend

       outs      gi_1odbfs*ayf*iamp*asig*kdclk*ipanl, gi_1odbfs*ayf*iamp*asig*kdclk*ipanr ;Output the sound

       endin
</textarea>
<textarea id='sco' cols=80 rows=25 style="visibility:hidden;">

f1  0 65537 10 1

f20 0 1025  5  .01 512 1 513 .01
f21 0 1025  7  .01 512 1 513 .01
f22 0 1025  10 .8 .5 .1 .2

f11 0 1025 -5  .5  256 1  256 4   256 2  257 2
f12 0 1025 -5  1   256 1  256 2   256 2  257 .25
f13 0 1025 -5  .75 256 1  256 1   256 .5 257 1
f14 0 1025 -5  2   256 .5 256 1   256 1  257 2
f15 0 1025 -5  1   256 2  256 2   256 2  257 2

;t 0 20

;    Sta     Dur  Amp    Pitch  Pan  PlsFqc WahRate  WahTable  PtchBend
i1   0       16   8000   7.06   .5   80     .5       20        13
i1   8       15   6000   8.00   0    32     .2       21        11
i1   11      17  13000   7.00   1    10     .4       22        12
i1   12      16  12000   8.07   .3   30     .2       20        14
i1   16      20  11000   9.05   .8   120    .3       21        15
i1   18      23   7000   10.03  .5   25     .1       22        11
i1   20      22   5000   7.00   .5   46     .4       20        12
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
// All Csound functions that return values are declared async. Here's how to 
// synchronously wait for the return value. Any number of `await` calls can be 
// made inside the anonymous async function.
(async () => {
    //let i_result = await csound.EvalCode("return 2.5\\n", onSuccess, onError);
    let zdbfs = await csound.Get0dBFS(onSuccess, onError);
    console.log("\\n0dBFS returned: " + zdbfs);
    await csound.Message("This message is from the embedded HTML.\\n");
    var orc = document.getElementById('orc').value;
    await csound.CompileOrc(orc);
    var sco = document.getElementById('sco').value;
    await csound.ReadScore(sco);
})();
</script>
</body>
</html>
}
}}

webserver_open_html i_webserver, gS_html_rpc, gS_open

prints "Csound performance is starting...\\n"

</CsInstruments>
<CsScore>
f 0 30
</CsScore>
</CsoundSynthesizer>
