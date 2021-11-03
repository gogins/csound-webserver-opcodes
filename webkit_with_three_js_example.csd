<CsoundSyntheizer>
<CsLicense>

I F S   S T U D Y   N O .   6

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

sr = 48000
ksmps = 128
nchnls = 2
0dbfs = 20

gi_maximum_voices init 10

// VST plugins must be loaded ahead of instruments that use them.

gi_Pianoteq vstinit "/home/mkg/Pianoteq\ 7/x86-64bit/Pianoteq\ 7.so", 0
gi_Mverb2020 vstinit "/home/mkg/.local/lib/Mverb2020.so", 1

// The following C++ code defines two invokable "opcodes" that compute a 
// phase-synchronous Gaussian chirp grain, or a phase-synchronous cosine 
// grain.

S_grain_code init {{

// Some C++ modules use this symbol, but the Clang/LLVM startup code does not
// automatically define it. Therefore, we define it here.

void* __dso_handle = (void *)&__dso_handle;

static bool diagnostics_enabled = false;

#include <cmath>
#include <complex>
#include <csound/csdl.h>
#include <cstdio>

/*
clang_invokable.hpp - this file is part of clang-opcodes.

Copyright (C) 2021 by Michael Gogins

clang-opcodes is free software; you can redistribute it
and/or modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

clang-opcodes is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with clang-opcodes; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
02110-1301 USA
*/

#include <csdl.h>
#include <cstring>

extern "C" {
    void webkit_execute(int handle, const char *javascript_code);
};

/**
 * Defines the pure abstract interface implemented by Clang modules to be 
 * called by Csound using the `clang_invoke` opcode.
 */
struct ClangInvokable {
	virtual ~ClangInvokable() {};
	/**
	 * Called once at init time. The inputs are the same as the 
	 * parameters passed to the `clang_invoke` opcode. The outputs become 
	 * the values returned from the `clang_invoke` opcode. Performs the 
	 * same work as `iopadr` in a standard Csound opcode definition. The 
	 * `opds` argument can be used to find many things about the invoking 
     * opcde and its enclosing instrument.
	 */
	virtual int init(CSOUND *csound, OPDS *opds, MYFLT **outputs, MYFLT **inputs) = 0;
	/**
	 * Called once every kperiod. The inputs are the same as the 
	 * parameters passed to the `clang_invoke` opcode. The outputs become 
	 * the values returned from the `clang_invoke` opcode. Performs the 
	 * same work as `kopadr` in a standard Csound opcode definition.
	 */
	virtual int kontrol(CSOUND *csound, MYFLT **outputs, MYFLT **inputs) = 0;
	/**
	 * Called by Csound when the Csound instrument that contains this 
	 * instance of the ClangInvokable is turned off.
	 */
	virtual int noteoff(CSOUND *csound) = 0;
};

/**
 * Concrete base class that implements `ClangInvokable`, with some helper 
 * facilities. Most users will implement a ClangInvokable by inheriting from 
 * `ClangInvokableBase` and overriding one or more of its virtual methods.
 */
class ClangInvokableBase : public ClangInvokable {
    public:
        virtual ~ClangInvokableBase() {
        };
        int init(CSOUND *csound_, OPDS *opds_, MYFLT **outputs, MYFLT **inputs) override {
            int result = OK;
            csound = csound_;
            opds = opds_;
            return result;
        }
        /**
         * Computes one sample of one frame of output audio.
         * The `kontrol` method then calls this as appropriate for 
         * --sample-accurate rendering. The implemention of this method must 
         * be compatible with Csound's inputs to `clang_invoke`.
         */
        virtual MYFLT tick(MYFLT **inputs) {
            MYFLT value = 0;
            return value;
        }
         int kontrol(CSOUND *csound_, MYFLT **outputs, MYFLT **inputs) override {
            int result = OK;
            int frame_index = 0;
            MYFLT input = 0.;
            for( ; frame_index < kperiodOffset(); ++frame_index) {
                outputs[0][frame_index] = 0;
            }
            for( ; frame_index < kperiodEnd(); ++frame_index) {
                MYFLT sample = tick(inputs);
                outputs[0][frame_index] = sample;
            }
            for( ; frame_index < ksmps(); ++frame_index) {
                outputs[0][frame_index] = 0;
            }
            return result;
        }
        int noteoff(CSOUND *csound) override 
        {
            int result = OK;
            return result;
        }
        uint32_t kperiodOffset() const
        {
            return opds->insdshead->ksmps_offset;
        }
        uint32_t kperiodEnd() const
        {
            uint32_t end = opds->insdshead->ksmps_no_end;
            if (end) {
                return end;
            } else {
                return ksmps();
            }
        }
        uint32_t ksmps() const
        {
            return opds->insdshead->ksmps;
        }
        uint32_t output_arg_count()
        {
            return (uint32_t)opds->optext->t.outArgCount;
        }
        uint32_t input_arg_count()
        {
            // The first two input arguments belong to the invoking opcode.
            return (uint32_t)opds->optext->t.inArgCount - 2;
        }
        void log(const char *format,...)
        {
            va_list args;
            va_start(args, format);
            if(csound) {
                csound->MessageV(csound, 0, format, args);
            } else {
                vfprintf(stdout, format, args);
            }
            va_end(args);
        }
        void warn(const char *format,...)
        {
            if(csound) {
                if(csound->GetMessageLevel(csound) & WARNMSG) {
                    va_list args;
                    va_start(args, format);
                    csound->MessageV(csound, CSOUNDMSG_WARNING, format, args);
                    va_end(args);
                }
            } else {
                va_list args;
                va_start(args, format);
                vfprintf(stdout, format, args);
                va_end(args);
            }
        }
    protected:
        CSOUND *csound = nullptr;
        OPDS *opds = nullptr;
};

/**
 * Synthesizes a Jones-Parks grain, i.e. a Gaussian chirp, which can be a 
 * simple grain without any actual chirp. If the synchronous_phase argument is 
 * true, then all grains of the same frequency will have synchronous phases, 
 * which can be useful in avoiding certain artifacts.
 *
 * The algorithm uses an efficient difference equation.
 */
struct InvokableGrain : public ClangInvokableBase {
    virtual ~InvokableGrain() {
    };
    int init(CSOUND *csound_, OPDS *opds_, MYFLT **outputs, MYFLT **inputs) override {
        if (diagnostics_enabled) std::fprintf(stderr, ">>>>>>> InvokableGrain::init...\\n");
        int result = OK;
        csound = csound_;
        opds = opds_;
        // Inputs:
        center_time_seconds = *(inputs[0]);
        duration_seconds = *(inputs[1]);
        starting_frequency_hz = *(inputs[2]);
        center_frequency_hz = *(inputs[3]);
        center_amplitude = *(inputs[4]);
        center_phase_offset_radians = *(inputs[5]);
        if (*(inputs[6]) != 0.) {
            synchronous_phase = true;
        } else {
            synchronous_phase = false;
        }
        if (synchronous_phase) {
            wavelength_seconds = 1.0 / center_frequency_hz;
            wavelengths = center_time_seconds / wavelength_seconds;
            whole_cycles = 0;
            fractional_cycle = std::modf(wavelengths, &whole_cycles);
            center_phase_offset_radians = 2.0 * M_PI * fractional_cycle;
        }
        center_time = - (duration_seconds / 2.0);
        sampling_rate = csound->GetSr(csound);
        sampling_interval = 1.0 / double(sampling_rate);
        frame_count = size_t(2.0 * duration_seconds / sampling_interval);
        gaussian_width = std::exp(1.0) / std::pow(duration_seconds / 4.0, 2.0);
        ending_frequency_hz = center_frequency_hz + (center_frequency_hz - starting_frequency_hz);
        chirp_rate = (ending_frequency_hz - starting_frequency_hz) / duration_seconds;
        omega = 2.0 * M_PI * center_frequency_hz;
        c0 = std::complex<double>(std::log(center_amplitude) - (gaussian_width * std::pow(center_time, 2.0)), 
                                (center_phase_offset_radians - (chirp_rate / 2.0) * center_time) - (omega * center_time));
        c1 = std::complex<double>(-2.0 * gaussian_width * sampling_interval * center_time,
                                - (sampling_interval * (chirp_rate * center_time + omega)));
        c2 = std::complex<double>(-std::complex<double>(gaussian_width, chirp_rate / 2.0)) * std::pow(sampling_interval, 2.0);
        exp_2_c2 = std::exp(2.0 * c2);
        h0 = std::exp(c1 + c2);
        h1 =std::complex<double>(0.0, 0.0);
        f0 = std::exp(c0);
        f1 = std::complex<double>(0.0, 0.0);
        if (diagnostics_enabled) std::fprintf(stderr, ">>>>>>> InvokableGrain::init.\\n");
        return result;
    }
    // The difference equation.
    MYFLT tick() {
        MYFLT sample = f0.real();
        h1 = h0 * exp_2_c2;
        h0 = h1;
        f1 = h1 * f0;
        f0 = f1;
        return sample;
    }
    int kontrol(CSOUND *csound_, MYFLT **outputs, MYFLT **inputs) override {
        int result = OK;
        int frame_index = 0;
        for( ; frame_index < kperiodOffset(); ++frame_index) {
            outputs[0][frame_index] = 0;
        }
        for( ; frame_index < kperiodEnd(); ++frame_index) {
            MYFLT sample = tick();
            outputs[0][frame_index] = sample;
        }
        for( ; frame_index < ksmps(); ++frame_index) {
            outputs[0][frame_index] = 0;
        }
        return result;
    }
    double center_time_seconds;
    double duration_seconds;
    double starting_frequency_hz;
    double center_frequency_hz;
    double center_amplitude;
    double center_phase_offset_radians;
    bool synchronous_phase;
    double wavelength_seconds;
    double wavelengths;
    double whole_cycles;
    double fractional_cycle;
    double center_time;
    int sampling_rate;
    double sampling_interval;
    size_t frame_count;
    double gaussian_width;
    double ending_frequency_hz;
    double chirp_rate;
    double omega;
    // Difference equation terms.
    std::complex<double> c0;
    std::complex<double> c1;
    std::complex<double> c2;
    std::complex<double> exp_2_c2;
    std::complex<double> h0;
    std::complex<double> h1;
    std::complex<double> f0;
    std::complex<double> f1;
};

/**
 * Computes a cosine grain. If the synchronous_phase argument is true
 * (the default value), then all grains of the same frequency
 * will have synchronous phases, which can be useful in avoiding certain artifacts.
 * For example, if cosine grains of the same frequency have synchronous phases,
 * they can be overlapped by 1/2 their duration without artifacts
 * to produce a continuous cosine tone.
 *
 * The algorithm uses an efficient difference equation.
 */
struct InvokableCosineGrain : public ClangInvokableBase {
    virtual ~InvokableCosineGrain() {
    };
    int init(CSOUND *csound_, OPDS *opds_, MYFLT **outputs, MYFLT **inputs) override {
        if (diagnostics_enabled) std::fprintf(stderr, ">>>>>>> InvokableCosineGrain::init...\\n");
        int result = OK;
        csound = csound_;
        opds = opds_;
        // Inputs.
        center_time_seconds = *(inputs[0]);
        duration_seconds = *(inputs[1]);
        frequency_hz = *(inputs[2]);
        amplitude = *(inputs[3]);
        phase_offset_radians = *(inputs[4]);
        if (*(inputs[5]) != 0.) {
            synchronous_phase = true;
        } else {
            synchronous_phase = false;
        }
        if (synchronous_phase) {
            wavelength_seconds = 1.0 / frequency_hz;
            wavelengths = center_time_seconds / wavelength_seconds;
            whole_cycles = 0;
            fractional_cycle = std::modf(wavelengths, &whole_cycles);
            phase_offset_radians = 2.0 * M_PI * fractional_cycle;
        }
        frames_per_second = csound->GetSr(csound);
        frame_count = size_t(std::round(duration_seconds * frames_per_second));
        // The signal is a cosine sinusoid.
        sinusoid_radians_per_frame = 2.0 * M_PI * frequency_hz / frames_per_second;
        sinusoid_coefficient = 2.0 * std::cos(sinusoid_radians_per_frame);
        // The initial frame.
        sinusoid_1 = std::cos(phase_offset_radians);
        // What would have been the previous frame.
        sinusoid_2 = std::cos(-sinusoid_radians_per_frame + phase_offset_radians);
        // The envelope is exactly 1 cycle of a cosine sinusoid, offset by -1.
        envelope_frequency_hz = 1.0 / duration_seconds;
        envelope_radians_per_frame = 2.0 * M_PI * envelope_frequency_hz / frames_per_second;
        envelope_coefficient = 2.0 * std::cos(envelope_radians_per_frame);
        // The initial frame.
        envelope_1 = std::cos(0.0);
        // What would have been the previous frame.
        envelope_2 = std::cos(-envelope_radians_per_frame);
        signal = 0.0;
        temporary = 0.0;
        return result;
    }
    // The difference equation.
    MYFLT tick(MYFLT  **inputs) override {
        signal = (sinusoid_1 * (envelope_1 - 1.0)) * amplitude;
        temporary = sinusoid_1;
        sinusoid_1 = sinusoid_coefficient * sinusoid_1 - sinusoid_2;
        sinusoid_2 = temporary;
        temporary = envelope_1;
        envelope_1 = envelope_coefficient * envelope_1 - envelope_2;
        envelope_2 = temporary;
        return signal;
    }
    double center_time_seconds;
    double duration_seconds;
    double frequency_hz;
    double amplitude;
    double phase_offset_radians;
    double wavelengths;
    double wavelength_seconds;
    double whole_cycles;
    double fractional_cycle;
    bool synchronous_phase;
    int frame_count;
    double frames_per_second;
    double sinusoid_radians_per_frame;
    double sinusoid_coefficient;    
    double sinusoid_1;
    double sinusoid_2;
    double envelope_frequency_hz;
    double envelope_radians_per_frame;
    double envelope_coefficient;
    double envelope_1;
    double envelope_2;
    double signal;
    double temporary;
};

extern "C" {
    
    int grain_main(CSOUND *csound) {
        int result = OK;
        if (diagnostics_enabled) std::fprintf(stderr, ">>>>>>> This is \\"grain_main\\".\\n");
        return result;
    }
    
    ClangInvokable *grain_factory() {
        if (diagnostics_enabled) std::fprintf(stderr, ">>>>>>> This is \\"grain_factory\\".\\n");
        auto result = new InvokableGrain;
        if (diagnostics_enabled) std::fprintf(stderr, ">>>>>>> \\"grain_factory\\" created %p.\\n", result);
        return result;
    }
    
    ClangInvokable *cosine_grain_factory() {
        if (diagnostics_enabled) std::fprintf(stderr, ">>>>>>> This is \\"cosine_grain_factory\\".\\n");
        auto result = new InvokableCosineGrain;
        if (diagnostics_enabled) std::fprintf(stderr, ">>>>>>> \\"cosine_grain_factory\\" created %p.\\n", result);
        return result;
    }

};

}}

