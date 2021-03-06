<CsoundSynthesizer>
<CsLicense>
Message From Another Planet (v4)  (Spring 1999, Fall 2017, Summer 2022)

Composed By Jacob Joaquin
Modified by Michael Gogins

To search for extraterrestrial intelligence from your home computer visit
https://setiathome.berkeley.edu/.

This piece demonstrates the use of the Csound embedded Web server opcodes
to display an interactive graphical user interface for the piece. This 
version of this piece is modified to work with csound-webserver-opcodes.
</CsLicense>
<CsOptions>
--m-amps=1 --m-range=1 --m-dB=1 --m-benchmarks=1 --m-warnings=0 -+msg_color=0 -d -odac
</CsOptions>
<CsInstruments>
sr              =           48000
ksmps           =           128
nchnls          =           2

gasendL         init        0
gasendR         init        0

gk2_spread      chnexport   "gk2_spread", 3 ; 0
gk2_bass_gain   chnexport   "gk2_bass_gain", 3 ; ""
gk2_spread      init        1.0125
gk2_bass_gain   init        0.005

                instr       Spreader
    kpch        =           cpspch(p5)
    imult       =           ((p6+1) / p6) * .5
    iseed       =           (rnd(100)+100)/200
    k2          expseg      .0625, 15, .0625, 113, 3.75, 113, .03125, 15, .0625
    k3          linseg      1, 240, 1, 16, 0
    k1          phasor      p7 * k2
    k1          tablei      256 * k1 , 100, 0, 0, 1
    krand       randi       30000, p7 * 5, iseed
    krand       =           (krand + 30000) / 60000
    kcps        =           kpch * p6 * gk2_spread
    kamp        =           p4 * imult * k1 * k3 * pow(kcps, -gk2_bass_gain)
    a1          poscil      kamp, kcps, 1
    aoutleft    =           a1 * sqrt(krand)
    aoutright   =           a1 * (sqrt(1-krand))
    gasendL     =           gasendL + aoutleft
    gasendR     =           gasendR + aoutright
                prints      "%-24s i %9.4f %9.4f %9.4f %9.4f %9.4f %9.4f #%3d\\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
                endin

gk_reverb_delay chnexport   "gk_reverb_delay", 3 ; 0
gk_reverb_hipass chnexport  "gk_reverb_hipass", 3 ; ""
gk_master_level chnexport   "gk_master_level", 3 ; ""
gk_reverb_delay init    .89
gk_reverb_hipass init   12000
gk_master_level init    -6
                instr       reverb
aL, aR          reverbsc    gasendL,gasendR,gk_reverb_delay,gk_reverb_hipass
kgain           =           ampdb(gk_master_level)
                outs        aL * kgain, aR * kgain
                clear       gasendL, gasendR
                prints      "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
                endin
                alwayson    "reverb"

