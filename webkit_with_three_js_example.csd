<CsoundSyntheizer>
<CsLicense>

Michael Gogins, 2021

This piece demonstrates the use of the Faust opcodes, the Clang opcodes, the 
vst4cs opcodes, the signal flow graph opcodes, and the WebKit opcodes, all in 
one .csd file.

Comments are provided throughout the piece. 

External dependencies are brought into the .csd file -- as far as that is 
possible. The remaining external dependencies are the WebKitGTK+ system 
packages, the Clang/LLVM system packages, the Eigen header-only library 
for linear algrebra, the Faust system packages and Faust opcodes for Csound,
the vst4cs opcodes for Csound, and the Pianoteq VST plugin from Modartt.

It's a lot, but it makes for a very powerful computer music system.

</CsLicense>
<CsOptions>
-m0 -d -odac
</CsOptions>
<CsInstruments>
; Change to 96000 with 1 ksmps for final rendering.
sr = 48000
ksmps = 128
nchnls = 2
0dbfs = 5

; Ensure the same random stream for each rendering.
; rand, randh, randi, rnd(x) and birnd(x) are not affected by seed.

;seed 81814
;seed 818145
seed 88818145

connect "Blower", "outleft", "ReverbSC", "inleft"
connect "Blower", "outright", "ReverbSC", "inright"
connect "Bower", "outleft", "ReverbSC", "inleft"
connect "Bower", "outright", "ReverbSC", "inright"
connect "Buzzer", "outleft", "ReverbSC", "inleft"
connect "Buzzer", "outright", "ReverbSC", "inright"
connect "Droner", "outleft", "ReverbSC", "inleft"
connect "Droner", "outright", "ReverbSC", "inright"
connect "FMWaterBell", "outleft", "ReverbSC", "inleft"
connect "FMWaterBell", "outright", "ReverbSC", "inright"
connect "Phaser", "outleft", "ReverbSC", "inleft"
connect "Phaser", "outright", "ReverbSC", "inright"
connect "PianoOutPianoteq", "outleft", "MasterOutput", "inleft"
connect "PianoOutPianoteq", "outright", "MasterOutput", "inright"
connect "Sweeper", "outleft", "ReverbSC", "inleft"
connect "Sweeper", "outright", "ReverbSC", "inright"
connect "Shiner", "outleft", "ReverbSC", "inleft"
connect "Shiner", "outright", "ReverbSC", "inright"
connect "ZakianFlute", "outleft", "ReverbSC", "inleft"
connect "ZakianFlute", "outright", "ReverbSC", "inright"
connect "ReverbSC", "outleft", "MVerb", "inleft"
connect "ReverbSC", "outright", "MVerb", "inright"
connect "MVerb", "outleft", "MasterOutput", "inleft"
connect "MVerb", "outright", "MasterOutput", "inright"

prealloc 1, 50
prealloc 2, 50
prealloc 3, 50
prealloc 4, 50
prealloc 5, 50
prealloc 6, 50
prealloc 7, 50
prealloc 8, 20
prealloc 9, 20

#include "FMWaterBell.inc"
#include "Phaser.inc"
#include "Droner.inc"
#include "Sweeper.inc"
#include "Buzzer.inc"
#include "Shiner.inc"
#include "Blower.inc"
#include "ZakianFlute.inc"
gi_Pianoteq vstinit "/home/mkg/Pianoteq\ 7/x86-64bit/Pianoteq\ 7.so", 0
#include "PianoNotePianoteq.inc"
#include "Bower.inc"
#include "PianoOutPianoteq.inc"
alwayson "PianoOutPianoteq"
#include "MVerb.inc"
alwayson "MVerb"
#include "ReverbSC.inc"
alwayson "ReverbSC"
#include "MasterOutput.inc"
alwayson "MasterOutput"

//////////////////////////////////////////////////////////////////////////////
// This instrument defines a WebKit browser that provides sliders for 
// real-time tweaking of instrument parameters, as well as a display of 
// Csound's diagnostic messages.
//
// Control values that are saved by clicking the "Save" button will
// automatically be restored on the next run of Csound.
instr Browser

// The following HTML5 code is pretty much the standard sort of thing for Web 
// pages.
//
// However, the <csound.js> script brings a proxy for the instance of Csound 
// that is performing into the JavaScript context of the Web page, so the 
// event handlers of the sliders on the page can call Csound to set control 
// channel values.

