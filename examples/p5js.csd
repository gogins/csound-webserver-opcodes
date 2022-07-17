<CsoundSynthesizer>
<CsLicense>
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
    <title>1-Dimensional Cellular Automata</title>
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
    <script src="https://cdnjs.cloudflare.com/ajax/libs/p5.js/1.4.1/p5.js" integrity="sha512-4P0ZJ49OMXe3jXT+EsEaq82eNwFeyYL81LeHGzGcEhowFbTqeQ80q+NEkgsE8tHPs6aCqvi7U+XWliAjDmT5Lg==" crossorigin="anonymous" referrerpolicy="no-referrer"></script>
</head>
<body style="background-color:CadetBlue">
    <h1>1-Dimesional Cellular Automata</h1>
    <h3>Adapted for Csound with HTML5 by Michael Gogins, from <a href="https://p5js.org/examples/simulate-wolfram-ca.html">p5.js Wolfram CA example</a><h3>
    <form id='persist'>
        <table>
            <col width="2*">
            <col width="5*">
            <col width="100px">
            <tr>
            <td>
            <label for=gk_rule>Rule</label>
            <td>
            <input class=persistent-element type=range min=0 max=255 value=110 id=gk_rule step=1>
            <td>
            <output for=gk_rule id=gk_rule_output>110</output>
            </tr>
            <tr>
            <td>
            <label for=gk_Pinitial_state>Initial state</label>
            <td>
            <input class=persistent-element type=range min=0 max=4 value=1 id=gk_initial_state step=.001>
            <td>
            <output for=gk_initial_state id=gk_initial_state_output>1</output>
            </tr>
            <tr>
            <td>
            <label for=gk_seconds_per_time_step>Seconds per time step</label>
            <td>
            <input class=persistent-element type=range min=0.01 max=4 value=.1 id=gk_seconds_per_time_step step=.01>
            <td>
            <output for=gk_seconds_per_time_step id=gk_seconds_per_time_step_output>.005</output>
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
        <input type="button" id='start' value="Start" />
        <input type="button" id='stop' value="Stop" />
        <input type="button" id='save_controls' value="Save" />
        <input type="button" id='restore_controls' value="Restore" />
    </form>   
    <p>
    <script src="csound_jsonrpc_stub.js"></script>
    <script>

    function decimal_to_ruleset(decimal, size) {
        const rule_set = [0];
        for (let i=0; i<size; i++) {
            let mask = 1;
            const bit = decimal & (mask << i);
            if (bit === 0) {
                rule_set[i] = 0;
            } else {
                rule_set[i] = 1;
            }
        }
        return rule_set;
    }
     
    continue_generating_score = false;
    let w = 1;
    // An array of 0s and 1s
    let cells;

    let generation = 0;

    //let ruleset = [0,1,0,1,1,0,1,0];
    //let ruleset = [0,1,1,0,1,1,0,1];
    // Rule 110 -- proved to be Turing complete.
    //let ruleset =   [0,1,1,0,1,1,1,0];
    //let ruleset = decimal_to_ruleset(110, 8);
    // Rule 54 -- possibly Turing complete.
    //let ruleset =   [0,0,1,1,0,1,1,0];

    function setup() {
        createCanvas(1000, 300);
        cells = Array(floor(width / w));
        for (let i = 0; i < cells.length; i++) {
            cells[i] = 0;
        }
        //cells[cells.length-1] = 1;
        cells[cells.length-100] = 1;
    }

    function draw_() {
        if (continue_generating_score === false) {
            return;
        }
        if (generation >= height / w) {
            generation = 0;
            clear();
        }
        for (let i = 0; i < cells.length; i++) {
            if (cells[i] === 1) {
                fill(200);
            } else {
                fill(51);
                noStroke();
                rect(i * w, generation * w, w, w);
            }
        }
        generate();
        let seconds_per_time_step = $('#gk_seconds_per_time_step').val()
        setTimeout(draw_, seconds_per_time_step * 1000);
    }

    function generate() {
        // First we create an empty array for the new values
        let nextgen = Array(cells.length);
        // For every spot, determine new state by examing current state, and neighbor states
        // Ignore edges that only have one neighor (i.e. do not wrap).
        for (let i = 1; i < cells.length-1; i++) {
            let left   = cells[i-1];   // Left neighbor state
            let me     = cells[i];     // Current state
            let right  = cells[i+1];   // Right neighbor state
            nextgen[i] = rules(left, me, right); // Compute next generation state based on ruleset
        }
        cells = nextgen;
        generation++;
    }

    function rules(a, b, c) {
        if (a == 1 && b == 1 && c == 1) return ruleset[7];
        if (a == 1 && b == 1 && c == 0) return ruleset[6];
        if (a == 1 && b == 0 && c == 1) return ruleset[5];
        if (a == 1 && b == 0 && c == 0) return ruleset[4];
        if (a == 0 && b == 1 && c == 1) return ruleset[3];
        if (a == 0 && b == 1 && c == 0) return ruleset[2];
        if (a == 0 && b == 0 && c == 1) return ruleset[1];
        if (a == 0 && b == 0 && c == 0) return ruleset[0];
        return 0;
    }
    
    function play_cells() {
    }
    
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
            if (event.target.id == 'gk_rule') {
                ruleset = decimal_to_ruleset(slider_value, 8);
                csound.Message(`rule_number ${rule_number} = binary rule ${ruleset}\n`);
            }
       });
        $('#start').on('click', async function() {
            rule_number = $('#gk_rule').val();
            ruleset = decimal_to_ruleset(rule_number, 8);
            csound.Message(`rule_number ${rule_number} = binary rule ${ruleset}\n`);
            continue_generating_score = true;
            let seconds_per_time_step = $('#gk_seconds_per_time_step').val()
            setTimeout(draw_, seconds_per_time_step * 1000);
        });
        $('#stop').on('click', async function() {
            continue_generating_score = false;
            noCanvas();
            generation = 0;
            setup();
        });
        $('#save_controls').on('click', async function() {
            $('.persistent-element').each(function() {
                localStorage.setItem(this.id, this.value);
            });
        });
        $('#restore_controls').on('click', async function() {
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

