<CsoundSynthesizer>
<CsLicense>
Bare test of the Csound webserver opcodes.
</CsLicense>
<CsOptions>
</CsOptions>
-m165 -odac0
<CsInstruments>
sr = 48000
nchnls = 2
ksmps = 100
0dbfs = 2

i_webserver webserver_create "/Users/michaelgogins/csound-webserver/examples", 8080, 1 

</CsInstruments>
<CsScore>
f 0 120
</CsScore>
</CsoundSynthesizer>
