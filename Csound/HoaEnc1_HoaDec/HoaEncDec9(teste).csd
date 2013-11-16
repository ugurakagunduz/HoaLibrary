<CsoundSynthesizer>
<CsOptions>
</CsOptions>
<CsInstruments>

/** XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX **/
/** UDO HoaEnc3 - ordre 1 **/

	opcode HoaEnc1, aaa, ak
	
ain, kangle xin 

setksmps 1


ah0 init 0
ah0  = ain * cos(0*kangle)

ahn1 init 0
ahn1 = ain * sin(1*kangle)

ahn init 0
ah1  = ain * cos(1*kangle)


xout ah0, ahn1, ah1

	endop

/** UDO HoaEnc1 - ordre 1 **/
/** XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX **/
/** UDO HoaDec1 - ordre 1 **/

	opcode HoaDec1, aaaa, aaa

aX, aY, aZ xin

asig1 = 0.5
asig2 = 0.25

aA, aB, aC HoaEnc1 asig1, 0
aD, aE, aF HoaEnc1 asig1, 1.570796
aG, aH, aI HoaEnc1 asig1, 3.141592
aJ, aK, aL HoaEnc1 asig1, 4.712388

a1 = asig2 + aB*aY + aC*aZ
a2 = asig2 + aE*aY + aF*aZ
a3 = asig2 + aH*aY + aI*aZ
a4 = asig2 + aK*aY + aL*aZ

xout a1, a2, a3, a4 

	endop

/** UDO HoaDec1 - ordre 1 **/
/** XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX **/


instr 1

kangle line 0, p3, 6.283185

ain oscil 1000, 400, 1

aM, aN, aO HoaEnc1 ain, kangle

a1, a2, a3, a4 HoaDec1 aM, aN, aO

outq a1, a2, a3, a4

endin

</CsInstruments>
<CsScore>
f1 0 16384 10 1


i1 0 1
i1 + .
i1 + .
i1 + .
i1 + .
i1 + .
i1 + .
i1 + .
i1 + .
i1 + .
i1 + .
i1 + .
i1 + .
i1 + .

e

</CsScore>
</CsoundSynthesizer>
