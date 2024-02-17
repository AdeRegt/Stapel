#include <stdio.h>
#include <stdlib.h> 
#include <stdint.h>

typedef struct {
    uint8_t signature[2];
    uint64_t version;
    uint8_t architecture;
}__attribute__((packed)) StapelFileHeader;

#define STAPEL_LIMITATION_STACK                     100

#define STAPEL_HEADER_SIGNATURE_A                   'S'
#define STAPEL_HEADER_SIGNATURE_B                   'T'
#define STAPEL_HEADER_VERSION                       1
#define STAPEL_HEADER_ARCHITECTURE                  2

#define STAPEL_INSTRUCTION_EXIT                     0x00
#define STAPEL_INSTRUCTION_PUSH_ADDRESS_VALUE       0x01 
#define STAPEL_INSTRUCTION_PUSH_VALUE               0x02 
#define STAPEL_INSTRUCTION_DEBUG                    0x03
#define STAPEL_INSTRUCTION_ADD                      0x04 
#define STAPEL_INSTRUCTION_SUB                      0x05
#define STAPEL_INSTRUCTION_MUL                      0x06
#define STAPEL_INSTRUCTION_DIV                      0x07 
#define STAPEL_INSTRUCTION_CALL                     0x08 
#define STAPEL_INSTRUCTION_JUMP                     0x09 
#define STAPEL_INSTRUCTION_INT                      0x0A 
#define STAPEL_INSTRUCTION_JUMP_EQUALS              0x0B 
#define STAPEL_INSTRUCTION_JUMP_MORE                0x0C 
#define STAPEL_INSTRUCTION_JUMP_LESS                0x0D 
#define STAPEL_INSTRUCTION_RET                      0x0E
#define STAPEL_INSTRUCTION_POP                      0x0F
#define STAPEL_INSTRUCTION_PUSH_RAW_ADDR            0x10

void* central_memory;
void* stack;
void* call_stack;

uint64_t instruction_pointer = 0;
uint64_t stack_pointer = 0;
uint64_t call_stack_pointer = 0;

void call_stack_push(uint64_t value)
{
    ((uint64_t*)call_stack_pointer)[0] = value;
    call_stack_pointer += sizeof(uint64_t);
}

uint64_t call_stack_pop()
{
    call_stack_pointer -= sizeof(uint64_t);
    uint64_t oldval = ((uint64_t*)call_stack_pointer)[0];
    ((uint64_t*)call_stack_pointer)[0] = 0;
    return oldval;
}

void stack_push(uint64_t value)
{
    ((uint64_t*)stack_pointer)[0] = value;
    stack_pointer += sizeof(uint64_t);
}

uint64_t stack_pop()
{
    stack_pointer -= sizeof(uint64_t);
    uint64_t oldval = ((uint64_t*)stack_pointer)[0];
    ((uint64_t*)stack_pointer)[0] = 0;
    return oldval;
}

uint8_t grab_next_instruction()
{
    return ((uint8_t*)instruction_pointer)[0];
}

uint64_t grab_next_argument()
{
    return ((uint64_t*)instruction_pointer)[0];
}

void add_instruction_pointer_uint8()
{
    instruction_pointer += sizeof(uint8_t);
}

void add_instruction_pointer_uint64()
{
    instruction_pointer += sizeof(uint64_t);
}

