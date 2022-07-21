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
0dbfs           =           100

//////////////////////////////////////////////
// Original by Steven Yi.
// Adapted by Michael Gogins.
//////////////////////////////////////////////
gk_FMWaterBell_level chnexport "gk_FMWaterBell_level", 3 ; 0
gi_FMWaterBell_attack chnexport "gi_FMWaterBell_attack", 3 ; 0.002
gi_FMWaterBell_release chnexport "gi_FMWaterBell_release", 3 ; 0.01
gi_FMWaterBell_sustain chnexport "gi_FMWaterBell_sustain", 3 ; 20
gi_FMWaterBell_sustain_level chnexport "gi_FMWaterBell_sustain_level", 3 ; .1
gk_FMWaterBell_index chnexport "gk_FMWaterBell_index", 3 ; .5
gk_FMWaterBell_crossfade chnexport "gk_FMWaterBell_crossfade", 3 ; .5
gk_FMWaterBell_vibrato_depth chnexport "gk_FMWaterBell_vibrato_depth", 3 ; 0.05
gk_FMWaterBell_vibrato_rate chnexport "gk_FMWaterBell_vibrato_rate", 3 ; 6
gk_FMWaterBell_midi_dynamic_range chnexport "gk_FMWaterBell_midi_dynamic_range", 3 ; 20

gk_FMWaterBell_level init 0
gi_FMWaterBell_attack init 0.002
gi_FMWaterBell_release init 0.01
gi_FMWaterBell_sustain init 20
gi_FMWaterBell_sustain_level init .1
gk_FMWaterBell_index init 5
gk_FMWaterBell_crossfade init .5
gk_FMWaterBell_vibrato_depth init 0.05
gk_FMWaterBell_vibrato_rate init 6
gk_FMWaterBell_midi_dynamic_range init 20

gi_FMWaterBell_cosine ftgen 0, 0, 65537, 11, 1

instr FMWaterBell
i_instrument = p1
i_time = p2
i_duration = p3
; One of the envelopes in this instrument should be releasing, and use this:
i_sustain = 1000
xtratim gi_FMWaterBell_attack + gi_FMWaterBell_release
i_midi_key = p4
i_midi_dynamic_range = i(gk_FMWaterBell_midi_dynamic_range)
i_midi_velocity = p5 * i_midi_dynamic_range / 127 + (63.6 - i_midi_dynamic_range / 2)
k_space_front_to_back = p6
k_space_left_to_right = p7
k_space_bottom_to_top = p8
i_phase = p9
i_frequency = cpsmidinn(i_midi_key)
; Adjust the following value until "overall amps" at the end of performance is about -6 dB.
i_level_correction = 75
i_normalization = ampdb(-i_level_correction) / 2
i_amplitude = ampdb(i_midi_velocity) * i_normalization * 1.6
k_gain = ampdb(gk_FMWaterBell_level)
a_signal fmbell	1, i_frequency, gk_FMWaterBell_index, gk_FMWaterBell_crossfade, gk_FMWaterBell_vibrato_depth, gk_FMWaterBell_vibrato_rate, gi_FMWaterBell_cosine, gi_FMWaterBell_cosine, gi_FMWaterBell_cosine, gi_FMWaterBell_cosine, gi_FMWaterBell_cosine ;, gi_FMWaterBell_sustain
;a_envelope linsegr 0, gi_FMWaterBell_attack, 1, i_sustain, gi_FMWaterBell_sustain_level, gi_FMWaterBell_release, 0
a_envelope linsegr 0, gi_FMWaterBell_attack, 1, i_sustain, 1, gi_FMWaterBell_release, 0
; ares transegr ia, idur, itype, ib [, idur2] [, itype] [, ic] ...
;;a_envelope transegr 0, gi_FMWaterBell_attack, 12, 1, i_sustain, 12, gi_FMWaterBell_sustain_level, gi_FMWaterBell_release, 12, 0
a_signal = a_signal * i_amplitude * a_envelope * k_gain

