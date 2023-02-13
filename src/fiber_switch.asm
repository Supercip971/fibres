


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
    mov [rdi] , r8 ; from->sp = rsp

    lea r8, [rsp+8] ; r8 = rsp + 8 (return address)
    mov [rdi+8], r8 ; from->sp = rsp + 8


    mov [rdi+16], rbx ; from->rbx = rbx
    mov [rdi+24], rbp ; from->rbp = rbp
    mov [rdi+32], r12 ; from->r12 = r12 
    mov [rdi+40], r13 ; from->r13 = r13
    mov [rdi+48], r14  ; from->r14 = r14 
    mov [rdi+56], r15 ; from->r15 = r15 
    stmxcsr [rdi+64] ; from->mxcsr = mxcsr
    fnstcw [rdi+68] ; from->x86_fcw = x86_fcw

    mov rsp, [rsi+8] ; rsp = to->sp

    mov rbx, [rsi+16] ; rbx = to->rbx
    mov rbp, [rsi+24] ; rbp = to->rbp
    mov r12, [rsi+32] ; r12 = to->r12
    mov r13, [rsi+40] ; r13 = to->r13 
    mov r14, [rsi+48] ; r14 = to->r14 
    mov r15, [rsi+56] ; r15 = to->r15

    ldmxcsr [rsi+64] ; mxcsr = to->mxcsr
    fldcw [rsi+68] ; x86_fcw = to->x86_fcw
    mov r8, [rsi] ; r8 = to->ip
    push r8 ; stack[] = to->ip 

    ret ; ip = stack[]