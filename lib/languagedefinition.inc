[ORG 0]
[BITS 64]
db 'S' ; header signature S
db 'T' ; header signature T
dq 1 ; header version
db 2 ; 64 bit

%macro exit 0
    db 0x00
%endmacro 

%macro push_value_from 1
    db 0x01
    dq %1
%endmacro 

%macro push_value_with 1
    db 0x02
    dq %1
%endmacro 

%macro debug 0
    db 0x03
%endmacro 

%macro add 0
    db 0x04
%endmacro 

%macro sub 0
    db 0x05
%endmacro 

%macro mul 0
    db 0x06
%endmacro 

%macro div 0
    db 0x07
%endmacro 

%macro call 1
    db 0x08
    dq %1
%endmacro 

%macro jump 1
    db 0x09
    dq %1
%endmacro 

%macro int 0
    db 0x0A
%endmacro 

%macro jump_equals 1
    db 0x0B
    dq %1
%endmacro 

%macro jump_more 1
    db 0x0C
    dq %1
%endmacro 

%macro jump_less 1
    db 0x0D
    dq %1
%endmacro 

%macro return 0
    db 0x0E
%endmacro 

%macro push_value_addr 1
    db 0x10
    dq %1
%endmacro 