gS_html init {{<!DOCTYPE html>
<html>
<head>
    <title>Blue Leaves version 3</title>
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
        box-shadow: inset 0 0 7px #234;
        background: chartreuse;
        margin-top: -4px;
        border-radius: 10px;
    }
    table table td {
        border-width: 2px;
        padding: 8px;
        border-style: solid;
        border-color: transparent;
        color:yellow;
        background-color: teal;
        font-family: sans-serif;
        text-align:right;
    }
    table table th {
        border-width: 2px;
        padding: 8px;
        border-style: solid;
        border-color: transparent;
        color:white;
        background-color:teal;
        font-family: sans-serif;
    }
    textarea {
        border-width: 2px;
        padding: 6px;
        border-style: solid;
        border-color: transparent;
        color:chartreuse;
        background-color:black;
        font-size:10pt;
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
    <script src='silencio/js/sprintf.js'></script>
    <script src='silencio/js/numeric.js'></script>
    <script src="silencio/js/three.js"></script>
    <script src='silencio/js/tinycolor.js'></script>
    <script src='silencio/js/Silencio.js'></script>
    <script src="silencio/js/TrackballControls.js"></script>
    <script src='silencio/js/ChordSpace.js'></script>
    <script src="silencio/js/dat.gui.js"></script>    
    <script src="silencio/js/PianoRoll3D.js"></script>    
</head>
<body style="background-color:black;box-sizing:border-box;padding:10px;:fullscreen">
    <script src="https://code.jquery.com/jquery-3.6.0.js" integrity="sha256-H+K7U5CnXl1h5ywQfKtSj8PCmoN9aaq30gDh27Xc0jk=" crossorigin="anonymous"></script>
    <script>
        /**
         * This file is generated by jsonrpcstub, DO NOT CHANGE IT MANUALLY!
         */
        function Csound(url) {
            this.url = url;
            var id = 1;
            
            function doJsonRpcRequest(method, params, methodCall, callback_success, callback_error) {
                var request = {};
                if (methodCall)
                    request.id = id++;
                request.jsonrpc = "2.0";
                request.method = method;
                if (params !== null) {
                    request.params = params;
                }
                JSON.stringify(request);
                $.ajax({
                    type: "POST",
                    url: url,
                    data: JSON.stringify(request),
                    success: function (response) {
                        if (methodCall) {
                            if (response.hasOwnProperty("result") && response.hasOwnProperty("id")) {
                                callback_success(response.id, response.result);
                            } else if (response.hasOwnProperty("error")) {
                                if (callback_error != null)
                                    callback_error(response.error.code,response.error.message);
                            } else {
                                if (callback_error != null)
                                    callback_error(-32001, "Invalid Server response: " + response);
                            }
                        }
                    },
                    error: function () {
                        if (methodCall)
                            callback_error(-32002, "AJAX Error");
                    },
                    dataType: "json"
                });
                return id-1;
            }
            this.doRPC = function(method, params, methodCall, callback_success, callback_error) {
                return doJsonRpcRequest(method, params, methodCall, callback_success, callback_error);
            }
        }

        Csound.prototype.CompileCsdText = function(csd_text, callbackSuccess, callbackError) {
            var params = {csd_text : csd_text};
            return this.doRPC("CompileCsdText", params, true, callbackSuccess, callbackError);
        };
        Csound.prototype.CompileOrc = function(orc_code, callbackSuccess, callbackError) {
            var params = {orc_code : orc_code};
            return this.doRPC("CompileOrc", params, true, callbackSuccess, callbackError);
        };
        Csound.prototype.EvalCode = function(orc_code, callbackSuccess, callbackError) {
            var params = {orc_code : orc_code};
            return this.doRPC("EvalCode", params, true, callbackSuccess, callbackError);
        };
        Csound.prototype.Get0dBFS = function(callbackSuccess, callbackError) {
            var params = null;
            return this.doRPC("Get0dBFS", params, true, callbackSuccess, callbackError);
        };
        Csound.prototype.GetAudioChannel = function(channel_name, callbackSuccess, callbackError) {
            var params = {channel_name : channel_name};
            return this.doRPC("GetAudioChannel", params, true, callbackSuccess, callbackError);
        };
        Csound.prototype.GetControlChannel = function(channel_name, callbackSuccess, callbackError) {
            var params = {channel_name : channel_name};
            return this.doRPC("GetControlChannel", params, true, callbackSuccess, callbackError);
        };
        Csound.prototype.GetDebug = function(callbackSuccess, callbackError) {
            var params = null;
            return this.doRPC("GetDebug", params, true, callbackSuccess, callbackError);
        };
        Csound.prototype.GetKsmps = function(callbackSuccess, callbackError) {
            var params = null;
            return this.doRPC("GetKsmps", params, true, callbackSuccess, callbackError);
        };
        Csound.prototype.GetNchnls = function(callbackSuccess, callbackError) {
            var params = null;
            return this.doRPC("GetNchnls", params, true, callbackSuccess, callbackError);
        };
        Csound.prototype.GetNchnlsInput = function(callbackSuccess, callbackError) {
            var params = null;
            return this.doRPC("GetNchnlsInput", params, true, callbackSuccess, callbackError);
        };
        Csound.prototype.GetScoreOffsetSeconds = function(callbackSuccess, callbackError) {
            var params = null;
            return this.doRPC("GetScoreOffsetSeconds", params, true, callbackSuccess, callbackError);
        };
        Csound.prototype.GetScoreTime = function(callbackSuccess, callbackError) {
            var params = null;
            return this.doRPC("GetScoreTime", params, true, callbackSuccess, callbackError);
        };
        Csound.prototype.GetSr = function(callbackSuccess, callbackError) {
            var params = null;
            return this.doRPC("GetSr", params, true, callbackSuccess, callbackError);
        };
        Csound.prototype.GetStringChannel = function(channel_name, callbackSuccess, callbackError) {
            var params = {channel_name : channel_name};
            return this.doRPC("GetStringChannel", params, true, callbackSuccess, callbackError);
        };
        Csound.prototype.InputMessage = function(sco_code, callbackSuccess, callbackError) {
            var params = {sco_code : sco_code};
            return this.doRPC("InputMessage", params, true, callbackSuccess, callbackError);
        };
        Csound.prototype.IsScorePending = function(callbackSuccess, callbackError) {
            var params = null;
            return this.doRPC("IsScorePending", params, true, callbackSuccess, callbackError);
        };
        Csound.prototype.Message = function(message, callbackSuccess, callbackError) {
            var params = {message : message};
            this.doRPC("Message", params, false, callbackSuccess, callbackError);
        };
        Csound.prototype.ReadScore = function(sco_code, callbackSuccess, callbackError) {
            var params = {sco_code : sco_code};
            return this.doRPC("ReadScore", params, true, callbackSuccess, callbackError);
        };
        Csound.prototype.RewindScore = function(callbackSuccess, callbackError) {
            var params = null;
            return this.doRPC("RewindScore", params, true, callbackSuccess, callbackError);
        };
        Csound.prototype.ScoreEvent = function(opcode_code, pfields, callbackSuccess, callbackError) {
            var params = {opcode_code : opcode_code, pfields : pfields};
            return this.doRPC("ScoreEvent", params, true, callbackSuccess, callbackError);
        };
        Csound.prototype.SetControlChannel = function(channel_name, channel_value, callbackSuccess, callbackError) {
            var params = {channel_name : channel_name, channel_value : channel_value};
            return this.doRPC("SetControlChannel", params, true, callbackSuccess, callbackError);
        };
        Csound.prototype.SetDebug = function(enabled, callbackSuccess, callbackError) {
            var params = {enabled : enabled};
            return this.doRPC("SetDebug", params, true, callbackSuccess, callbackError);
        };
        Csound.prototype.SetMessageCallback = function(callback, callbackSuccess, callbackError) {
            var params = {callback : callback};
            return this.doRPC("SetMessageCallback", params, true, callbackSuccess, callbackError);
        };
        Csound.prototype.SetScoreOffsetSeconds = function(score_time, callbackSuccess, callbackError) {
            var params = {score_time : score_time};
            return this.doRPC("SetScoreOffsetSeconds", params, true, callbackSuccess, callbackError);
        };
        Csound.prototype.SetScorePending = function(pending, callbackSuccess, callbackError) {
            var params = {pending : pending};
            return this.doRPC("SetScorePending", params, true, callbackSuccess, callbackError);
        };
        Csound.prototype.SetStringChannel = function(channel_name, channel_value, callbackSuccess, callbackError) {
            var params = {channel_name : channel_name, channel_value : channel_value};
            return this.doRPC("SetStringChannel", params, true, callbackSuccess, callbackError);
        };
        Csound.prototype.TableLength = function(table_number, callbackSuccess, callbackError) {
            var params = {table_number : table_number};
            return this.doRPC("TableLength", params, true, callbackSuccess, callbackError);
        };
        Csound.prototype.TableGet = function(index, table_number, callbackSuccess, callbackError) {
            var params = {index : index, table_number : table_number};
            return this.doRPC("TableGet", params, true, callbackSuccess, callbackError);
        };
        Csound.prototype.TableSet = function(index, table_number, value, callbackSuccess, callbackError) {
            var params = {index : index, table_number : table_number, value : value};
            return this.doRPC("TableSet", params, true, callbackSuccess, callbackError);
        };
    </script>
    <canvas id="canvas" style="block;height:100vh;width:100vw">
    </canvas>
    <script>
        csound = new Csound("http://localhost:8383");
        var canvas = document.getElementById("canvas");
        var piano_roll = new PianoRoll.PianoRoll3D(canvas);
        
    function animate() {
        requestAnimationFrame(animate)
        // trackball controls needs to be updated in the animation loop before it will work
        piano_roll.controls.update()
        piano_roll.render3D()
        ///stats.update()
    }

    function render() {
        renderer.render(scene, camera)
    }
    
    var updateScoreTime = function() {
        var score_time; /// = csound.getScoreTime();
        setTimeout(updateScoreTime, 200);
        lsys.score.progress3D(score_time);
    };
        
    var stop = function() {
        Silencio.saveDatGuiJson(gui);
    }

    var parameters = {
        gk_FMWaterBell_level: 15,
        gi_FMWaterBell_attack: 0.002,
        gi_FMWaterBell_release: 0.01,
        gi_FMWaterBell_exponent: 15,
        gi_FMWaterBell_sustain: 20,
        gi_FMWaterBell_sustain_level: .1,
        gk_FMWaterBell_index: .5,
        gk_FMWaterBell_crossfade: .5,
        gk_FMWaterBell_vibrato_depth: 0.05,
        gk_FMWaterBell_vibrato_rate: 6,
        gk_Bower_level: 0.5,
        gk_Bower_pressure: 0.25,
        gk_Blower_grainDensity: .150,
        gk_Blower_grainDuration: .2,
        gk_Blower_grainAmplitudeRange: .1,
        gk_Blower_grainFrequencyRange: .033,
        gk_Blower_level: 0.5,
        gk_Buzzer_harmonics: 15,
        gk_Buzzer_level: 0.5,
        gk_Droner_partial1: 0.1,
        gk_Droner_partial2: 0.1,
        gk_Droner_partial3: 0.1,
        gk_Droner_partial4: 0.1,
        gk_Droner_partial5: 0.1,
        gk_Droner_level: 30,
        gk_Phaser_ratio1: 1,
        gk_Phaser_ratio2: .3333334,
        gk_Phaser_index1: 1,
        gk_Phaser_index2: .0125,
        gk_Phaser_level: 0.5,
        gk_PianoteqOut_level: 0.5,
        gk_Shiner_level: 0.5,
        gk_Sweeper_britel: 0,
        gk_Sweeper_briteh: 2.9,
        gk_Sweeper_britels: .2 / 3,
        gk_Sweeper_britehs: 2.5 / 2,
        gk_Sweeper_level: 0.5,
        gk_ZakianFlute_level: 0,
        gk_MVerb_feedback: .975,
        gk_ReverbSC_feedback: .8,
        gk_MasterOutput_level: .4,
        gk_overlap: 0.05,
        generate: window.generate,
        stop: window.stop,
    };
        
    var default_json = {
        "preset": "Default",
        "remembered": {
            "Default": {
                "0": {
                    "gk_overlap": 5.504950495049505,
                    "gk_MVerb_feedback": 0.975,
                    "gk_ReverbSC_feedback": 0.85,
                    "gk_MasterOutput_level": -10,
                    "gi_FMWaterBell_attack": 0.002936276551436901,
                    "gi_FMWaterBell_release": 0.022698875468554768,
                    "gi_FMWaterBell_exponent": 12.544773011245312,
                    "gi_FMWaterBell_sustain": 5.385256143273636,
                    "gi_FMWaterBell_sustain_level": 0.08267388588088297,
                    "gk_FMWaterBell_crossfade": 0.48250728862973763,
                    "gk_FMWaterBell_index": 1.1401499375260309,
                    "gk_FMWaterBell_vibrato_depth": 0.2897954989209742,
                    "gk_FMWaterBell_vibrato_rate": 4.762100503545371,
                    "gk_FMWaterBell_level": 35., 
                    "gk_Phaser_ratio1": 1,
                    "gk_Phaser_ratio2": 5./3.,
                    "gk_Phaser_index1": 1,
                    "gk_Phaser_index2": 2,
                    "gk_Phaser_level": 14,
                    "gk_Bower_pressure": 3.050919377652051,
                    "gk_Bower_level": -12.677504515770458,
                    "gk_Droner_partial1": 0.11032374600527997,
                    "gk_Droner_partial2": 0.4927052938724468,
                    "gk_Droner_partial3": 0.11921634014172572,
                    "gk_Droner_partial4": 0.06586077532305128,
                    "gk_Droner_partial5": 0.6616645824649159,
                    "gk_Droner_level": 10.,
                    "gk_Sweeper_britel": 0.2773844231570179,
                    "gk_Sweeper_briteh": 3.382178217821782,
                    "gk_Sweeper_britels": 0.19575671852899576,
                    "gk_Sweeper_britehs": 0.8837340876944837,
                    "gk_Sweeper_level": 10,
                    "gk_Buzzer_harmonics": 2.608203677510608,
                    "gk_Buzzer_level": 27.56,
                    "gk_Shiner_level": 16,
                    "gk_Blower_grainDensity": 79.99177885109444,
                    "gk_Blower_grainDuration": 0.2,
                    "gk_Blower_grainAmplitudeRange": 87.88408180043162,
                    "gk_Blower_grainFrequencyRange": 30.596081700708627,
                    "gk_Blower_level": 4,
                    "gk_ZakianFlute_level": 0,
                    "gk_PianoteqOut_level": 14.5
                }
            }
        },
            "closed": false,
            "folders": {
            "Master": {
            "preset": "Default",
            "closed": true,
            "folders": {}
            },
            "FMWaterBell": {
            "preset": "Default",
            "closed": false,
            "folders": {}
            },
            "Phaser": {
            "preset": "Default",
            "closed": true,
            "folders": {}
            },
            "Bower": {
            "preset": "Default",
            "closed": true,
            "folders": {}
            },
            "Droner": {
            "preset": "Default",
            "closed": false,
            "folders": {}
            },
            "Sweeper": {
            "preset": "Default",
            "closed": false,
            "folders": {}
            },
            "Buzzer": {
            "preset": "Default",
            "closed": false,
            "folders": {}
            },
            "Shiner": {
            "preset": "Default",
            "closed": false,
            "folders": {}
            },
            "Blower": {
            "preset": "Default",
            "closed": false,
            "folders": {}
            },
            "Pianoteq": {
            "preset": "Default",
            "closed": false,
            "folders": {}
        }
    }
            };
        
        window.onload = function() {
        try {
         var temporary_json = Silencio.restoreDatGuiJson(default_json);
         //default_json = temporary_json;
         gui = new dat.GUI({load: default_json, width: 400});
         csound.Message("Restored Csound instrument parameters.\\n");
         } catch(e) {
             csound.Message("Failed to restore Csound instrument parameters.\\n");
             gui = new dat.GUI({width: 400});
         }
         gui.remember(parameters);
         gui.add(parameters, 'stop').name('Stop [Ctrl-S]');
         var Master = gui.addFolder('Master');
         add_slider(Master, 'gk_MVerb_feedback', 0, 1);
         add_slider(Master, 'gk_ReverbSC_feedback', 0, 1);
         add_slider(Master, 'gk_MasterOutput_level', -40, 40);
         var FMWaterBell = gui.addFolder('FMWaterBell');
         add_slider(FMWaterBell, 'gi_FMWaterBell_attack', 0, .1);
         add_slider(FMWaterBell, 'gi_FMWaterBell_release', 0, .1);
         add_slider(FMWaterBell, 'gi_FMWaterBell_exponent', -30, 30);
         add_slider(FMWaterBell, 'gi_FMWaterBell_sustain', 0, 20);
         add_slider(FMWaterBell, 'gi_FMWaterBell_sustain_level', 0, 1);
         add_slider(FMWaterBell, 'gk_FMWaterBell_crossfade', 0, 1);
         add_slider(FMWaterBell, 'gk_FMWaterBell_index', 0, 15);
         add_slider(FMWaterBell, 'gk_FMWaterBell_vibrato_depth', 0, 10);
         add_slider(FMWaterBell, 'gk_FMWaterBell_vibrato_rate', 0, 10);
         add_slider(FMWaterBell, 'gk_FMWaterBell_level',-40, 40);
         var Phaser = gui.addFolder('Phaser');
         add_slider(Phaser, 'gk_Phaser_ratio1', 0, 5);
         add_slider(Phaser, 'gk_Phaser_ratio2', 0, 5);
         add_slider(Phaser, 'gk_Phaser_index1', 0, 15);
         add_slider(Phaser, 'gk_Phaser_index2', 0, 15);
         add_slider(Phaser, 'gk_Phaser_level', -40, 40);
         var Bower = gui.addFolder('Bower');
         add_slider(Bower, 'gk_Bower_pressure', 0, 5);
         add_slider(Bower, 'gk_Bower_level', -40, 40);
         var Droner = gui.addFolder('Droner');
         add_slider(Droner, 'gk_Droner_partial1', 0, 1);
         add_slider(Droner, 'gk_Droner_partial2', 0, 1);
         add_slider(Droner, 'gk_Droner_partial3', 0, 1);
         add_slider(Droner, 'gk_Droner_partial4', 0, 1);
         add_slider(Droner, 'gk_Droner_partial5', 0, 1);
         add_slider(Droner, 'gk_Droner_level', -40, 40);
         var Sweeper = gui.addFolder('Sweeper');
         add_slider(Sweeper, 'gk_Sweeper_britel', 0, 4);
         add_slider(Sweeper, 'gk_Sweeper_briteh', 0, 4);
         add_slider(Sweeper, 'gk_Sweeper_britels', 0, 4);
         add_slider(Sweeper, 'gk_Sweeper_britehs', 0, 4);
         add_slider(Sweeper, 'gk_Sweeper_level', -40, 40);
         var Buzzer = gui.addFolder('Buzzer');
         add_slider(Buzzer, 'gk_Buzzer_harmonics', 0, 20);
         add_slider(Buzzer, 'gk_Buzzer_level', -40, 40);
         var Shiner = gui.addFolder('Shiner');
         add_slider(Shiner, 'gk_Shiner_level', -40, 40);
         var Blower = gui.addFolder('Blower');
         add_slider(Blower, 'gk_Blower_grainDensity', 0, 400);
         add_slider(Blower, 'gk_Blower_grainDuration', 0, .5);
         add_slider(Blower, 'gk_Blower_grainAmplitudeRange', 0, 400);
         add_slider(Blower, 'gk_Blower_grainFrequencyRange', 0, 100);
         add_slider(Blower, 'gk_Blower_level', -40, 40);
         var Flute = gui.addFolder('Zakian Flute');
         add_slider(Flute, 'gk_ZakianFlute_level', -40, 40);
         var Pianoteq = gui.addFolder('Pianoteq');
         add_slider(Pianoteq, 'gk_PianoteqOut_level', -40, 40);
         gui.revert(); 
         document.addEventListener("keydown", function (e) {
         var e_char = String.fromCharCode(e.keyCode || e.charCode);
         if (e.ctrlKey === true) {
          if (e_char === 'H') {
          var console = document.getElementById("console");
          if (console.style.display === "none") {
           console.style.display = "block";
          } else {
           console.style.display = "none";
          }
          gui.closed = true;
          gui.closed = false;
          } else if (e_char === 'G') {
          generate();
          } else if (e_char === 'P') {
          parameters.play();
          } else if (e_char === 'S') {
          parameters.stop();
          }
         }
         });
        };
        var on_success = function(result) {
        };
        var on_error = function(message) {
        };
        
        var gk_update = function(name, value) {
         var numberValue = parseFloat(value);
         csound.SetControlChannel(name, numberValue, on_success, on_error);
        }
        
        var add_slider = function(gui_folder, token, minimum, maximum, name) {
         var on_parameter_change = function(value) {
         gk_update(token, value);
         };
         gui_folder.add(parameters, token, minimum, maximum).onChange(on_parameter_change);
        };
        
        window.addEventListener("unload", function(event) { 
         parameters.stop();
         nw_window.close();
        });
    var number_format = new Intl.NumberFormat('en-US', {minimumFractionDigits: 3, maximumFractionDigits: 3 });
    $(document).ready(function() {
        var callbackSuccess_ = function(result) {
        };
        var callbackError_ = function(message) {
        };
        var csound = new Csound("http://localhost:8383");
        $('input').on('input', function(event) {
            var slider_value = parseFloat(event.target.value);
            csound.SetControlChannel(event.target.id, slider_value, on_success, on_error);
            var output_selector = '#' + event.target.id + '_output';
            var formatted = number_format.format(slider_value);
            $(output_selector).val(formatted);
        });
        $('#save').on('click', function() {
            $('.persistent-element').each(function() {
                localStorage.setItem(this.id, this.value);
            });
        });
        $('#restore').on('click', function() {
            $('.persistent-element').each(function() {
                this.value = localStorage.getItem(this.id);
                csound.SetControlChannel(this.id, parseFloat(this.value), on_success, on_error);
                var output_selector = '#' + this.id + '_output';
                $(output_selector).val(this.value);
            });
        });
        $('#restore').click();
    });
    animate();
</script>
</body>
</html>
}}

