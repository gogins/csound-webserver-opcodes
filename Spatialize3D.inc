gShelp init {{
S P A T I A L I Z E   3 D

Copyright (c) 2014 by Michael Gogins.
This software is licensed under the terms of the
GNU Lesser Public License.

Many thanks to Jan Jacob Hofmann for his advanced Csound
spatialization system (http://www.sonicarchitecture.de/), 
which inspired this work.

The available apeaker rigs are:

 0 Ambisonic stereo with minimal distance cues
 (possibly the best choice for efficiency in live performance).
 1 Ambisonic binaural with HRTF decoding (periphonic, a suitable choice
   for checking in stereophonic studio work).
 2 Ambisonic quadraphonic.
 3 Ambisonic pentaphonic or 5.0 (might work for 5.1, omit .1)
 4 Ambisonic octaphonic.
 5 Ambisonic cubic (periphonic, probably the best choice for concerts).

Please note the coordinate conventions. The listener is always the origin.
In this system, spatial locations are passed to the spatialize opcode in
Cartesian coordinates. These are the same as those used in physics:

-- x runs from ahead to behind.
-- y runs from left to right. This is the same as stereo pan.
-- z runs from below to above.

Spherical coordinates are more or less standard for Ambisonics, and are
similar to but not identical with those used in physics. The only differences
are that Ambisonics uses elevation but physics uses inclination, and
Ambisonics uses degrees but physics uses radians.

-- r is distance.
-- azimuth or theta is anti-clockwise angle from 0 in front of the listener.
-- elevation or phi is angle above or below the x-y plane. (Note that
   inclination, also called polar angle, would be angle away from the
   zenith pole.)


; Global parameters that can be re-defined in the orchestra header.

gk_Spatialize_Verbose chnexport "gk_Spatialize_Verbose", 3 ;  0
gk_BformatDecoder_SpeakerRig chnexport "gk_BformatDecoder_SpeakerRig", 3 ;  5
gk_Spatialize_SpeakerRigRadius chnexport "gk_Spatialize_SpeakerRigRadius", 3 ;  5.

gk_Spatialize_Verbose init 0
gk_BformatDecoder_SpeakerRig init 5
gk_Spatialize_SpeakerRigRadius init 5.0


opcode distance, k, kkk
kx, ky, kz xin
; Calculate the distance from the origin (the listener).
kdistance = sqrt(kx^2 + ky^2 + kz^2)
xout kdistance
endop

opcode CartesianToPolar, kkk, kkk
kx, ky, kz xin
; Calculate the distance from the origin (the listener).
kdistance = sqrt(kx^2 + ky^2 + kz^2)
; Normalize the coordinates to unit vectors from the origin.
kx = kx / kdistance
ky = ky / kdistance
kz = kz / kdistance
; Limit distance to prevent too close a sound becoming too loud.
kdistance = (kdistance < 0.3 ? 0.3 : kdistance)
; Calculate the elevation.
kelevation = cosinv(sqrt(1 - kz^2))
; If z is negative, make elevation negative also.
kelevation = (kz < 0 ? -kelevation : kelevation)
; Calculate the angle.
kangle = sininv(ky / cos(kelevation))
; Distinguish between positive x and negative x.
kangle = (kx >= 0 ? kangle : 3.14159265 - kangle)
; Distinguish between positive and negative y and x.
kangle = (ky <= 0 && kx >= 0 ? 6.28318531 + kangle : kangle)
xout kangle, kelevation, kdistance
endop

opcode CartesianToPolarDegrees, kkk, kkk
kx, ky, kz xin
; Calculate the distance from the origin (the listener).
kdistance = sqrt(kx^2 + ky^2 + kz^2)
; Normalize the coordinates to unit vectors from the origin.
kx = kx / kdistance
ky = ky / kdistance
kz = kz / kdistance
; Limit distance to prevent too close a sound becoming too loud.
kdistance = (kdistance < 0.3 ? 0.3 : kdistance)
; Calculate the elevation.
kelevation = cosinv(sqrt(1 - kz^2))
; If z is negative, make elevation negative also.
kelevation = (kz < 0 ? -kelevation : kelevation)
; Calculate the angle.
kangle = sininv(ky / cos(kelevation))
; Distinguish between positive x and negative x.
kangle = (kx >= 0 ? kangle : 3.14159265 - kangle)
; Distinguish between positive and negative y and x.
kangle = (ky <= 0 && kx >= 0 ? 6.28318531 + kangle : kangle)
kangle = kangle * 57.295779513
kelevation = kelevation * 57.295779513
xout kangle, kelevation, kdistance
endop

; Zero distance blows this up.
gk_DopplerByDistance_Mach init 340.29
opcode DopplerByDistance, a, ak
asignal, kdistance xin
; ashifted doppler asource, ksourceposition, kmicposition [, isoundspeed, ifiltercutoff]
adopplered doppler asignal, kdistance, 0, i(gk_DopplerByDistance_Mach), 6
xout adopplered
endop

; Zero distance blows this up.
opcode LowpassByDistance, a, ak
asignal, kdistance xin
afiltered butterlp asignal, 22000 * sqrt(1.0 / kdistance) + .000001
abalanced balance afiltered, asignal
xout abalanced
endop

opcode ReverbSC, a, akkkk
asignal, kReverbFeedback, kDelayModulation, kReverbWet, itone xin
; p4 = gain of reverb. Adjust empirically
; for desired reverb time. .6 gives
; a good small "live" room sound, .8
; a small hall, .9 a large hall,
; .99 an enormous stone cavern.

; p5 = amount of random pitch modulation
; for the delay lines. 1 is the "normal"
; amount, but this may be too high for
; held pitches such as piano tones.
; Adjust to taste.

; p6 = cutoff frequency of lowpass filters
; in feedback loops of delay lines,
; in Hz. Lower cutoff frequencies results
; in a sound with more high-frequency
; damping.

; 8 delay line FDN reverb, with feedback matrix based upon
; physical modeling scattering junction of 8 lossless waveguides
; of equal characteristic impedance. Based on Julius O. Smith III,
; "A New Approach to Digital Reverberation using Closed Waveguide
; Networks," Proceedings of the International Computer Music
; Conference 1985, p. 47-53 (also available as a seperate
; publication from CCRMA), as well as some more recent papers by
; Smith and others.
; Coded by Sean Costello, October 1999
afilt1 init 0
afilt2 init 0
afilt3 init 0
afilt4 init 0
afilt5 init 0
afilt6 init 0
afilt7 init 0
afilt8 init 0
idel1 = (2473.000/sr)
idel2 = (2767.000/sr)
idel3 = (3217.000/sr)
idel4 = (3557.000/sr)
idel5 = (3907.000/sr)
idel6 = (4127.000/sr)
idel7 = (2143.000/sr)
idel8 = (1933.000/sr)
; k1-k8 are used to add random pitch modulation to the
; delay lines. Helps eliminate metallic overtones
; in the reverb sound.
ak1 randi .001, 3.1, .06
ak2 randi .0011, 3.5, .9
ak3 randi .0017, 1.11, .7
ak4 randi .0006, 3.973, .3
ak5 randi .001, 2.341, .63
ak6 randi .0011, 1.897, .7
ak7 randi .0017, 0.891, .9
ak8 randi .0006, 3.221, .44
; apj is used to calculate "resultant junction pressure" for
; the scattering junction of 8 lossless waveguides
; of equal characteristic impedance. If you wish to
; add more delay lines, simply add them to the following
; equation, and replace the .25 by 2/N, where N is the
; number of delay lines.
apj = .25 * (afilt1 + afilt2 + afilt3 + afilt4 + afilt5 + afilt6 + afilt7 + afilt8)
adum1 delayr 1
adel1 deltapi idel1 + ak1 * kDelayModulation
 delayw asignal + apj - afilt1
adum2 delayr 1
adel2 deltapi idel2 + ak2 * kDelayModulation
 delayw asignal + apj - afilt2
adum3 delayr 1
adel3 deltapi idel3 + ak3 * kDelayModulation
 delayw asignal + apj - afilt3
adum4 delayr 1
adel4 deltapi idel4 + ak4 * kDelayModulation
 delayw asignal + apj - afilt4
adum5 delayr 1
adel5 deltapi idel5 + ak5 * kDelayModulation
 delayw asignal + apj - afilt5
adum6 delayr 1
adel6 deltapi idel6 + ak6 * kDelayModulation
 delayw asignal + apj - afilt6
adum7 delayr 1
adel7 deltapi idel7 + ak7 * kDelayModulation
 delayw asignal + apj - afilt7
adum8 delayr 1
adel8 deltapi idel8 + ak8 * kDelayModulation
 delayw asignal + apj - afilt8
; 1st order lowpass filters in feedback
; loops of delay lines.
afilt1 tone adel1 * kReverbFeedback, itone
afilt2 tone adel2 * kReverbFeedback, itone
afilt3 tone adel3 * kReverbFeedback, itone
afilt4 tone adel4 * kReverbFeedback, itone
afilt5 tone adel5 * kReverbFeedback, itone
afilt6 tone adel6 * kReverbFeedback, itone
afilt7 tone adel7 * kReverbFeedback, itone
afilt8 tone adel8 * kReverbFeedback, itone
; The outputs of the delay lines are summed
; and sent to the stereo outputs. This could
; easily be modified for a 4 or 8-channel
; sound system.
aout1 = (afilt1 + afilt3 + afilt5 + afilt7)
aout2 = (afilt2 + afilt4 + afilt6 + afilt8)
aoutput = aout1 + aout2
aoutwet = aoutput * kReverbWet
aoutdry = aoutput * (1 - kReverbWet)
aoutput = aoutwet + aoutdry
xout aoutput
endop

/**
 * Spatializes a one channel audio signal to 3 dimensions using Ambisonic 
 * panning with movement, Doppler effects, and distance cues. The output is 
 * sent directly to Csound's audio output.
 *
 * Spatialize3D a_signal, k_x, k_y, k_z
 *
 * Much of the configuration for this opcode is of necessity contained in 
 * global variables, so that instances of the panner can share room data, etc.
 */
 
gi_Spatialize3D_room_table ftgen
gi_Spatialize3D_unit_circle_radius init 1
gi_Spatialize3D_oversampling init 2
/**
 *  1. Stereo -- L(90), R(-90).
 *  2. Quad -- FL(45), BL(135), BR(-135), FR(-45). This is a first-order 
 *     decode.
 *  3. 5.0 -- L(30), R(-30), C(0), BL(110), BR(-110). This is the 
 *     configuration suggested by the International Telecommunication Union 
 *     for 5.1 surround systems for music listening. This is a first- or 
 *     second- order decode.
 *  4. Octagon -- FFL(22.5), FLL(67.5), BLL(112.5), BBL(157.5), BBR(-157.5), 
 *     BRR(-112.5), FRR(-67.5), FFR(-22.5). This is a first-, second- or 
 *     third-order decode, depending on the number of input channels.
 *  5. Cube -- FLD(45,-35.26), FLU(45,35.26), BLD(135,-35.26), BLU(135,35.26), 
 *     BRD(-135,-35.26), BRU(-135,35.26), FRD(-45,-35.26), FRU(-45,35.26). 
 *     This is a first-order decode.
 *  6. Hexagon -- FL(30), L(90) BL(150), BR(-150), R(-90), FR(-30). This is a 
 *     first- or second- order decode.
 * 21. 2D binaural configuration. This first decodes to an octagon 
 *     configuration, and then applies head-related transfer function filters.
 * 31. 3D binaural configuration. This first decodes to a dodecahedron 
 *     configuration, and then applies head-related transfer function filters.
 */
gi_Spatialize3D_speaker_rig init 31


opcode Spatialize3D,,akkk
bsignal[] init 
a_signal, k_x, k_y, k_z xin
bsignal
endop

