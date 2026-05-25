    .section .text.prologue
    .globl _entry
    .globl _isr
    .globl setjmp
    .globl longjmp
    .globl __dispatch_init

    .extern main
    .extern system_init

_entry:
    /* Run only on U54_1 (hart 1). Park all others. */
    csrr    a0, mhartid
    li      t0, 1
    beq     a0, t0, .Lboot_hart1

.Lpark_other_harts:
    wfi
    j       .Lpark_other_harts

.Lboot_hart1:
    /* Set up global pointer */
    .option push
    .option norelax
    la      gp, _gp
    .option pop

    /* Set up stack */
    la      sp, _stack

    /* Set thread pointer to a harmless aligned value */
    la      tp, _end
    addi    tp, tp, 63
    andi    tp, tp, -64

    /* Clear / disable interrupts and delegation */
    csrw    mie, zero
    csrw    mip, zero
    csrw    medeleg, zero
    csrw    mideleg, zero
    csrci   mstatus, 8

    /* Install trap vector */
    la      t0, _isr
    csrw    mtvec, t0

    /* Clear .bss bytewise */
    la      a0, _sbss
    la      a1, _ebss
.Lclear_bss:
    bgeu    a0, a1, .Lprepare_data
    sb      zero, 0(a0)
    addi    a0, a0, 1
    j       .Lclear_bss

.Lprepare_data:
    /* Copy .data only if LMA != VMA */
    la      a0, _sidata
    la      a1, _sdata
    la      a2, _edata
    beq     a0, a1, .Lcall_init

.Lcopy_data:
    bgeu    a1, a2, .Lcall_init
    lbu     t1, 0(a0)
    sb      t1, 0(a1)
    addi    a0, a0, 1
    addi    a1, a1, 1
    j       .Lcopy_data

.Lcall_init:
    call    system_init
    call    main

.Lhang:
    wfi
    j       .Lhang

    .align  2
_isr:
    /* Save caller-saved context + trap metadata */
    addi    sp, sp, -160

    sd      ra,   0(sp)
    sd      t0,   8(sp)
    sd      t1,  16(sp)
    sd      t2,  24(sp)
    sd      a0,  32(sp)
    sd      a1,  40(sp)
    sd      a2,  48(sp)
    sd      a3,  56(sp)
    sd      a4,  64(sp)
    sd      a5,  72(sp)
    sd      a6,  80(sp)
    sd      a7,  88(sp)
    sd      t3,  96(sp)
    sd      t4, 104(sp)
    sd      t5, 112(sp)
    sd      t6, 120(sp)

    csrr    a0, mcause
    csrr    a1, mepc
    sd      a0, 128(sp)
    sd      a1, 136(sp)

    call    _irq_handler

    /* Allow handler to modify return PC if desired */
    ld      a1, 136(sp)
    csrw    mepc, a1

    ld      ra,   0(sp)
    ld      t0,   8(sp)
    ld      t1,  16(sp)
    ld      t2,  24(sp)
    ld      a0,  32(sp)
    ld      a1,  40(sp)
    ld      a2,  48(sp)
    ld      a3,  56(sp)
    ld      a4,  64(sp)
    ld      a5,  72(sp)
    ld      a6,  80(sp)
    ld      a7,  88(sp)
    ld      t3,  96(sp)
    ld      t4, 104(sp)
    ld      t5, 112(sp)
    ld      t6, 120(sp)

    addi    sp, sp, 160
    mret

setjmp:
    sd      s0,   0(a0)
    sd      s1,   8(a0)
    sd      s2,  16(a0)
    sd      s3,  24(a0)
    sd      s4,  32(a0)
    sd      s5,  40(a0)
    sd      s6,  48(a0)
    sd      s7,  56(a0)
    sd      s8,  64(a0)
    sd      s9,  72(a0)
    sd      s10, 80(a0)
    sd      s11, 88(a0)
    sd      gp,  96(a0)
    sd      tp, 104(a0)
    sd      sp, 112(a0)
    sd      ra, 120(a0)
    li      a0, 0
    ret

longjmp:
    ld      s0,   0(a0)
    ld      s1,   8(a0)
    ld      s2,  16(a0)
    ld      s3,  24(a0)
    ld      s4,  32(a0)
    ld      s5,  40(a0)
    ld      s6,  48(a0)
    ld      s7,  56(a0)
    ld      s8,  64(a0)
    ld      s9,  72(a0)
    ld      s10, 80(a0)
    ld      s11, 88(a0)
    ld      gp,  96(a0)
    ld      tp, 104(a0)
    ld      sp, 112(a0)
    ld      ra, 120(a0)
    mv      a0, a1
    ret

__dispatch_init:
    ld      s0,   0(a0)
    ld      s1,   8(a0)
    ld      s2,  16(a0)
    ld      s3,  24(a0)
    ld      s4,  32(a0)
    ld      s5,  40(a0)
    ld      s6,  48(a0)
    ld      s7,  56(a0)
    ld      s8,  64(a0)
    ld      s9,  72(a0)
    ld      s10, 80(a0)
    ld      s11, 88(a0)
    ld      gp,  96(a0)
    ld      tp, 104(a0)
    ld      sp, 112(a0)
    ld      ra, 120(a0)

    /* Global interrupt enable; mie remains zero unless configured later */
    csrsi   mstatus, 8
    ret