gi_browser webkit_create
// The following lines find the current working directory from Csound, 
// and then use that to construct the base URI of the HTML code.
S_pwd pwd
S_base_uri sprintf "file://%s/", S_pwd
prints S_base_uri
webkit_open_html gi_browser, "Poustinia version 8", gS_html, S_base_uri, 12000, 10000, 0
endin
alwayson "Browser"
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// The following C++ code defines and executes a score generator that 
// implements the "multiple copy reducing machine" algorithm for computing an 
// iterated function system (IFS).
//
// Only a limited number of iterations are computed. On the final iteration, 
// each point in the attractor of the IFS is translated to a single note of 
// music.
//
// This code uses only the Eigen 3 header file-only library and the C++ 
// standard library.

S_score_generator_code init {{

#include <eigen3/Eigen/Dense>
#include <csound.h>
#include <csound/csdl.h>
#include <iostream>
#include <cstdio>
#include <random>
#include <vector>
#include <Composition.hpp>
#include <functional>
#include <memory>
#include <MusicModel.hpp>
#include <random>
#include <ScoreNode.hpp>
#include <VoiceleadingNode.hpp>

void* __dso_handle = (void *)&__dso_handle;
static bool diagnostics_enabled = false;

extern "C" { 

void webkit_execute(int browser_handle, std::string javascript_code);

struct Cursor
{
    csound::Event note;
    csound::Chord chord;
};

auto generator = [] (const Cursor &cursor, int depth, csound::Score &score)
{
    Cursor result = cursor;
    return result;
};

/**
 * Computes a deterministic, finite recurrent iterated function system by
 * recursively applying a set of generators (transformations) to a pen
 * which represents the position of a "pen" on a "score." The entries in
 * the transitions matrix represent open or closed paths of recurrence through
 * the tree of calls. Because the pen is passed by value, it is in effect
 * copied at each call of a generator and at each layer of recursion. The
 * generators may or may not append a local copy of the pen to the score,
 * thus "writing" a "note" or other event on the "score."
 */
void recurrent(std::vector< std::function<Cursor(const Cursor &,int, csound::Score &)> > &generators,
        Eigen::MatrixXd &transitions,
        int depth,
        int transformationIndex,
        const Cursor pen,
        csound::Score &score) {
    depth = depth - 1;
    if (depth == 0) {
        return;
    }
    // std::printf("recurrent(depth: %3d  index: %3d  %s)\\n", depth, transformationIndex, pen.toString().c_str());
    // Index is that of the current transformation. The column vector at that index determines which
    // transformations may be applied.
    for (int transitionIndex = 0, transitionN = transitions.rows(); transitionIndex < transitionN; ++transitionIndex) {
        if (transitions(transformationIndex, transitionIndex)) {
            auto newCursor = generators[transitionIndex](pen, depth, score);
            recurrent(generators, transitions, depth, transitionIndex, newCursor, score);
        }
    }
}

extern "C" int score_generator(CSOUND *csound) {
    int result = OK;
    csound::ScoreModel model;
    // These fields determine output filenames and ID 3 tags.
    model.setTitle("Blue Leaves von Ruesner 2");
    model.setFilename("Blue_Leaves_von_Ruesner-2");
    model.setAlbum("Silence");
    model.setArtist("Michael Gogins");
    model.setAuthor("Michael Gogins");
    model.setCopyright("(C) 2021 by Michael Gogins");
    std::map<double, csound::Chord> chordsForTimes;
    csound::Chord modality;
    Cursor pen;
    modality.fromString("0 4 7 11 14");
    pen.chord = modality;
    pen.note = {1,1,144,0,1,1,0,0,0,0,1};
    pen.note[csound::Event::DURATION] = 0.0125;

    std::vector<std::function<Cursor(const Cursor &, int, csound::Score &)>> generators;

    auto g1 = [&chordsForTimes, &modality](const Cursor &pen_, int depth, csound::Score &score) {
        Cursor pen = pen_;
        if (depth <= 1) {
            return pen;
        }
        pen.note[csound::Event::TIME] = pen.note[csound::Event::TIME] * .7 + .25;
        pen.note[csound::Event::KEY] = pen.note[csound::Event::KEY] + 3.105;
        pen.note[csound::Event::INSTRUMENT] = 4.0 * 1.0 + double(depth % 4);
        pen.note[csound::Event::VELOCITY] =  1.0;
        pen.note[csound::Event::PAN] = .875;
        score.append(pen.note);     
        return pen;
    };
    generators.push_back(g1);

    auto g2 = [&chordsForTimes, &modality](const Cursor &pen_, int depth, csound::Score &score) {
        Cursor pen = pen_;
        if (depth <= 2) {
            return pen;
        }
        pen.note[csound::Event::TIME] = pen.note[csound::Event::TIME] * .7464 - .203487;
        pen.note[csound::Event::KEY] = pen.note[csound::Event::KEY] *.99 + 3.0;
        pen.note[csound::Event::INSTRUMENT] = 4.0 * 2.0 + double(depth % 4);      
        pen.note[csound::Event::VELOCITY] =  2.0;                        
        pen.note[csound::Event::PAN] = .675;
        score.append(pen.note);
        return pen;
    };
    generators.push_back(g2);

    auto g3 = [&chordsForTimes, &modality](const Cursor &pen_, int depth, csound::Score &score) {
        Cursor pen = pen_;
        if (depth <= 1) {
            return pen;
        }
        pen.note[csound::Event::TIME] = pen.note[csound::Event::TIME] * .65 - .23;
        pen.note[csound::Event::KEY] = pen.note[csound::Event::KEY] - 3.07;
        pen.note[csound::Event::INSTRUMENT] = 4.0 * 3.0 + double(depth % 4);
        pen.note[csound::Event::VELOCITY] =  1.0;
        pen.note[csound::Event::PAN] = -.675;
        score.append(pen.note);
        if (depth >= 5) {
            pen.chord = pen.chord.Q(3., modality);
            chordsForTimes[pen.note.getTime()] = pen.chord;
        }
        return pen;
    };
    generators.push_back(g3);

    auto g4 = [&chordsForTimes, &modality](const Cursor &pen_, int depth, csound::Score &score) {
        Cursor pen = pen_;
        if (depth <= 0) {
            return pen;
        }
        pen.note[csound::Event::TIME] = pen.note[csound::Event::TIME] * .79 + .2;
        pen.note[csound::Event::KEY] = pen.note[csound::Event::KEY] - 4.25;
        pen.note[csound::Event::INSTRUMENT] = 4.0 * 4.0 + double(depth % 4);
        pen.note[csound::Event::VELOCITY] =  2.0;
        pen.note[csound::Event::PAN] = -.875;
        if (depth == 5) {
            pen.chord = pen.chord.K();
            chordsForTimes[pen.note.getTime()] = pen.chord;
        }
        
        return pen;
    };
    generators.push_back(g4);

    // Generate the score.
    Eigen::MatrixXd transitions(4,4);
    transitions <<  1, 1, 0, 1,
                    1, 1, 0, 1,
                    0, 1, 1, 0,
                    1, 0, 1, 1;
    //csound::ScoreNode scoreNode;
    //model.addChild(&scoreNode);
    csound::Score score;
    recurrent(generators, transitions, 9, 0, pen, score);
    std::cout << "Generated duration:     " << score.getDuration() << std::endl;
    score.rescale(csound::Event::KEY,           true, 24.0, true,  72.0);
    score.temper(12.);
    score.sort();
    std::cout << "Generated notes:        " << score.size() << std::endl;
    double endTime = score.back().getTime();
    std::cout << "Chord segments:         " << chordsForTimes.size() << std::endl;
    int size = 0;
    for (auto it = chordsForTimes.rbegin(); it != chordsForTimes.rend(); ++it) {
        auto startTime = it->first;
        auto &chord = it->second;
        auto segment = csound::slice(score, startTime, endTime);
        size += segment.size();
        std::fprintf(stderr, "From %9.4f to %9.4f apply %s to %d notes.\\n", startTime, endTime, chord.eOP().name().c_str(), segment.size());
        std::fprintf(stderr, "Before:\\n");
        for (int i = 0, n = segment.size(); i < n; ++i) {
            std::fprintf(stderr, "  %s\\n", segment[i]->toString().c_str());
        }  
        csound::apply(score, chord, startTime, endTime, true);
        std::fprintf(stderr, "After:\\n");
        for (int i = 0, n = segment.size(); i < n; ++i) {
            std::fprintf(stderr, "  %s\\n", segment[i]->toString().c_str());
        }  
        endTime = startTime;
    }
    std::cout << "Conformed notes:        " << size << std::endl;
    score.rescale(csound::Event::TIME,          true,  0.0, false,  0.0);
    score.rescale(csound::Event::INSTRUMENT,    true,  1.0, true,   9.99999);
    score.rescale(csound::Event::VELOCITY,      true, 40.0, true,   0.0);
    score.rescale(csound::Event::DURATION,      true,  2.0, true,   4.0);
    score.rescale(csound::Event::PAN,           true,  0.0, true,   0.0);
    std::cout << "Move to origin duration:" << score.getDuration() << std::endl;
    score.setDuration(240.0);
    std::cout << "set duration:           " << score.getDuration() << std::endl;
    score.rescale(csound::Event::DURATION,      true,  2.375, true,   4.0);
    score.tieOverlappingNotes(true);
    score.findScale();
    score.setDuration(360.0);
    std::mt19937 mersenneTwister;
    std::uniform_real_distribution<> randomvariable(.05,.95);
    for (int i = 0, n = score.size(); i < n; ++i) {
        score[i].setPan(randomvariable(mersenneTwister));
        score[i].setDepth(randomvariable(mersenneTwister));
        score[i].setPhase(randomvariable(mersenneTwister));
    }
    std::cout << "Final duration:         " << score.getDuration() << std::endl;
    // Using the EVTBLK struct for each note is more efficient than using a 
    // string for each note.
    EVTBLK evtblk;
    std::memset(&evtblk, 0, sizeof(EVTBLK));
    for (const auto &note : score) {
        evtblk.strarg = nullptr;
        evtblk.scnt = 0;
        evtblk.opcod = 'i';
        evtblk.pcnt = 9;
        evtblk.p[1] = std::floor(note.getInstrument());
        evtblk.p[2] = note.getTime();
        evtblk.p[3] = note.getDuration();
        evtblk.p[4] = note.getKey();
        evtblk.p[5] = note.getVelocity();
        evtblk.p[6] = note.getDepth();
        evtblk.p[7] = note.getPan();
        evtblk.p[8] = note.getHeight();
        std::fprintf(stderr, "%c %9.4f %9.4f %9.4f %9.4f %9.4f %9.4f %9.4f %9.4f\\n", evtblk.opcod, evtblk.p[1], evtblk.p[2], evtblk.p[3], evtblk.p[4], evtblk.p[5], evtblk.p[6], evtblk.p[7], evtblk.p[8]);
        int result = csound->insert_score_event(csound, &evtblk, 0.);
    }
    auto json_score = score.toJson();
    std::cerr << "JSON score:" << std::endl << json_score << std::endl;
    std::string javascript_code = "piano_roll.fromJson(`" + json_score + "`);";
    std::cerr << javascript_code << std::endl; 
    webkit_execute(0, javascript_code);
    return result;
};

};

}}

i_result clang_compile "score_generator", S_score_generator_code, "-g -O2 -std=c++17 -I/home/mkg/clang-opcodes -I/home/mkg/csound-extended/CsoundAC -I/usr/local/include/csound -stdlib=libstdc++", "/usr/lib/libCsoundAC.so.6.0 /usr/lib/gcc/x86_64-linux-gnu/9/libstdc++.so /usr/lib/gcc/x86_64-linux-gnu/9/libgcc_s.so /home/mkg/webkit-opcodes/webkit_opcodes.so /usr/lib/x86_64-linux-gnu/libm.so /usr/lib/x86_64-linux-gnu/libpthread.so"

//endin
//alwayson "ScoreGenerator"

instr Exit
prints "exitnow i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
exitnow
endin

</CsInstruments>
<CsScore>
; f 0 does not work here, we actually need to schedule an instrument that 
; turns off Csound.
i "Exit" [11 * 60 + 5]
</CsScore>
</CsoundSynthesizer>