


struc fibers_ctx
    .ip: resq 1
    .sp: resq 1
    .rbx: resq 1
    .rbp: resq 1
    .r12: resq 1
    .r13: resq 1
    .r14: resq 1
    .r15: resq 1
    .mxcsr: resq 1
    .x86_fcw: resq 1
endstruc

global fibers_switch
fibers_switch:
    ; SAVING 
    mov r8, [rsp]
    mov [rdi + fibers_ctx.ip] , r8 ; from->sp = rsp

    lea r8, [rsp+8] ; r8 = rsp + 8 (return address)
    mov [rdi+ fibers_ctx.sp], r8 ; from->sp = rsp + 8


    mov [rdi + fibers_ctx.rbx]  , rbx ; from->rbx = rbx
    mov [rdi + fibers_ctx.rbp]  , rbp ; from->rbp = rbp
    mov [rdi + fibers_ctx.r12]  , r12 ; from->r12 = r12 
    mov [rdi + fibers_ctx.r13]  , r13 ; from->r13 = r13
    mov [rdi + fibers_ctx.r14]  , r14 ; from->r14 = r14 
    mov [rdi + fibers_ctx.r15]  , r15 ; from->r15 = r15 

    stmxcsr [rdi + fibers_ctx.mxcsr] ; from->mxcsr = mxcsr
    fnstcw  [rdi + fibers_ctx.x86_fcw] ; from->x86_fcw = x86_fcw

    ; LOADING
    mov rsp, [rsi + fibers_ctx.sp] ; rsp = to->sp

    mov rbx, [rsi + fibers_ctx.rbx] ; rbx = to->rbx
    mov rbp, [rsi + fibers_ctx.rbp] ; rbp = to->rbp
    mov r12, [rsi + fibers_ctx.r12] ; r12 = to->r12
    mov r13, [rsi + fibers_ctx.r13] ; r13 = to->r13 
    mov r14, [rsi + fibers_ctx.r14] ; r14 = to->r14 
    mov r15, [rsi + fibers_ctx.r15] ; r15 = to->r15

    ldmxcsr  [rsi + fibers_ctx.mxcsr] ; mxcsr = to->mxcsr
    fldcw    [rsi + fibers_ctx.x86_fcw] ; x86_fcw = to->x86_fcw

    mov r8,  [rsi + fibers_ctx.ip] ; r8 = to->ip
    push r8 ; stack[] = to->ip 

    ret ; ip = stack[]