void handle_next_instruction()
{
    uint8_t instruction = grab_next_instruction();
    #ifdef DEBUG
    printf("DEBUG: address: 0x%lx instruction: 0x%x \n",instruction_pointer,instruction);
    #endif 
    if(instruction==STAPEL_INSTRUCTION_EXIT)
    {
        exit(EXIT_SUCCESS);
    }
    else if(instruction==STAPEL_INSTRUCTION_PUSH_ADDRESS_VALUE)
    {
        add_instruction_pointer_uint8();
        uint64_t address_to_get = grab_next_argument();
        uint64_t value_at_address = ((uint64_t*)( address_to_get + central_memory ))[0];
        add_instruction_pointer_uint64();
        #ifdef DEBUG
            printf("DEBUG: reading value of 0x%lx at 0x%lx \n",value_at_address,address_to_get);
        #endif 
        stack_push(value_at_address);
    }
    else if(instruction==STAPEL_INSTRUCTION_PUSH_VALUE)
    {
        add_instruction_pointer_uint8();
        uint64_t value_at_address = grab_next_argument();
        add_instruction_pointer_uint64();
        #ifdef DEBUG
            printf("DEBUG: reading value of 0x%lx \n",value_at_address);
        #endif 
        stack_push(value_at_address);
    }
    else if(instruction==STAPEL_INSTRUCTION_DEBUG)
    {
        printf("----START DEBUGGING----\n");
        printf("address: 0x%lx instruction: 0x%x \n",instruction_pointer,instruction);
        printf("\nstack:\n");
        uint64_t internalstack = (uint64_t) stack;
        while(1)
        {
            printf("- address: 0x%lx value:%lx \n",internalstack,((uint64_t*)internalstack)[0]);
            if(internalstack==stack_pointer)
            {
                break;
            }
            internalstack += sizeof(uint64_t);
        }
        printf("\ncall stack:\n");
        internalstack = (uint64_t) call_stack;
        while(1)
        {
            printf("- address: 0x%lx value:%lx \n",internalstack,((uint64_t*)internalstack)[0]);
            if(internalstack==call_stack_pointer)
            {
                break;
            }
            internalstack += sizeof(uint64_t);
        }
        printf("----END   DEBUGGING----\n");
        add_instruction_pointer_uint8();
    }
    else if(instruction==STAPEL_INSTRUCTION_ADD)
    {
        uint64_t valA = stack_pop();
        uint64_t valB = stack_pop();
        stack_push(valA+valB);
        add_instruction_pointer_uint8();
    }
    else if(instruction==STAPEL_INSTRUCTION_SUB)
    {
        uint64_t valA = stack_pop();
        uint64_t valB = stack_pop();
        stack_push(valA+valB);
        add_instruction_pointer_uint8();
    }
    else if(instruction==STAPEL_INSTRUCTION_MUL)
    {
        uint64_t valA = stack_pop();
        uint64_t valB = stack_pop();
        stack_push(valA*valB);
        add_instruction_pointer_uint8();
    }
    else if(instruction==STAPEL_INSTRUCTION_DIV)
    {
        uint64_t valA = stack_pop();
        uint64_t valB = stack_pop();
        stack_push(valA/valB);
        add_instruction_pointer_uint8();
    }
    else if(instruction==STAPEL_INSTRUCTION_CALL)
    {
        add_instruction_pointer_uint8();
        uint64_t value_at_address = grab_next_argument();
        #ifdef DEBUG
            printf("DEBUG: calling 0x%lx from 0x%lx \n",value_at_address,instruction_pointer);
        #endif 
        add_instruction_pointer_uint64();
        call_stack_push(instruction_pointer);
        instruction_pointer = ( value_at_address + (uint64_t)central_memory );
    }
    else if(instruction==STAPEL_INSTRUCTION_JUMP)
    {
        add_instruction_pointer_uint8();
        uint64_t value_at_address = grab_next_argument();
        #ifdef DEBUG
            printf("DEBUG: jumping 0x%lx from 0x%lx \n",value_at_address,instruction_pointer);
        #endif 
        add_instruction_pointer_uint64();
        instruction_pointer = ( value_at_address + (uint64_t)central_memory );
    }
    else if(instruction==STAPEL_INSTRUCTION_INT)
    {
        add_instruction_pointer_uint8();
        uint64_t asmEDI = stack_pop();
        uint64_t asmESI = stack_pop();
        uint64_t asmEDX = stack_pop();
        uint64_t asmECX = stack_pop();
        uint64_t asmEBX = stack_pop();
        uint64_t asmEAX = stack_pop();
        #ifdef DEBUG
            printf("DEBUG: SYSTEMCALL: eax:0x%lx ebx:0x%lx ecx:0x%lx edx:0x%lx esi:0x%lx edi:0x%lx \n",asmEAX,asmEBX,asmECX,asmEDX,asmESI,asmEDI);
        #endif 
        void* res = 0;
        __asm__ __volatile__( "int $0x80" : "=a"(res) : "a"(asmEAX) , "b" (asmEBX), "c" (asmECX), "d" (asmEDX), "S" (asmESI), "D" (asmEDI) );
        stack_push((uint64_t)res);
    }
    else if(instruction==STAPEL_INSTRUCTION_PUSH_RAW_ADDR)
    {
        add_instruction_pointer_uint8();
        uint64_t value_at_address =  grab_next_argument();
        add_instruction_pointer_uint64();
        #ifdef DEBUG
            printf("DEBUG: reading value of 0x%lx \n",value_at_address);
        #endif 
        stack_push(value_at_address + (uint64_t)central_memory);
    }
    else
    {
        printf("FATAL: Invalid instruction\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc , char** argv)
{
    //
    // show debugging information
    #ifdef DEBUG
    printf("DEBUG: Stapel is compiled in debug mode!\n");
    printf("DEBUG: boot: argc: %d \n",argc);
    for(int i = 0 ; i < argc ; i++)
    {
        printf("%d: %s \n",i+1,argv[i]);
    }
    #endif 

    //
    // look if there is a file we need to load!
    if(argc!=2)
    {
        printf("FATAL: Expected one parameter: filename! Example: %s example.st \n",argv[0]);
        exit(EXIT_FAILURE);
    }

    //
    // look if the file exists...
    #ifdef DEBUG
    printf("DEBUG: Trying to open \"%s\" \n",argv[1]);
    #endif 
    FILE* targetfile = fopen(argv[1],"r");
    if(!targetfile)
    {
        printf("FATAL: Unable to open file in read mode: \"%s\" \n",argv[1]);
        exit(EXIT_FAILURE);
    }

    fseek(targetfile,0,SEEK_END);
    uint64_t filesize = ftell(targetfile);
    #ifdef DEBUG
    printf("DEBUG: The filesize of \"%s\" is %ld bytes \n",argv[1],filesize);
    #endif 
    central_memory = malloc(filesize);
    stack = malloc(STAPEL_LIMITATION_STACK*sizeof(uint64_t));
    call_stack = malloc(STAPEL_LIMITATION_STACK*sizeof(uint64_t));

    fseek(targetfile,0,SEEK_SET);
    fread(central_memory,filesize,1,targetfile);

    #ifdef DEBUG
    printf("DEBUG: Trying to close \"%s\" \n",argv[1]);
    #endif 
    fclose(targetfile);

    StapelFileHeader* sth = (StapelFileHeader*)central_memory;
    #ifdef DEBUG
    printf("DEBUG: beginning of central memory: 0x%lx \n",(uint64_t)sth);
    printf("DEBUG: grote van stapelfileheader: %ld \n",sizeof(StapelFileHeader));
    printf("DEBUG: signature: %c %c \n",sth->signature[0],sth->signature[1]);
    printf("DEBUG: version: 0x%lx \n",sth->version);
    printf("DEBUG: architecture: 0x%x \n",sth->architecture);
    #endif 

    if(!(sth->signature[0]==STAPEL_HEADER_SIGNATURE_A&&sth->signature[1]==STAPEL_HEADER_SIGNATURE_B))
    {
        printf("FATAL: Invalid signature \n");
        exit(EXIT_FAILURE);
    }

    if(sth->version!=STAPEL_HEADER_VERSION)
    {
        printf("FATAL: Invalid version \n");
        exit(EXIT_FAILURE);
    }

    if(sth->architecture!=STAPEL_HEADER_ARCHITECTURE)
    {
        printf("FATAL: Invalid architecture \n");
        exit(EXIT_FAILURE);
    }

    instruction_pointer = 0;
    stack_pointer = 0;
    call_stack_pointer = 0;

    instruction_pointer     = (uint64_t) ( central_memory + sizeof(StapelFileHeader) );
    stack_pointer           = (uint64_t) stack;
    call_stack_pointer      = (uint64_t) call_stack;

    while(1)
    {
        handle_next_instruction();
    }
    
    exit(EXIT_SUCCESS);
}