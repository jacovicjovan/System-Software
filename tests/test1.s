# test1 testira aritmeticke i logicke instrukcije, kao i load i store
.global myStart

.section ivt
.word isr_reset
.skip 2
.word 0x9988, 0x7766
.skip 12
isr_reset:
jmp myStart

.section .text
myStart:
ldr r0, $0x1
ldr r1, $0x7
ldr r2, $0x3
ldr r4, $0xc
add r1, r0
not r0
or r2, r4
ldr r5, a
add r5, r4
str r5, a
ldr r0, $a
ldr r3, r1
halt

.data
a:
.word 0xaabb

.end