[ORG 0]
[BITS 64]
db 'S' ; header signature S
db 'T' ; header signature T
dq 1 ; header version
db 2 ; 64 bit

%macro exit 0
    db 0
%endmacro 

%macro push_value_from 1
    db 1
    dq %1
%endmacro 

%macro push_value 1
    db 2
    dq %1
%endmacro 

%macro debug 0
    db 3
%endmacro 

%macro add 0
    db 4
%endmacro 

%macro sub 0
    db 5
%endmacro 

%macro mul 0
    db 6
%endmacro 

%macro div 0
    db 7
%endmacro 

%macro call 1
    db 8
    dq %1
%endmacro 

%macro jump 1
    db 9
    dq %1
%endmacro 

%macro int 0
    db 0xA
%endmacro 

%macro jump_equals 1
    db 0xB
    dq %1
%endmacro 

%macro jump_more 1
    db 0xC
    dq %1
%endmacro 

%macro jump_less 1
    db 0xD
    dq %1
%endmacro 