#ifdef USE_SPATIALIZATION
a_spatial_reverb_send init 0
a_bsignal[] init 16
a_bsignal, a_spatial_reverb_send Spatialize a_signal, k_space_front_to_back, k_space_left_to_right, k_space_bottom_to_top
outletv "outbformat", a_bsignal
outleta "out", a_spatial_reverb_send
#else
a_out_left, a_out_right pan2 a_signal, k_space_left_to_right
outleta "outleft", a_out_left
outleta "outright", a_out_right
#endif
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

gk_ReverbSC_feedback chnexport "gk_ReverbSC_feedback", 3
gk_ReverbSC_wet chnexport "gk_ReverbSC_wet", 3
gi_ReverbSC_delay_modulation chnexport "gi_ReverbSC_delay_modulation", 3
gk_ReverbSC_frequency_cutoff chnexport "gk_ReverbSC_frequency_cutoff", 3

gk_ReverbSC_feedback init 0.875
gk_ReverbSC_wet init 0.5
gi_ReverbSC_delay_modulation init 0.0075
gk_ReverbSC_frequency_cutoff init 15000

instr ReverbSC
gk_ReverbSC_dry = 1.0 - gk_ReverbSC_wet
aleftin init 0
arightin init 0
aleftout init 0
arightout init 0
aleftin inleta "inleft"
arightin inleta "inright"
aleftout, arightout reverbsc aleftin, arightin, gk_ReverbSC_feedback, gk_ReverbSC_frequency_cutoff, sr, gi_ReverbSC_delay_modulation
aleftoutmix = aleftin * gk_ReverbSC_dry + aleftout * gk_ReverbSC_wet
arightoutmix = arightin * gk_ReverbSC_dry + arightout * gk_ReverbSC_wet
outleta "outleft", aleftoutmix
outleta "outright", arightoutmix
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

gk_MasterOutput_level chnexport "gk_MasterOutput_level", 3 ; 0
gS_MasterOutput_filename chnexport "gS_MasterOutput_filename", 3 ; ""

gk_MasterOutput_level init 0
gS_MasterOutput_filename init ""

instr MasterOutput
aleft inleta "inleft"
aright inleta "inright"
k_gain = ampdb(gk_MasterOutput_level)
printks2 "Master gain: %f\n", k_gain
iamp init 1
aleft butterlp aleft, 18000
aright butterlp aright, 18000
outs aleft * k_gain, aright * k_gain
; We want something that will play on my phone.
i_amplitude_adjustment = ampdbfs(-3) / 32767
i_filename_length strlen gS_MasterOutput_filename
if i_filename_length > 0 then
prints sprintf("Output filename: %s\n", gS_MasterOutput_filename)
fout gS_MasterOutput_filename, 18, aleft * i_amplitude_adjustment, aright * i_amplitude_adjustment
endif
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

instr Exit
prints "Exit"
endin

connect "FMWaterBell", "outleft", "ReverbSC", "inleft"
connect "FMWaterBell", "outright", "ReverbSC", "inright"
connect "ReverbSC", "outleft", "MasterOutput", "inleft"
connect "ReverbSC", "outright", "MasterOutput", "inright"

alwayson "ReverbSC"
alwayson "MasterOutput"

