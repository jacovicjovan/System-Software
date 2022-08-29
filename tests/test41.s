.extern a

.section ivt
.word isr_reset
.skip 2
.word 0x9988, 0x7766
.skip 12
isr_reset:
jmp myStart

.section text
myStart:
ldr r0, a
halt

.end