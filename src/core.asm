section .text

global GetCoreVersion
GetCoreVersion:
    mov rax, 1337      
    ret


global AsmComputeTotal
AsmComputeTotal:
    mov rax, rcx        
    add rax, rdx        
    ret