i_result clang_compile "grain_main", S_grain_code, "-g -Ofast -march=native -std=c++14 -I/home/mkg/clang-opcodes -I/usr/local/include/csound  -I. -stdlib=libstdc++", "/usr/lib/gcc/x86_64-linux-gnu/9/libstdc++.so /usr/lib/gcc/x86_64-linux-gnu/9/libgcc_s.so /usr/local/lib/libstk.so /usr/lib/x86_64-linux-gnu/libm.so /usr/lib/x86_64-linux-gnu/libpthread.so"

// Instruments are defined in blocks, along with their signal flow graph 
// connections, the initial values of their control parameters, and whether 
// they are "always on." 
//
// This ensures that the whole block for an instrument can be cut and pasted 
// as a unit without affecting other instruments.

//////////////////////////////////////////////////////////////////////////////
// Original by Steven Yi.
// Adapted by Michael Gogins.
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
gk_FMWaterBell_index init .5
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
; a_envelope transegr 0, gi_FMWaterBell_attack, 12, 1, i_sustain, 12, gi_FMWaterBell_sustain_level, gi_FMWaterBell_release, 12, 0
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
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin
connect "FMWaterBell", "outleft",  "Mverb2020", "inleft"
connect "FMWaterBell", "outright", "Mverb2020", "inright"
gk_FMWaterBell_level init 0
gi_FMWaterBell_attack init 0.002
gi_FMWaterBell_release init 0.01
gi_FMWaterBell_sustain init 20
gi_FMWaterBell_sustain_level init .1
gk_FMWaterBell_index init .5
gk_FMWaterBell_crossfade init .5
gk_FMWaterBell_vibrato_depth init 0.05
gk_FMWaterBell_vibrato_rate init 6
gk_FMWaterBell_midi_dynamic_range init 20
maxalloc "FMWaterBell", gi_maximum_voices
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
gi_FaustBubble_compiled faustcompile {{

declare name "bubble";
declare description "Production of a water drop bubble sound.";
declare license "MIT";
declare copyright "(c) 2017: Yann Orlarey, GRAME";

import("stdfaust.lib");

//---------------------------`bubble`--------------------------
// bubble(f0, trig) : produces a water drop bubble sound
//
// #### Usage
//
// ```
// bubble(f0, trig) : _
// ```
//
// Where:
//
// * ` f0 `: base frequency of bubble sound
// * `trig`: trigs the bubble sound on the rising front
//
// #### Example
//
// ```
// button("drop") : bubble(600) : _
// ```
//
// #### Reference:
//
// <http://www.cs.ubc.ca/~kvdoel/publications/tap05.pdf>
//------------------------------------------------------------

bubble(f0,trig) = os.osc(f) * (exp(-damp*time) : si.smooth(0.99))
	with {
		damp = 0.043*f0 + 0.0014*f0^(3/2);
		f = f0*(1+sigma*time);
		sigma = eta * damp;
		eta = 0.075;
		time = 0 : (select2(trig>trig'):+(1)) ~ _ : ba.samp2sec;
	};

process = button("drop") : bubble(hslider("v:bubble/freq", 600, 150, 2000, 1)) <: dm.freeverb_demo;

}}, "--import-dir \"/home/mkg/faust/libraries\"", 0

print gi_FaustBubble_compiled

gk_FaustBubble_level chnexport "gk_FaustBubble_level", 3
gk_FaustBubble_midi_dynamic_range chnexport "gk_FaustBubble_midi_dynamic_range", 3

gk_FaustBubble_level init 0
gk_FaustBubble_midi_dynamic_range init 20

instr FaustBubble
i_attack = .005
i_sustain = p3
i_release = .1
xtratim i_attack + i_release
i_instrument = p1
i_time = p2
i_midi_key = p4
while i_midi_key < 49 do
  i_midi_key = i_midi_key + 12
od
while i_midi_key > 99 do
  i_midi_key = i_midi_key - 12
od
i_midi_dynamic_range = i(gk_FaustBubble_midi_dynamic_range)
i_midi_velocity = p5 * i_midi_dynamic_range / 127 + (63.5 - i_midi_dynamic_range / 2)
k_space_front_to_back = p6
k_space_left_to_right = p7
k_space_bottom_to_top = p8
i_phase = p9
i_frequency = cpsmidinn(i_midi_key)
; Adjust the following value until "overall amps" at the end of performance is about -6 dB.
i_level_correction = 68
i_normalization = ampdb(-i_level_correction) / 2
i_amplitude = ampdb(i_midi_velocity) * i_normalization
k_gain = ampdb(gk_FaustBubble_level)
i_faust_dsp faustdsp gi_FaustBubble_compiled
k_frequency cpsmidinn i_midi_key
faustctl i_faust_dsp, "freq", k_frequency
faustctl i_faust_dsp, "drop", k(1)
a_left, a_right faustplay i_faust_dsp
a_declicking linsegr 0, i_attack, 1, i_sustain, 1, i_release, 0
a_left = a_left * i_amplitude * a_declicking * k_gain
a_right = a_right * i_amplitude * a_declicking * k_gain

#ifdef USE_SPATIALIZATION
a_signal = a_left + a_right
a_spatial_reverb_send init 0
a_bsignal[] init 16
a_bsignal, a_spatial_reverb_send Spatialize a_signal, k_space_front_to_back, k_space_left_to_right, k_space_bottom_to_top
outletv "outbformat", a_bsignal
outleta "out", a_spatial_reverb_send
#else
outleta "outleft", a_left
outleta "outright", a_right
#endif
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin
connect "FaustBubble", "outleft",  "Mverb2020", "inleft"
connect "FaustBubble", "outright", "Mverb2020", "inright"
gk_FaustBubble_level init 0
maxalloc "FaustBubble", gi_maximum_voices
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
gk_Rhodes_level chnexport "gk_Rhodes_level", 3 ;  0
gk_Rhodes_level init 0

gi_Rhodes_sine ftgen 0, 0, 65537, 10, 1
gi_Rhodes_cosine ftgen 0, 0, 65537, 11, 1
gi_Rhodes_blank ftgen 0, 0, 65537, 10, 0 ; Blank wavetable for some Cook FM opcodes.

instr Rhodes
; Authors: Perry Cook, John ffitch, Michael Gogins
i_instrument = p1
i_time = p2
i_duration = p3
i_midi_key = p4
i_midi_velocity = p5
k_space_front_to_back = p6
k_space_left_to_right = p7
k_space_bottom_to_top = p8
i_phase = p9
i_frequency = cpsmidinn(i_midi_key)
; Adjust the following value until "overall amps" at the end of performance is about -6 dB.
i_overall_amps = 82
i_normalization = ampdb(-i_overall_amps) / 2
i_amplitude = ampdb(i_midi_velocity) * i_normalization
k_gain = ampdb(gk_Rhodes_level)
iindex = 4
icrossfade = 3
ivibedepth = 0.2
iviberate = 6
ifn1 = gi_Rhodes_sine
ifn2 = gi_Rhodes_cosine
ifn3 = gi_Rhodes_sine
ifn4 = gi_Rhodes_blank
ivibefn = gi_Rhodes_sine
a_signal fmrhode i_amplitude, i_frequency, iindex, icrossfade, ivibedepth, iviberate, ifn1, ifn2, ifn3, ifn4, ivibefn
i_attack = .002
i_sustain = p3
i_release = 0.01
xtratim i_attack + i_release
a_declicking linsegr 0, i_attack, 1, i_sustain, 1, i_release, 0
a_signal = a_signal * a_declicking * k_gain

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
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin
connect "Rhodes", "outleft",  "Mverb2020", "inleft"
connect "Rhodes", "outright", "Mverb2020", "inright"
gk_Rhodes_level init 0
maxalloc "Rhodes", gi_maximum_voices
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
gk_ZakianFlute_midi_dynamic_range chnexport "gk_ZakianFlute_midi_dynamic_range", 3 ;  20
gk_ZakianFlute_level chnexport "gk_ZakianFlute_level", 3 ;  0
gk_ZakianFlute_pan chnexport "gk_ZakianFlute_pan", 3 ;  .5
gi_ZakianFLute_seed chnexport "gi_ZakianFLute_seed", 3 ;  .5

gk_ZakianFlute_midi_dynamic_range init 20
gk_ZakianFlute_level init 0
gk_ZakianFlute_pan init .5
gi_ZakianFLute_seed init .5

gi_ZakianFLute_f2  ftgen 0, 0, 16, -2, 40, 40, 80, 160, 320, 640, 1280, 2560, 5120, 10240, 10240
gi_ZakianFlute_f26 ftgen 0, 0, 65537, -10, 2000, 489, 74, 219, 125, 9, 33, 5, 5
gi_ZakianFlute_f27 ftgen 0, 0, 65537, -10, 2729, 1926, 346, 662, 537, 110, 61, 29, 7
gi_ZakianFlute_f28 ftgen 0, 0, 65537, -10, 2558, 2012, 390, 361, 534, 139, 53, 22, 10, 13, 10
gi_ZakianFlute_f29 ftgen 0, 0, 65537, -10, 12318, 8844, 1841, 1636, 256, 150, 60, 46, 11
gi_ZakianFlute_f30 ftgen 0, 0, 65537, -10, 1229, 16, 34, 57, 32
gi_ZakianFlute_f31 ftgen 0, 0, 65537, -10, 163, 31, 1, 50, 31
gi_ZakianFlute_f32 ftgen 0, 0, 65537, -10, 4128, 883, 354, 79, 59, 23
gi_ZakianFlute_f33 ftgen 0, 0, 65537, -10, 1924, 930, 251, 50, 25, 14
gi_ZakianFlute_f34 ftgen 0, 0, 65537, -10, 94, 6, 22, 8
gi_ZakianFlute_f35 ftgen 0, 0, 65537, -10, 2661, 87, 33, 18
gi_ZakianFlute_f36 ftgen 0, 0, 65537, -10, 174, 12
gi_ZakianFlute_f37 ftgen 0, 0, 65537, -10, 314, 13
gi_ZakianFlute_wtsin ftgen 0, 0, 65537, 10, 1

instr ZakianFlute
; Author: Lee Zakian
; Adapted by: Michael Gogins
i_instrument = p1
i_time = p2
i_duration = p3
i_midi_key = p4
i_midi_velocity = p5
k_space_front_to_back = p6
k_space_left_to_right = p7
k_space_bottom_to_top = p8
i_phase = p9
i_overall_amps = 65.2
i_normalization = ampdb(-i_overall_amps) / 2
i_midi_dynamic_range = i(gk_ZakianFlute_midi_dynamic_range)
i_midi_velocity = p5 * i_midi_dynamic_range / 127 + (63.5 - i_midi_dynamic_range / 2)
i_amplitude = ampdb(i_midi_velocity) * i_normalization
k_gain = ampdb(gk_ZakianFlute_level)
;;;iattack = .002
;;;isustain = p3
;;;irelease = .3
;;;xtratim iattack + irelease
iHz = cpsmidinn(i_midi_key)
kHz = k(iHz)
// Bug?
// aenvelope transeg 1.0, 20.0, -10.0, 0.05
aenvelope transegr 1.0, 20.0, -10.0, 0.05
ip3 = (p3 < 3.0 ? p3 : 3.0)
; parameters
; p4 overall amplitude scaling factor
ip4 init i_amplitude
; p5 pitch in Hertz (normal pitch range: C4-C7)
ip5 init iHz
; p6 percent vibrato depth, recommended values in range [-1., +1.]
ip6 init 0.5
; 0.0 -> no vibrato
; +1. -> 1% vibrato depth, where vibrato rate increases slightly
; -1. -> 1% vibrato depth, where vibrato rate decreases slightly
; p7 attack time in seconds
; recommended value: .12 for slurred notes, .06 for tongued notes
; (.03 for short notes)
ip7 init .08
; p8 decay time in seconds
; recommended value: .1 (.05 for short notes)
ip8 init .08
; p9 overall brightness / filter cutoff factor
; 1 -> least bright / minimum filter cutoff frequency (40 Hz)
; 9 -> brightest / maximum filter cutoff frequency (10,240Hz)
ip9 init 5
; initial variables
iampscale = ip4 ; overall amplitude scaling factor
ifreq = ip5 ; pitch in Hertz
ivibdepth = abs(ip6*ifreq/100.0) ; vibrato depth relative to fundamental frequency
iattack = ip7 * (1.1 - .2*gi_ZakianFLute_seed) ; attack time with up to +-10% random deviation
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947) ; reset gi_ZakianFLute_seed
idecay = ip8 * (1.1 - .2*gi_ZakianFLute_seed) ; decay time with up to +-10% random deviation
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
ifiltcut tablei ip9, gi_ZakianFLute_f2 ; lowpass filter cutoff frequency
iattack = (iattack < 6/kr ? 6/kr : iattack) ; minimal attack length
idecay = (idecay < 6/kr ? 6/kr : idecay) ; minimal decay length
isustain = p3 - iattack - idecay
p3 = (isustain < 5/kr ? iattack+idecay+5/kr : p3) ; minimal sustain length
isustain = (isustain < 5/kr ? 5/kr : isustain)
iatt = iattack/6
isus = isustain/4
idec = idecay/6
iphase = gi_ZakianFLute_seed ; use same phase for all wavetables
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
; vibrato block
; kvibdepth linseg .1, .8*p3, 1, .2*p3, .7
kvibdepth linseg .1, .8*ip3, 1, isustain, 1, .2*ip3, .7
kvibdepth = kvibdepth* ivibdepth ; vibrato depth
kvibdepthr randi .1*kvibdepth, 5, gi_ZakianFLute_seed ; up to 10% vibrato depth variation
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
kvibdepth = kvibdepth + kvibdepthr
ivibr1 = gi_ZakianFLute_seed ; vibrato rate
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
ivibr2 = gi_ZakianFLute_seed
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
if ip6 < 0 goto vibrato1
kvibrate linseg 2.5+ivibr1, p3, 4.5+ivibr2 ; if p6 positive vibrato gets faster
goto vibrato2
vibrato1:
ivibr3 = gi_ZakianFLute_seed
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
kvibrate linseg 3.5+ivibr1, .1, 4.5+ivibr2, p3-.1, 2.5+ivibr3 ; if p6 negative vibrato gets slower
vibrato2:
kvibrater randi .1*kvibrate, 5, gi_ZakianFLute_seed ; up to 10% vibrato rate variation
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
kvibrate = kvibrate + kvibrater
kvib oscili kvibdepth, kvibrate, gi_ZakianFlute_wtsin
ifdev1 = -.03 * gi_ZakianFLute_seed ; frequency deviation
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
ifdev2 = .003 * gi_ZakianFLute_seed
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
ifdev3 = -.0015 * gi_ZakianFLute_seed
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
ifdev4 = .012 * gi_ZakianFLute_seed
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
kfreqr linseg ifdev1, iattack, ifdev2, isustain, ifdev3, idecay, ifdev4
kfreq = kHz * (1 + kfreqr) + kvib
if ifreq < 427.28 goto range1 ; (cpspch(8.08) + cpspch(8.09))/2
if ifreq < 608.22 goto range2 ; (cpspch(9.02) + cpspch(9.03))/2
if ifreq < 1013.7 goto range3 ; (cpspch(9.11) + cpspch(10.00))/2
goto range4
; wavetable amplitude envelopes
range1: ; for low range tones
kamp1 linseg 0, iatt, 0.002, iatt, 0.045, iatt, 0.146, iatt, \
0.272, iatt, 0.072, iatt, 0.043, isus, 0.230, isus, 0.000, isus, \
0.118, isus, 0.923, idec, 1.191, idec, 0.794, idec, 0.418, idec, \
0.172, idec, 0.053, idec, 0
kamp2 linseg 0, iatt, 0.009, iatt, 0.022, iatt, -0.049, iatt, \
-0.120, iatt, 0.297, iatt, 1.890, isus, 1.543, isus, 0.000, isus, \
0.546, isus, 0.690, idec, -0.318, idec, -0.326, idec, -0.116, idec, \
-0.035, idec, -0.020, idec, 0
kamp3 linseg 0, iatt, 0.005, iatt, -0.026, iatt, 0.023, iatt, \
0.133, iatt, 0.060, iatt, -1.245, isus, -0.760, isus, 1.000, isus, \
0.360, isus, -0.526, idec, 0.165, idec, 0.184, idec, 0.060, idec, \
0.010, idec, 0.013, idec, 0
iwt1 = gi_ZakianFlute_f26 ; wavetable numbers
iwt2 = gi_ZakianFlute_f27
iwt3 = gi_ZakianFlute_f28
inorm = 3949
goto end
range2: ; for low mid-range tones
kamp1 linseg 0, iatt, 0.000, iatt, -0.005, iatt, 0.000, iatt, \
0.030, iatt, 0.198, iatt, 0.664, isus, 1.451, isus, 1.782, isus, \
1.316, isus, 0.817, idec, 0.284, idec, 0.171, idec, 0.082, idec, \
0.037, idec, 0.012, idec, 0
kamp2 linseg 0, iatt, 0.000, iatt, 0.320, iatt, 0.882, iatt, \
1.863, iatt, 4.175, iatt, 4.355, isus, -5.329, isus, -8.303, isus, \
-1.480, isus, -0.472, idec, 1.819, idec, -0.135, idec, -0.082, idec, \
-0.170, idec, -0.065, idec, 0
kamp3 linseg 0, iatt, 1.000, iatt, 0.520, iatt, -0.303, iatt, \
0.059, iatt, -4.103, iatt, -6.784, isus, 7.006, isus, 11, isus, \
12.495, isus, -0.562, idec, -4.946, idec, -0.587, idec, 0.440, idec, \
0.174, idec, -0.027, idec, 0
iwt1 = gi_ZakianFlute_f29
iwt2 = gi_ZakianFlute_f30
iwt3 = gi_ZakianFlute_f31
inorm = 27668.2
goto end
range3: ; for high mid-range tones
kamp1 linseg 0, iatt, 0.005, iatt, 0.000, iatt, -0.082, iatt, \
0.36, iatt, 0.581, iatt, 0.416, isus, 1.073, isus, 0.000, isus, \
0.356, isus, .86, idec, 0.532, idec, 0.162, idec, 0.076, idec, 0.064, \
idec, 0.031, idec, 0
kamp2 linseg 0, iatt, -0.005, iatt, 0.000, iatt, 0.205, iatt, \
-0.284, iatt, -0.208, iatt, 0.326, isus, -0.401, isus, 1.540, isus, \
0.589, isus, -0.486, idec, -0.016, idec, 0.141, idec, 0.105, idec, \
-0.003, idec, -0.023, idec, 0
kamp3 linseg 0, iatt, 0.722, iatt, 1.500, iatt, 3.697, iatt, \
0.080, iatt, -2.327, iatt, -0.684, isus, -2.638, isus, 0.000, isus, \
1.347, isus, 0.485, idec, -0.419, idec, -.700, idec, -0.278, idec, \
0.167, idec, -0.059, idec, 0
iwt1 = gi_ZakianFlute_f32
iwt2 = gi_ZakianFlute_f33
iwt3 = gi_ZakianFlute_f34
inorm = 3775
goto end
range4: ; for high range tones
kamp1 linseg 0, iatt, 0.000, iatt, 0.000, iatt, 0.211, iatt, \
0.526, iatt, 0.989, iatt, 1.216, isus, 1.727, isus, 1.881, isus, \
1.462, isus, 1.28, idec, 0.75, idec, 0.34, idec, 0.154, idec, 0.122, \
idec, 0.028, idec, 0
kamp2 linseg 0, iatt, 0.500, iatt, 0.000, iatt, 0.181, iatt, \
0.859, iatt, -0.205, iatt, -0.430, isus, -0.725, isus, -0.544, isus, \
-0.436, isus, -0.109, idec, -0.03, idec, -0.022, idec, -0.046, idec, \
-0.071, idec, -0.019, idec, 0
kamp3 linseg 0, iatt, 0.000, iatt, 1.000, iatt, 0.426, iatt, \
0.222, iatt, 0.175, iatt, -0.153, isus, 0.355, isus, 0.175, isus, \
0.16, isus, -0.246, idec, -0.045, idec, -0.072, idec, 0.057, idec, \
-0.024, idec, 0.002, idec, 0
iwt1 = gi_ZakianFlute_f35
iwt2 = gi_ZakianFlute_f36
iwt3 = gi_ZakianFlute_f37
inorm = 4909.05
goto end
end:
kampr1 randi .02*kamp1, 10, gi_ZakianFLute_seed ; up to 2% wavetable amplitude variation
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
kamp1 = kamp1 + kampr1
kampr2 randi .02*kamp2, 10, gi_ZakianFLute_seed ; up to 2% wavetable amplitude variation
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
kamp2 = kamp2 + kampr2
kampr3 randi .02*kamp3, 10, gi_ZakianFLute_seed ; up to 2% wavetable amplitude variation
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
kamp3 = kamp3 + kampr3
awt1 poscil kamp1, kfreq, iwt1, iphase ; wavetable lookup
awt2 poscil kamp2, kfreq, iwt2, iphase
awt3 poscil kamp3, kfreq, iwt3, iphase
asig = awt1 + awt2 + awt3
asig = asig*(iampscale/inorm)
kcut linseg 0, iattack, ifiltcut, isustain, ifiltcut, idecay, 0 ; lowpass filter for brightness control
afilt tone asig, kcut
a_signal balance afilt, asig
i_attack = .002
i_sustain = p3
i_release = 0.01
xtratim i_attack + i_release
a_declicking linseg 0, i_attack, 1, i_sustain, 1, i_release, 0
a_signal = a_signal * i_amplitude * a_declicking * k_gain

#ifdef USE_SPATIALIZATION
a_spatial_reverb_send init 0
a_bsignal[] init 16
a_bsignal, a_spatial_reverb_send Spatialize a_signal, k_space_front_to_back, k_space_left_to_right, k_space_bottom_to_top
outletv "outbformat", a_bsignal
outleta "out", a_spatial_reverb_send
#else
a_signal *= .7
a_out_left, a_out_right pan2 a_signal, k_space_left_to_right
outleta "outleft", a_out_left
outleta "outright", a_out_right
#endif
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin
connect "ZakianFlute", "outleft",  "Mverb2020", "inleft"
connect "ZakianFlute", "outright", "Mverb2020", "inright"
gk_ZakianFlute_midi_dynamic_range init 20
gk_ZakianFlute_level init 0
maxalloc "ZakianFlute", gi_maximum_voices
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
gk_PianoNotePianoteq_midi_dynamic_range chnexport "gk_PianoNotePianoteq_midi_dynamic_range", 3 ;  20

gk_PianoNotePianoteq_midi_dynamic_range init 20

instr PianoNotePianoteq
if p3 == -1 then
  p3 = 1000000
endif
i_instrument = p1
i_time = p2
i_duration = p3
i_midi_key = p4
i_midi_dynamic_range = i(gk_PianoNotePianoteq_midi_dynamic_range)
i_midi_velocity = p5 * i_midi_dynamic_range / 127 + (63.6 - i_midi_dynamic_range / 2)
k_space_front_to_back = p6
k_space_left_to_right = p7
k_space_bottom_to_top = p8
i_phase = p9
i_instrument = p1
i_time = p2
i_duration = p3
i_midi_key = p4
i_midi_velocity = p5
i_homogeneity = p11
instances active p1
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
i_pitch_correction = 44100 / sr
; prints "Pitch factor:   %9.4f\n", i_pitch_correction
vstnote gi_Pianoteq, 0, i_midi_key, i_midi_velocity, i_duration
endin
maxalloc "PianoNotePianoteq", gi_maximum_voices
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
gk_CosineGrain_level chnexport "gk_CosineGrain_level", 3
gk_CosineGrain_midi_dynamic_range chnexport "gk_CosineGrain_midi_dynamic_range", 3
instr CosineGrain
i_instrument = p1
i_time = p2
i_duration = p3
i_midi_key = p4
i_midi_dynamic_range = i(gk_CosineGrain_midi_dynamic_range)
i_midi_velocity = p5 * i_midi_dynamic_range / 127 + (63.5 - i_midi_dynamic_range / 2)
k_space_front_to_back = p6
k_space_left_to_right = p7
k_space_bottom_to_top = p8
i_phase = p9
i_frequency = cpsmidinn(i_midi_key)
; Adjust the following value until "overall amps" at the end of performance is about -6 dB.
i_level_correction = 90
i_normalization = ampdb(-i_level_correction) / 2
i_amplitude = ampdb(i_midi_velocity) * i_normalization
k_gain = ampdb(gk_CosineGrain_level)

; Grain inputs.
i_center_time_seconds init i_time
i_duration_seconds init i_duration
i_frequency_hz init i_frequency
i_amplitude init i_amplitude
i_phase_offset_radians init 0
i_synchronous_phase init 1
a_signal clang_invoke "cosine_grain_factory", 3, i_center_time_seconds, i_duration_seconds, i_frequency_hz, i_amplitude, i_phase_offset_radians, i_synchronous_phase 
a_signal = a_signal * k_gain
a_out_left, a_out_right pan2 a_signal, k_space_left_to_right
; printks "a_signal: %9.4f a_out_left: %9.4f a_out_right: %9.4f\\n", 0, k(a_signal), k(a_out_left), k(a_out_right)
outleta "outleft", a_out_left
outleta "outright", a_out_right
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin
connect "CosineGrain", "outleft",  "Mverb2020", "inleft"
connect "CosineGrain", "outright", "Mverb2020", "inright"
gk_CosineGrain_level init 0
gk_CosineGrain_midi_dynamic_range init 20
maxalloc "CosineGrain", gi_maximum_voices
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
gk_JonesParksGrain_level chnexport "gk_JonesParksGrain_level", 3
gk_JonesParksGrain_midi_dynamic_range chnexport "gk_JonesParksGrain_midi_dynamic_range", 3
instr JonesParksGrain
i_instrument = p1
i_time = p2
i_duration = p3
i_midi_key = p4
i_midi_dynamic_range = i(gk_JonesParksGrain_midi_dynamic_range)
i_midi_velocity = p5 * i_midi_dynamic_range / 127 + (63.5 - i_midi_dynamic_range / 2)
k_space_front_to_back = p6
k_space_left_to_right = p7
k_space_bottom_to_top = p8
i_phase = p9
i_frequency = cpsmidinn(i_midi_key)
; Adjust the following value until "overall amps" at the end of performance is about -6 dB.
i_level_correction = 65
i_normalization = ampdb(-i_level_correction) / 2
i_amplitude = ampdb(i_midi_velocity) * i_normalization
k_gain = ampdb(gk_JonesParksGrain_level)

; Grain inputs.
i_center_time_seconds init i_time
i_duration_seconds init i_duration
i_starting_frequency_hz init i_frequency
i_center_frequency_hz init i_frequency
i_center_amplitude init i_amplitude
i_center_phase_offset_radians init 0
i_synchronous_phase init 1
a_signal clang_invoke "grain_factory", 3, i_center_time_seconds, i_duration_seconds, i_starting_frequency_hz, i_center_frequency_hz, i_center_amplitude, i_center_phase_offset_radians, i_synchronous_phase 
a_signal = a_signal * k_gain
a_out_left, a_out_right pan2 a_signal, k_space_left_to_right
; printks "a_signal: %9.4f a_out_left: %9.4f a_out_right: %9.4f\\n", 0, k(a_signal), k(a_out_left), k(a_out_right)
outleta "outleft", a_out_left
outleta "outright", a_out_right
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin
connect "JonesParksGrain", "outleft",  "Mverb2020", "inleft"
connect "JonesParksGrain", "outright", "Mverb2020", "inright"
gk_JonesParksGrain_level init 0
gk_JonesParksGrain_midi_dynamic_range init 20
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// This must be initialized in the orc header before any #includes.

gi_Pianoteq vstinit "/home/mkg/Pianoteq\ 7/x86-64bit/Pianoteq\ 7.so", 0
vstinfo gi_Pianoteq 

gk_PianoOutPianoteq_level chnexport "gk_PianoOutPianoteq_level", 3 ;  0
gi_PianoOutPianoteq_print chnexport "gi_PianoOutPianoteq_print", 3 ;  1
gk_PianoOutPianoteq_front_to_back chnexport "gk_PianoOutPianoteq_front_to_back", 3 ;  0
gk_PianoOutPianoteq_left_to_right chnexport "gk_PianoOutPianoteq_left_to_right", 3 ;  0.5
gk_PianoOutPianoteq_bottom_to_top chnexport "gk_PianoOutPianoteq_bottom_to_top", 3 ;  0

gk_PianoOutPianoteq_level init 0
gi_PianoOutPianoteq_print init 1
gk_PianoOutPianoteq_front_to_back init 0
gk_PianoOutPianoteq_left_to_right init 0.5
gk_PianoOutPianoteq_bottom_to_top init 0

instr PianoOutPianoteq
; Should be "D4 Daily Practice".
vstprogset gi_Pianoteq, 0
; Sustain off.
vstparamset gi_Pianoteq, 0, 0
; Reverb off.
vstparamset gi_Pianoteq, 72, 0
k_gain = ampdb(gk_PianoOutPianoteq_level)
i_overall_amps = 87
i_normalization = ampdb(-i_overall_amps) * 2
i_amplitude = ampdb(80) * i_normalization
if gi_PianoOutPianoteq_print == 1 then
  vstinfo gi_PianoOutPianoteq_print
endif
i_instrument = p1
i_time = p2
i_duration = p3
i_midi_key = p4
i_midi_velocity = p5
ainleft init 0
ainright init 0
aoutleft, aoutright vstaudio gi_Pianoteq, ainleft, ainright
a_signal = aoutleft + aoutright
a_signal *= k_gain
a_signal *= i_amplitude
a_out_left, a_out_right pan2 a_signal, gk_PianoOutPianoteq_left_to_right
; printks "vstaudio:       %9.4f   %9.4f\n", 0.5, aoutleft, aoutright

#ifdef USE_SPATIALIZATION
a_signal = a_out_left + a_out_right
a_spatial_reverb_send init 0
a_bsignal[] init 16
a_bsignal, a_spatial_reverb_send Spatialize a_signal, gk_PianoOutPianoteq_front_to_back, gk_PianoOutPianoteq_left_to_right, gk_PianoOutPianoteq_bottom_to_top
outletv "outbformat", a_bsignal
outleta "out", a_spatial_reverb_send
#else
; printks "PianoOutPt     L %9.4f R %9.4f l %9.4f\\n", 0.5, a_out_left, a_out_right, gk_Piano_level
outleta "outleft", a_out_left
outleta "outright", a_out_right
#endif
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin
connect "PianoOutPianoteq", "outleft",  "Mverb2020", "inleft"
connect "PianoOutPianoteq", "outright", "Mverb2020", "inright"
gk_PianoOutPianoteq_level init -25
alwayson "PianoOutPianoteq"
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
; Use with, e.g.: gi_Mverb2020 vstinit "/home/mkg/.local/lib/Mverb2020.so", 1
; Obtain from: http://socalabs.com/effects/mverb2020/

gk_Mverb2020_level chnexport "gk_Mverb2020_level", 3 ; 0
gk_Mverb2020_Mix chnexport "gk_Mverb2020_Mix", 3 ; .5
gk_Mverb2020_Pre_delay chnexport "gk_Mverb2020_Pre_delay", 3 ; 0.5
gk_Mverb2020_Early_late_mix chnexport "gk_Mverb2020_Early_late_mix", 3 ; 0.5
gk_Mverb2020_Size chnexport "gk_Mverb2020_Size", 3 ; 0.5
gk_Mverb2020_Density chnexport "gk_Mverb2020_Density", 3 ; 0.5
gk_Mverb2020_Bandwith_Frequency chnexport "gk_Mverb2020_Bandwith_Frequency", 3 ; 0.5
gk_Mverb2020_Decay chnexport "gk_Mverb2020_Decay", 3 ; 0.85
gk_Mverb2020_Damping_Frequency chnexport "gk_Mverb2020_Damping_Frequency", 3 ; 0.5
gk_Mverb2020_Gain chnexport "gk_Mverb2020_Gain", 3 ; 1
gi_Mverb2020_Program chnexport "gi_Mverb2020_Program", 3 ; 4

gk_Mverb2020_level init 0
gk_Mverb2020_Mix init .5
gk_Mverb2020_Pre_delay init 0.5
gk_Mverb2020_Early_late_mix init 0.5
gk_Mverb2020_Size init 0.5
gk_Mverb2020_Density init 0.5
gk_Mverb2020_Bandwith_Frequency init 0.5
gk_Mverb2020_Decay init 0.85
gk_Mverb2020_Damping_Frequency init 0.5
gk_Mverb2020_Gain init 1
gi_Mverb2020_Program init 4

instr Mverb2020
vstprogset gi_Mverb2020, gi_Mverb2020_Program
vstparamset gi_Mverb2020, 1, gk_Mverb2020_Mix
;vstparamset gi_Mverb2020, 1, gk_Mverb2020_Pre_delay
;vstparamset gi_Mverb2020, 2, gk_Mverb2020_Early_late_mix
;vstparamset gi_Mverb2020, 3, gk_Mverb2020_Size
;vstparamset gi_Mverb2020, 4, gk_Mverb2020_Density
;vstparamset gi_Mverb2020, 5, gk_Mverb2020_Bandwith_Frequency
vstparamset gi_Mverb2020, 6, gk_Mverb2020_Decay
;vstparamset gi_Mverb2020, 7, gk_Mverb2020_Damping_Frequency
;vstparamset gi_Mverb2020, 8, gk_Mverb2020_Gain
k_gain = ampdb(gk_Mverb2020_level)
ainleft inleta "inleft"
ainright inleta "inright"
aoutleft, aoutright vstaudio gi_Mverb2020, ainleft, ainright
outleta "outleft", aoutleft
outleta "outright", aoutright
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f -- %3d\\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin
connect "Mverb2020", "outleft",  "MasterOutput", "inleft"
connect "Mverb2020", "outright", "MasterOutput", "inright"
gk_Mverb2020_level init 0
gk_Mverb2020_Mix init .5
gk_Mverb2020_Pre_delay init 0.5
gk_Mverb2020_Early_late_mix init 0.5
gk_Mverb2020_Size init 0.5
gk_Mverb2020_Density init 0.5
gk_Mverb2020_Bandwith_Frequency init 0.5
gk_Mverb2020_Decay init 0.85
gk_Mverb2020_Damping_Frequency init 0.5
gk_Mverb2020_Gain init 1
gi_Mverb2020_Program init 2
alwayson "Mverb2020"
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
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
if i_filename_length > 0 goto filename_exists
goto filename_endif
filename_exists:
prints sprintf("Output filename: %s\n", gS_MasterOutput_filename)
fout gS_MasterOutput_filename, 18, aleft * i_amplitude_adjustment, aright * i_amplitude_adjustment
filename_endif:
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin
gk_MasterOutput_level init 10
gS_MasterOutput_filename init ""
alwayson "MasterOutput"
//////////////////////////////////////////////////////////////////////////////

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
    <title>Poustinia version 9</title>
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
</head>
<body style="background-color:black;box-sizing:border-box;padding:10px;:fullscreen">
    <script>
        csound_message = function(message) {
            if (typeof webkit_initialized == "undefined" || webkit_initialized === null) {
                return;
            }
            let messages_textarea = document.getElementById("console");
            if (typeof messages_textarea == "undefined" || messages_textarea === null) {
                return;
            }
            let existing = messages_textarea.value;
            messages_textarea.value = existing + message;
            messages_textarea.scrollTop = messages_textarea.scrollHeight;
        };
    </script>
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
    <textarea id="console" style="position:absolute;left:1vw;top:1vw;width:50vw;height:98vh;color:SkyBlue;background-color:transparent;border:none;text-align:left;font-size:8.25px;overflow:auto;cursor:none;"></textarea>
    <script>
        csound = new Csound("http://localhost:8383");
        var lsys = new ChordSpace.LSys();
        var page = window.location.pathname.split("/").pop();
        console.log("Current file: " + page);
        var title = page;
        window.onerror = function (message, url, line) {
            csound.Message(message + '\\n' + url + '\\n' + line + '\\n');
            console.trace();
        }
        var generate = function() {
            csound.Message("generate()...\\n");
            try {
                lsys.axiom = "- [ g f f I I I I a ] g a";
                lsys.addRule('g', 'Ja b a3 c aa F F c2 a2 t,/,1.025 b2 K c2 g b2 t,*,1.025 T,5 F Jb F F c g a b g');
                lsys.addRule('a', 'F b F a2 b a2 [ b3 F F f F [ c F F F F ] ] F F F Jb c2 a2 c F c F F F b2 Ja a2');
                lsys.addRule('aa', '[ b3 F F F c3 f F F F F ] F b F a2 b I I I F F [ c2 F F F F F F ] F c2 a2 c F c F F b2 a2');
                lsys.addRule('xaa', '[ b3 F F F c3 f F F F F ] F b F a2 b I I F F [ c2 F F F F F F ] F c2 a2 c F c F F b2 a2');
                lsys.addRule('xa2', 'I F b F b a3 i i b2 f F F f c2 a3 c F c2 F b2 a3 i i c');
                lsys.addRule('a3', 'F b2 b F a I v F F c F F F F F c2 aa V f c2 F F b2 a');
                lsys.addRule('xa3', 'F b2 b F a I v F F c F F F F F c2 aa i i V f c2 F F b2 a');
                lsys.addRule('Ja', 'J,2,0');
                lsys.addRule('Jb', 'J,2,2'); 
                ///lsys.addRule('b', '+ f f f f f - -'); 
                ///lsys.addRule('c', '- f f f f f +');
                lsys.addRule('b', '+ f f f f F f - -'); 
                lsys.addRule('c', '- f f F f f f +');
                lsys.addRule('b2', '+ f f f f -'); 
                ///lsys.addRule('c2', '- - f f f f + +');
                lsys.addRule('c2', '- - f f f f + +');
                lsys.addRule('b3', '- - f f f + +'); 
                ///lsys.addRule('b3', '- - f f f f + +'); 
                var chord = ChordSpace.chordForName('CM9'); 
                var t = new ChordSpace.Turtle(4.5, Math.PI/2, chord, chord);
                lsys.generate(4);
                lsys.write_score(t);
                csound.Message("Finding size of score...\\n");
                lsys.score.findScales();
                csound.Message("Rescaling score...\\n");
                lsys.score.setScale(3, 1, 8.125);
                //lsys.score.setScale(3, 3, 0);
                lsys.score.setScale(4, 24, 72);
                ///lsys.score.setScale(5, 60, 8);
                ///lsys.score.setScale(5, 60, 9);
                lsys.score.setScale(5, 60, 11);
                lsys.score.setScale(7, 0, 1);
                lsys.score.temper(12);
                csound.Message("Conforming to chords...\\n");
                lsys.conformToChords();
                lsys.score.setDuration(12*60);
                csound.Message("Tieing overlapping notes...\\n");
                lsys.score.tieOverlaps(true);
                lsys.score.sort();
                // Fix up score before rendering.
                var i;
                var n = lsys.score.data.length;
                var cutoff = 7 * 60 + 35;
                var note;
                for (i = n - 1; i >= 0; --i) {
                    note = lsys.score.data[i];
                    // Extend the notes at the end.
                    if ((note.time + note.duration) >= cutoff) {
                        note.duration = (note.duration + 2.5);
                    }
                    if (note.duration < .5) {
                        note.duration = .5;
                    }
                }
                // Pianoteq misbehaves if we start right at time 0.
                for (var i = 0; i < lsys.score.data.length; i++) {
                    var event = lsys.score.data[i];
                    event.time = event.time + 2;
                }
                if (false) {
                    // Also if keys overlap.
                    for (var i = 0; i < lsys.score.data.length; i++) {
                        var event = lsys.score.data[i];
                        csound.Message(event.toIStatement() + ' e: ' + event.end);
                        var channel = Math.floor(event.channel);
                        var key = event.key;
                        for (var j = i + 1; j < lsys.score.data.length; j++) {
                            var overlap = lsys.score.data[j];
                            var overlap_channel = Math.floor(overlap.channel);
                            var overlap_key = overlap.key;
                            if (key === overlap_key && channel === overlap_channel && channel === 2) {
                                if (event.end > overlap.time) {
                                    var the_end = Math.max(event.end, overlap.end);
                                    csound.Message('Correcting overlap for:');
                                    csound.Message(' ' + event.toIStatement() + ' e: ' + event.end);
                                    csound.Message(' ' + overlap.toIStatement() + ' e: ' + overlap.end);
                                    event.end = the_end;
                                    overlap.end = the_end;
                                    csound.Message('to:');
                                    csound.Message(' ' + event.toIStatement() + ' e: ' + event.end);
                                    csound.Message(' ' + overlap.toIStatement() + ' e: ' + overlap.end + '\\n');
                                }
                            }
                        }
                    }
                }
                // Reassign Pianoteq to FMWaterBell,
                // randomize pans
                for (var i = 0; i < lsys.score.data.length; i++) {
                    var event = lsys.score.data[i];
                    event.channel = Math.round(event.channel);
                    let random_variable = Math.random();
                    random_variable *= .6;
                    random_variable += .2;
                    event.pan = event.pan + random_variable
                    if (event.channel >= 9 && event.channel < 10) {
                      event.channel = 1;
                      csound.Message('Reassigned...\\n');
                    }
                } 
                // Shorten duration preserving corrections...
                lsys.score.setDuration(9*60);
                // Bring in final notes.
                let size = lsys.score.data.length;
                lsys.score.data[size - 1].time = (lsys.score.data[size - 1].time - 12.);
                lsys.score.data[size - 2].time = (lsys.score.data[size - 2].time - 12.);
                if (true) {
                    let start_shortening = 4*60+40;
                    let old_end_shortening = 4*60+57;
                    let old_duration = old_end_shortening - start_shortening;
                    let new_end_shortening = old_end_shortening - 18;
                    let new_duration = new_end_shortening - start_shortening;
                    let shrinkage = new_duration / old_duration;
                    for (var i = 0; i < lsys.score.data.length; i++) {
                        var event = lsys.score.data[i];
                        // Try transposing without changing interval structure.
                        event.key += 1;
                        if (event.time >= old_end_shortening) {
                            let old_time = event.time;
                            let old_end = event.end;
                            event.time = event.time - start_shortening;
                                if (old_time < old_end_shortening) {
                                    event.time = event.time * shrinkage;
                                    if (old_end < old_end_shortening) {
                                        event.duration = event.duration * shrinkage;
                                    } 
                                }
                            event.time = event.time + start_shortening;
                        } 
                    }
                }
                if (false) {
                    let start_shortening = 4*60+40;
                    ///let old_end_shortening = 4*60+48;
                    let old_end_shortening = 290;;
                    let old_duration = old_end_shortening - start_shortening;
                    let new_end_shortening = 270;
                    let new_duration = new_end_shortening - start_shortening;
                    let shrinkage = new_duration / old_duration;
                    for (var i = 0; i < lsys.score.data.length; i++) {
                        var event = lsys.score.data[i];
                        // Try transposing without changing interval structure.
                        event.key += 1;
                        if (event.time >= old_end_shortening) {
                            let old_time = event.time;
                            let old_end = event.end;
                            event.time = event.time - start_shortening;
                            if (old_time < old_end_shortening) {
                                event.time = event.time * shrinkage;
                                if (old_end < old_end_shortening) {
                                    event.duration = event.duration * shrinkage;
                                } 
                            }
                            event.time = event.time + start_shortening;
                        }
                    }
                }
                csound.Message("Plotting score...\\n");
                lsys.score.draw3D(document.getElementById("canvas"));
                csound.Message('Generated ' + lsys.score.data.length + ' notes.\\n');
                let csound_score = lsys.score.toCsoundScore();
                csound.ReadScore(csound_score);
                csound.Message("Playing generated score...\\n");
                updateScoreTime();
            } catch(err) {
                csound.Message(err.name + ': ' + err.message + ' ' + err.line + '\\n');
            }
        };
        
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
         gui = new dat.GUI({load: default_json, width: 300});
         csound.Message("Restored Csound instrument parameters.\\n");
         } catch(e) {
         csound.Message("Failed to restore Csound instrument parameters.\\n");
         gui = new dat.GUI({width: 400});
         }
         gui.remember(parameters);
         gui.add(parameters, 'generate').name('Generate [Ctrl-G]');
         gui.add(parameters, 'stop').name('Stop [Ctrl-S]');
         var Master = gui.addFolder('Master');
         add_slider(Master, 'gk_overlap', 0, 20);
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
        
        var gk_update = function(name, value) {
         var numberValue = parseFloat(value);
         csound.setControlChannel(name, numberValue);
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
        var csound = new Csound("http://localhost:8383");
        $('input').on('input', function(event) {
            var slider_value = parseFloat(event.target.value);
            csound.SetControlChannel(event.target.id, slider_value);
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
                csound.SetControlChannel(this.id, parseFloat(this.value));
                var output_selector = '#' + this.id + '_output';
                $(output_selector).val(this.value);
            });
        });
        $('#restore').click();
    });
    webkit_initialized = true;
</script>
</body>
</html>
}}

gi_browser webkit_create 8383, 0
// The following lines find the current working directory from Csound, 
// and then use that to construct the base URI of the HTML code.
S_pwd pwd
S_base_uri sprintf "file://%s/", S_pwd
prints S_base_uri
webkit_open_html gi_browser, "Iterated Function System Study No. 6", gS_html, S_base_uri, 12000, 10000, 0
endin
alwayson "Browser"
//////////////////////////////////////////////////////////////////////////////

instr Exit
exitnow 0
endin

</CsInstruments>
<CsScore>
; f 0 does not work here, we actually need to schedule an instrument that 
; turns off Csound.
i "Exit" [11 * 60 + 5]
</CsScore>
</CsoundSynthesizer>