//////////////////////////////////////////////////////////////////////////
// This is all of the HTML5 code for the embedded Web page that controls 
// this piece.
//////////////////////////////////////////////////////////////////////////  
gS_html init {{
<!DOCTYPE html>
<html>
<head>
    <title>Message from Another Planet version 4</title>
    <style type="text/css">
    input[type='range'] {
        -webkit-appearance: none;
        box-shadow: inset 0 0 5px #333;
        background-color: gray;
        height: 10px;
        width: 100%;
        vertical-align: middle;
    }
    input[type=range]::-webkit-slider-thumb {
        -webkit-appearance: none;
        border: none;
        height: 16px;
        width: 16px;
        border-radius: 50%;
        box-shadow: inset 0 0 7px #234;
        background: chartreuse;
        margin-top: -4px;
        border-radius: 10px;
    }
    table td {
        border-width: 2px;
        padding: 8px;
        border-style: solid;
        border-color: transparent;
        color:yellow;
        background-color: teal;
        font-family: sans-serif
    }
    table th {
        border-width: 2px;
        padding: 8px;
        border-style: solid;
        border-color: transparent;
        color:white;
        background-color:teal;
         font-family: sans-serif
   }
    textarea {
        border-width: 2px;
        padding: 8px;
        border-style: solid;
        border-color: transparent;
        color:chartreuse;
        background-color:black;
        font-family: 'Courier', sans-serif
    }
    h1 {
        margin: 1em 0 0.5em 0;
        color: #343434;
        font-weight: normal;
        font-family: 'Ultra', sans-serif;   
        font-size: 36px;
        line-height: 42px;
        text-transform: uppercase;
    }
    h2 {
        margin: 1em 0 0.5em 0;
        color: #343434;
        font-weight: normal;
        font-size: 30px;
        line-height: 40px;
        font-family: 'Orienta', sans-serif;
    }    
    h3 {
        margin: 1em 0 0.5em 0;
        color: #343434;
        font-weight: normal;
        font-size:24px;
        line-height: 30px;
        font-family: 'Orienta', sans-serif;
    }    
    </style>
    <!--
    //////////////////////////////////////////////////////////////////////////
    // All HTML dependencies that are in some sense standard and widely used, 
    // are loaded from content delivery networks.
    //////////////////////////////////////////////////////////////////////////  
    -->
    <script src="https://code.jquery.com/jquery-3.6.0.js" integrity="sha256-H+K7U5CnXl1h5ywQfKtSj8PCmoN9aaq30gDh27Xc0jk=" crossorigin="anonymous"></script>
</head>
<body style="background-color:CadetBlue">
    <h1>Message from Another Planet, version 4</h1>
    <h3>Adapted for Csound with HTML5 by Michael Gogins, from "Message from Another Planet" by Jacob Joaquin</h3>
    <form id='persist'>
        <table>
            <col width="2*">
            <col width="5*">
            <col width="100px">
            <tr>
            <td>
            <label for=gk2_spread>Frequency spread factor</label>
            <td>
            <input class=persistent-element type=range min=0 max=4 value=1 id=gk2_spread step=.001>
            <td>
            <output for=gk2_spread id=gk2_spread_output>1</output>
            </tr>
            <tr>
            <td>
            <label for=gk2_bass_gain>Bass emphasis factor</label>
            <td>
            <input class=persistent-element type=range min=0.0001 max=1 value=.005 id=gk2_bass_gain step=.001>
            <td>
            <output for=gk2_bass_gain id=gk2_bass_gain_output>.005</output>
            </tr>
            <tr>
            <td>
            <label for=gk_reverb_delay>Reverb delay feedback</label>
            <td>
            <input class=persistent-element type=range min=0 max=1 value=.89 id=gk_reverb_delay step=.001>
            <td>
            <output for=gk_reverb_delay id=gk_reverb_delay_output>.89</output>
            </tr>
            <tr>
            <td>
            <label for=gk_reverb_hipass>Reverb highpass cutoff (Hz)</label>
            <td>
            <input class=persistent-element type=range min=0 max=20000 value=12000 id=gk_reverb_hipass step=.001>
            <td>
            <output for=gk_reverb_hipass id=gk_reverb_hipass_output>12000</output>
            </tr>
            <tr>
            <td>
            <label for=gk_master_level>Master output level (dB)</label>
            <td>
            <input class=persistent-element type=range min=-40 max=40 value=-6 id=gk_master_level step=.001>
            <td>
            <output for=gk_master_level id=gk_master_level_output>-6</output>
            </tr>
        </table>
        <p>
        <input type="button" id='save' value="Save" />
        <input type="button" id='restore' value="Restore" />
    </form>   
    <p>
    <textarea id='csound_diagnostics' cols=80 rows=25>
    </textarea>
    <script src="csound_jsonrpc_stub.js"></script>
    <script>
    $(document).ready(function() {
        //////////////////////////////////////////////////////////////////////
        // This is the JSON-RPC proxy of the instance of Csound that is 
        // performing this piece.
        //////////////////////////////////////////////////////////////////////
        let csound = new Csound(origin);
        message_callback_ = function(message) {
            let notifications_textarea = document.getElementById("csound_diagnostics");
            let existing_notifications = notifications_textarea.value;
            notifications_textarea.value = existing_notifications + message;
            notifications_textarea.scrollTop = notifications_textarea.scrollHeight;
        }; 
        csound.SetMessageCallback(message_callback_, true);
        $('input').on('input', async function(event) {
            var slider_value = parseFloat(event.target.value);
            csound.SetControlChannel(event.target.id, slider_value);
            var output_selector = '#' + event.target.id + '_output';
            $(output_selector).val(slider_value);
            csound.Message(event.target.id + " = " + event.target.value + "\\n");
        });
        $('#save').on('click', async function() {
            $('.persistent-element').each(function() {
                localStorage.setItem(this.id, this.value);
            });
        });
        $('#restore').on('click', async function() {
            $('.persistent-element').each(function() {
                this.value = localStorage.getItem(this.id);
                csound.SetControlChannel(this.id, parseFloat(this.value));
                var output_selector = '#' + this.id + '_output';
                $(output_selector).val(this.value);
            });
        });
    });
</script>
</body>
</html>
}}

; For Linux, uncomment and if necessary edit this line:
; gi_webserver webserver_create "/home/mkg//csound-webserver-opcodes/examples/", 8080, 0

; For macOS, uncomment and if necessary edit this line:
gi_webserver webserver_create "/Users/michaelgogins/csound-webserver-opcodes/examples/", 8080, 0

webserver_open_html gi_webserver, gS_html

</CsInstruments>
<CsScore>
; p1   p2   p3     p4      p5      p6         p7
f 1    0    65536  10      1
f 100  0    256    -7      0       16         1    240    0
t 0    60
i 1    0    256    3000    6.00    1          1.24    
i 1    0    .      .       .       1.0666     1.23    
i 1    0    .      .       .       1.125      1.22    
i 1    0    .      .       .       1.14285    1.21    
i 1    0    .      .       .       1.23076    1.20    
i 1    0    .      .       .       1.28571    1.19    
i 1    0    .      .       .       1.333      1.18    
i 1    0    .      .       .       1.4545     1.17    
i 1    0    .      .       .       1.5        1.16    
i 1    0    .      .       .       1.6        1.15    
i 1    0    .      .       .       1.777      1.14    
i 1    0    .      .       .       1.8        1.13    
i 1    0    .      .       .       2          1.12    
i 1    0    .      .       .       2.25       1.10    
i 1    0    .      .       .       2.28571    1.09    
i 1    0    .      .       .       2.666      1.08    
i 1    0    .      .       .       3          1.07    
i 1    0    .      .       .       3.2        1.06    
i 1    0    .      .       .       4          1.05    
i 1    0    .      .       .       4.5        1.04    
i 1    0    .      .       .       5.333      1.03    
i 1    0    .      .       .       8          1.02    
i 1    0    .      1000    .       9          1.01    
i 1    0    .      500     .       16         1.00    

</CsScore>
</CsoundSynthesizer>

