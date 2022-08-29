# test2 testira instrukcije skokova i push i pop, shl i shr
.global myStart

.section ivt
.word isr_reset
.skip 2
.word 0x9988, 0x7766
.skip 12
isr_reset:
jmp myStart

.section atext
myStart:
ldr r1, $0x1
jmp label1
ldr r2, $0x1
label1:
ldr r3, $0x3
shl r3, r1
shr r2, r1
push r3
pop r4
xchg r3, r2

.end