//////////////////////////////////////////////////////////////////////////
// This is all of the HTML5 code for the embedded Web page that controls 
// this piece.
//////////////////////////////////////////////////////////////////////////  
gS_html init {{
<!DOCTYPE html>
<html>
<head>
    <title>Penrose Tiling</title>
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
        height: 12px;
        width: 12px;
        border-radius: 50%;
        box-shadow: inset 0 0 5px #234;
        background: chartreuse;
        margin-top: -4px;
        border-radius: 8px;
    }
    table td {
        padding: 2px;
        border-style: solid;
        border-color: transparent;
        color:yellow;
        background-color: teal;
        font-family: sans-serif
    }
    table th {
        border-width: 2px;
        padding: 2px;
        border-style: solid;
        border-color: transparent;
        color:white;
        background-color:teal;
        font-family: sans-serif
   }
    textarea {
        color:chartreuse;
        background-color:black;
        font-family: 'Courier', sans-serif
        border: none;
        width: 100%;
        height: 100%;
        -webkit-box-sizing: border-box; /* <=iOS4, <= Android  2.3 */
        -moz-box-sizing: border-box; /* FF1+ */
        box-sizing: border-box; /* Chrome, IE8, Opera, Safari 5.1*/
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
    canvas {
        overflow: true;
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
    <script src="https://cdnjs.cloudflare.com/ajax/libs/sprintf/1.1.2/sprintf.js" integrity="sha512-dY9NsJoe4eisOR4ZtU0WaFNOxGcGZMfaviwSYHoiiEXvC6QLBsOOVsv3uY+5lEvuRtGTATg7usKQGajlDWSo7Q==" crossorigin="anonymous" referrerpolicy="no-referrer"></script>
</head>
<body style="background-color:CadetBlue">
    <h1>Penrose Tiling</h1>
    <h3>Adapted for Csound with HTML5 by Michael Gogins, from <a href="https://p5js.org/examples/simulate-penrose-tiles.html">p5.js Penrose tiling example</a><h3>
    <form id='persist'>
        <table style="font-size:12pt;">
            <col width="2*">
            <col width="5*">
            <col width="2*">
            <col width="12*">
            <tr>
            <td>
            <label for=gk_FMWaterBell_level>Water bell level</label>
            <td>
            <input class=persistent-element type=range min=-40 max=40 value=0 id=gk_FMWaterBell_level step=1>
            <td>
            <output for=gk_FMWaterBell_level id=gk_FMWaterBell_level_output>0</output>
            <td rowspan=6>
                <textarea id="csound_diagnostics" cols=128 rows=14>
                </textarea>
            </td>
            </tr>
            <tr>
            <td>
            <label for=gk_frame_rate>Frame rate</label>
            <td>
            <input class=persistent-element type=range min=.5 max=8 value=1 id=gk_frame_rate step=.1>
            <td>
            <output for=gk_frame_rate id=gk_frame_rate_output>1</output>
            </tr>
            <tr>
            <td>
            <label for=gk_ReverbSC_feedback>Reverb delay feedback</label>
            <td>
            <input class=persistent-element type=range min=0 max=1 value=.89 id=gk_ReverbSC_feedback step=.001>
            <td>
            <output for=gk_ReverbSC_feedback id=gk_ReverbSC_feedback_output>.89</output>
            </tr>
            <tr>
            <td>
            <label for=gk_ReverbSC_frequency_cutoff>Reverb highpass cutoff (Hz)</label>
            <td>
            <input class=persistent-element type=range min=0 max=20000 value=12000 id=gk_ReverbSC_frequency_cutoff step=.001>
            <td>
            <output for=gk_ReverbSC_frequency_cutoff id=gk_ReverbSC_frequency_cutoff>12000</output>
            </tr>
            <tr>
            <td>
            <label for=gk_MasterOutput_level>Master output level (dB)</label>
            <td>
            <input class=persistent-element type=range min=-40 max=40 value=-6 id=gk_MasterOutput_level step=.001>
            <td>
            <output for=gk_MasterOutput_level id=gk_MasterOutput_level_output>-6</output>
            </tr>
            <tr>
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

    let ds;
    var gk_frame_rate = 1.;
    var do_render = false;

    function setup() {
        createCanvas(710, 400);
        ds = new PenroseLSystem();
        //please, play around with the following line
        ds.simulate(5);
    }

    function draw() {
        if (do_render === true) {
            background(0);
            frameRate(gk_frame_rate);
            ds.render();
        }
    }

    function PenroseLSystem() {
        this.steps = 0;
        this.axiom = "[X]++[X]++[X]++[X]++[X]";
        this.ruleW = "YF++ZF----XF[-YF----WF]++";
        this.ruleX = "+YF--ZF[---WF--XF]+";
        this.ruleY = "-WF++XF[+++YF++ZF]-";
        this.ruleZ = "--YF++++WF[+ZF++++XF]--XF";
        this.startLength = 500;//460.0;
        this.theta = TWO_PI / 10.0; //10 = 36 degrees, try TWO_PI / 6.0, ...
        this.reset();
    }

    PenroseLSystem.prototype.simulate = function (gen) {
        while (this.getAge() < gen) {
            this.iterate(this.production);
        }
    }

PenroseLSystem.prototype.reset = function () {
        this.production = this.axiom;
        this.drawLength = this.startLength;
        this.generations = 0;
    }

    PenroseLSystem.prototype.getAge = function () {
        return this.generations;
    }

    PenroseLSystem.prototype.iterate = function() {
        let newProduction = "";
        for(let i=0; i < this.production.length; ++i) {
            let step = this.production.charAt(i);
            // If the current character is 'W', replace the current character
            //by the corresponding rule.
            if (step == 'W') {
                newProduction = newProduction + this.ruleW;
            }
            else if (step == 'X') {
                newProduction = newProduction + this.ruleX;
            }
            else if (step == 'Y') {
                newProduction = newProduction + this.ruleY;
            }
            else if (step == 'Z') {
                newProduction = newProduction + this.ruleZ;
            }
            else {
                // Drop all 'F' characters, don't touch other
                //c haracters (i.e. '+', '-', '[', ']'
                if (step != 'F') {
                  newProduction = newProduction + step;
                }
            }
        }
        this.drawLength = this.drawLength * 0.5;
        this.generations++;
        this.production = newProduction;
    }

    /**
     * Render the production string to a turtle drawing and also to real-time 
     * Csound notes, one time step per animation frame. We draw 1 'F' for each 
     * animation frame.
     */
    PenroseLSystem.prototype.render = function () {
        // Move the origin from the upper right hand corner of the canvas to the 
        // center.
        translate(width / 2, height / 2);
        this.steps += 20;
        if (this.steps > this.production.length) {
            this.steps = this.production.length;
        }
        // The production is divided into chunks that become strokes.
        for(let i=0; i<this.steps; ++i) {
            let step = this.production.charAt(i);
            //'W', 'X', 'Y', 'Z' symbols don't actually correspond to a turtle action
            if( step == 'F') {
                // Gray, alpha.
                stroke(255, 60);
                let score = new Array();
                // x0, y0, x2, y2
                // It's always move by -y2 in the direction of theta.
                line(0, 0, 0, -this.drawLength);
                // Draw lines only.
                noFill();
                // Move the graphics context to the end of the just-drawn line.
                translate(0, -this.drawLength);
                matrix = drawingContext.getTransform();
                let x = matrix.e;
                let y = matrix.f;
                let position = sprintf("x: %9.4f y: %9.4f\\n", x, y);
                // Draw a green dot for each note.
                fill("green");
                circle(0, 0, 4);
                // The commented out code below is how I figured out that, 
                // at least in this case, the translation elements of the 
                // current transformation are the x, y coordinates of the 
                // current point under the _original_ transformation.
                //~ push()
                //~ drawingContext.resetTransform();
                //~ fill("red");
                //~ circle(x, y, 4);
                //~ pop()
                // Rescale by some reasonable amount.
                // Note that y origin is at top, this must be reversed.
                let midi_key = Math.round(128. - (y / 5.));
                let midi_velocity = x / 10.;
                let pan = (x / 800) - .5;
                let note = sprintf("i 1 0 2 %9.4f %9.4f 0 %9.4f", midi_key, midi_velocity, pan);
                csound.ReadScore(note);
            }
            else if (step == '+') {
                rotate(this.theta);
            }
            else if (step == '-') {
                rotate(-this.theta);
            }
            else if (step == '[') {
                push();
            }
            else if (step == ']') {
                pop();
                }
            }
        }
    
    $(document).ready(function() {
        //////////////////////////////////////////////////////////////////////
        // This is the JSON-RPC proxy of the instance of Csound that is 
        // performing this piece.
        //////////////////////////////////////////////////////////////////////
        csound = new Csound(origin);
        message_callback_ = async function(message) {
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
            if (event.target.id == "gk_frame_rate") {
                gk_frame_rate = slider_value;
            }
         });
        $('#start').on('click', async function() {
            do_render = true;
        });
        $('#stop').on('click', async function() {
            do_render = false;
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
i "Exit" 360 1
</CsScore>
</CsoundSynthesizer>

