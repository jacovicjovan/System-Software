.global myStart

.section ivt
.word isr_reset
.skip 16
isr_reset:
jmp myStart

.section text
myStart:
ldr r0, $0x1
jmp %lab1
ldr r2, $0x3
lab1:
ldr r1, $0x2
jmp %lab2
halt

.section text2
lab2:
ldr r3, $0x3
halt

.end