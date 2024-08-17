#ifndef STAPELOS
#include <stdio.h>
#include <stdlib.h>
#else
#include "../../kernel/include/string.h"
#include "../../kernel/include/memory.h"
#include "../../kernel/include/ps2_keyboard.h"
#define printf printk
#endif
#include <stdint.h>
#include "include/general.h"

#ifdef WASM
    #define PRINTLONG "%llx"
#endif 
#ifndef WASM
    #define PRINTLONG "%lx"
#endif 
#ifdef PROGRAM
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#endif 
int multitaskingmax = 0;
StapelMultitaskingInstance multitaskingarea[10];

#ifdef PROGRAM
static const int STDIN = 0;

void enable_terminal_features(){
    struct termios term;
    tcgetattr(STDIN, &term);
    term.c_lflag &= ~ICANON;
    tcsetattr(STDIN, TCSANOW, &term);
    setbuf(stdin, NULL);
}

int  getch(void) {
    int ch = 0;
    int bytesWaiting;
    ioctl(STDIN, FIONREAD, &bytesWaiting);
    if(bytesWaiting>0){
        struct termios oldattr, newattr;
        tcgetattr( STDIN_FILENO, &oldattr );
        newattr = oldattr;
        newattr.c_lflag &= ~( ICANON | ECHO );
        tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
        ch = getchar();
        tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
    }
    return ch;
}
#endif 
#ifdef WASM
int kbbuff = 0;
void setKbbuf(int val){
    kbbuff = val;
}
int  getch(void){
    return kbbuff;
}
#endif 

void call_stack_push(StapelMultitaskingInstance* cv,uint64_t value)
{
    ((uint64_t*)cv->call_stack_pointer)[0] = value;
    cv->call_stack_pointer += sizeof(uint64_t);
}

uint64_t call_stack_pop(StapelMultitaskingInstance* cv)
{
    cv->call_stack_pointer -= sizeof(uint64_t);
    uint64_t oldval = ((uint64_t*)cv->call_stack_pointer)[0];
    ((uint64_t*)cv->call_stack_pointer)[0] = 0;
    return oldval;
}

void stack_push_uint64(StapelMultitaskingInstance* cv,uint64_t value)
{
    ((uint64_t*)cv->stack_pointer)[0] = value;
    cv->stack_pointer += sizeof(uint64_t);
}

void stack_push_uint8(StapelMultitaskingInstance* cv,uint8_t value)
{
    ((uint8_t*)cv->stack_pointer)[0] = value;
    cv->stack_pointer += sizeof(uint8_t);
}

void stack_push_uint16(StapelMultitaskingInstance* cv,uint16_t value)
{
    ((uint16_t*)cv->stack_pointer)[0] = value;
    cv->stack_pointer += sizeof(uint16_t);
}

uint64_t stack_pop_uint64(StapelMultitaskingInstance* cv)
{
    cv->stack_pointer -= sizeof(uint64_t);
    uint64_t oldval = ((uint64_t*)cv->stack_pointer)[0];
    ((uint64_t*)cv->stack_pointer)[0] = 0;
    return oldval;
}

uint8_t stack_pop_uint8(StapelMultitaskingInstance* cv)
{
    cv->stack_pointer -= sizeof(uint8_t);
    uint8_t oldval = ((uint8_t*)cv->stack_pointer)[0];
    ((uint8_t*)cv->stack_pointer)[0] = 0;
    return oldval;
}

uint16_t stack_pop_uint16(StapelMultitaskingInstance* cv)
{
    cv->stack_pointer -= sizeof(uint16_t);
    uint16_t oldval = ((uint16_t*)cv->stack_pointer)[0];
    ((uint16_t*)cv->stack_pointer)[0] = 0;
    return oldval;
}

uint64_t grab_next_instruction_uint64(StapelMultitaskingInstance* cv)
{
    return ((uint64_t*)cv->instruction_pointer)[0];
}

uint8_t grab_next_instruction_uint8(StapelMultitaskingInstance* cv)
{
    return ((uint8_t*)cv->instruction_pointer)[0];
}

uint16_t grab_next_instruction_uint16(StapelMultitaskingInstance* cv)
{
    return ((uint16_t*)cv->instruction_pointer)[0];
}

void add_instruction_pointer_uint8(StapelMultitaskingInstance* cv)
{
    cv->instruction_pointer += sizeof(uint8_t);
}

void add_instruction_pointer_uint64(StapelMultitaskingInstance* cv)
{
    cv->instruction_pointer += sizeof(uint64_t);
}

void add_instruction_pointer_uint16(StapelMultitaskingInstance* cv)
{
    cv->instruction_pointer += sizeof(uint16_t);
}

int handle_next_instruction(StapelMultitaskingInstance* cv)
{
    #if STAPEL_HEADER_VERSION > 1
    StapelFileHeader* sth = (StapelFileHeader*)cv->central_memory;
    #endif 
    uint8_t instruction = grab_next_instruction_uint8(cv);
    #ifdef DEBUG
    printf("DEBUG: address: 0x" PRINTLONG " instruction: 0x%x \n",cv->instruction_pointer,instruction);
    #endif
    if(instruction==STAPEL_INSTRUCTION_EXIT)
    {
        #ifdef DEBUG
        printf("DEBUG: program ends with EXIT\n");
        #endif
		return 0;
    }
    else if(instruction==STAPEL_INSTRUCTION_PUSH_ADDRESS_VALUE_64)
    {
        add_instruction_pointer_uint8(cv);
        uint64_t address_to_get = grab_next_instruction_uint64(cv);
        uint64_t value_at_address = ((uint64_t*)( address_to_get + cv->central_memory ))[0];
        add_instruction_pointer_uint64(cv);
        #ifdef DEBUG
            printf("DEBUG: reading 64b value of 0x" PRINTLONG " at 0x" PRINTLONG " \n",value_at_address,address_to_get);
        #endif
        stack_push_uint64(cv,value_at_address);
    }
    else if(instruction==STAPEL_INSTRUCTION_PUSH_VALUE_64)
    {
        add_instruction_pointer_uint8(cv);
        uint64_t value_at_address = grab_next_instruction_uint64(cv);
        add_instruction_pointer_uint64(cv);
        #ifdef DEBUG
            printf("DEBUG: reading 64b value of 0x" PRINTLONG " \n",value_at_address);
        #endif
        stack_push_uint64(cv,value_at_address);
    }
    else if(instruction==STAPEL_INSTRUCTION_DEBUG)
    {
        printf("----START DEBUGGING----\n");
        printf("address: 0x" PRINTLONG " instruction: 0x%x \n",cv->instruction_pointer,instruction);
        printf("\nstack:\n");
        uint64_t internalstack = (uint64_t) cv->stack;
        while(1)
        {
            printf("- address: 0x" PRINTLONG " value: 0x%x \n",internalstack,((uint8_t*)internalstack)[0]);
            if(internalstack==cv->stack_pointer)
            {
                break;
            }
            internalstack += sizeof(uint8_t);
        }
        printf("\ncall stack:\n");
        internalstack = (uint64_t) cv->call_stack;
        while(1)
        {
            printf("- address: 0x" PRINTLONG " value: 0x" PRINTLONG " \n",internalstack,((uint64_t*)internalstack)[0]);
            if(internalstack==cv->call_stack_pointer)
            {
                break;
            }
            internalstack += sizeof(uint64_t);
        }
        printf("----END   DEBUGGING----\n");
        add_instruction_pointer_uint8(cv);
    }
    else if(instruction==STAPEL_INSTRUCTION_ADD)
    {
        uint64_t valA = stack_pop_uint64(cv);
        uint64_t valB = stack_pop_uint64(cv);
        stack_push_uint64(cv,valA+valB);
        add_instruction_pointer_uint8(cv);
    }
    else if(instruction==STAPEL_INSTRUCTION_SUB)
    {
        uint64_t valA = stack_pop_uint64(cv);
        uint64_t valB = stack_pop_uint64(cv);
        stack_push_uint64(cv,valA+valB);
        add_instruction_pointer_uint8(cv);
    }
    else if(instruction==STAPEL_INSTRUCTION_MUL)
    {
        uint64_t valA = stack_pop_uint64(cv);
        uint64_t valB = stack_pop_uint64(cv);
        stack_push_uint64(cv,valA*valB);
        add_instruction_pointer_uint8(cv);
    }
    else if(instruction==STAPEL_INSTRUCTION_DIV)
    {
        uint64_t valA = stack_pop_uint64(cv);
        uint64_t valB = stack_pop_uint64(cv);
        stack_push_uint64(cv,valA/valB);
        add_instruction_pointer_uint8(cv);
    }
    else if(instruction==STAPEL_INSTRUCTION_CALL)
    {
        add_instruction_pointer_uint8(cv);
        uint64_t value_at_address = grab_next_instruction_uint64(cv);
        #ifdef DEBUG
            printf("DEBUG: calling 0x" PRINTLONG " from 0x" PRINTLONG " \n",value_at_address,cv->instruction_pointer);
        #endif
        add_instruction_pointer_uint64(cv);
        call_stack_push(cv,cv->instruction_pointer);
        cv->instruction_pointer = ( value_at_address + (uint64_t)cv->central_memory );
    }
    else if(instruction==STAPEL_INSTRUCTION_JUMP)
    {
        add_instruction_pointer_uint8(cv);
        uint64_t value_at_address = grab_next_instruction_uint64(cv);
        #ifdef DEBUG
            printf("DEBUG: jumping 0x" PRINTLONG " from 0x" PRINTLONG " \n",value_at_address,cv->instruction_pointer);
        #endif
        add_instruction_pointer_uint64(cv);
        cv->instruction_pointer = ( value_at_address + (uint64_t)cv->central_memory );
    }
    else if(instruction==STAPEL_INSTRUCTION_INT)
    {
        #ifdef WASM
            printf("FATAL: Invalid instruction; interrupt is not supported\n");
            exit(EXIT_FAILURE);
        #else 
            add_instruction_pointer_uint8(cv);
            uint64_t asmEDI = stack_pop_uint64(cv);
            uint64_t asmESI = stack_pop_uint64(cv);
            uint64_t asmEDX = stack_pop_uint64(cv);
            uint64_t asmECX = stack_pop_uint64(cv);
            uint64_t asmEBX = stack_pop_uint64(cv);
            uint64_t asmEAX = stack_pop_uint64(cv);
            #ifdef DEBUG
                printf("DEBUG: SYSTEMCALL: eax:0x" PRINTLONG " ebx:0x" PRINTLONG " ecx:0x" PRINTLONG " edx:0x" PRINTLONG " esi:0x" PRINTLONG " edi:0x" PRINTLONG " \n",asmEAX,asmEBX,asmECX,asmEDX,asmESI,asmEDI);
            #endif
            void* res = 0;
            __asm__ __volatile__( "int $0x80" : "=a"(res) : "a"(asmEAX) , "b" (asmEBX), "c" (asmECX), "d" (asmEDX), "S" (asmESI), "D" (asmEDI) );
            stack_push_uint64(cv,(uint64_t)res);
        #endif 
    }
    else if(instruction==STAPEL_INSTRUCTION_JUMP_EQUALS)
    {
        add_instruction_pointer_uint8(cv);
        uint64_t value_at_address =  grab_next_instruction_uint64(cv);
        add_instruction_pointer_uint64(cv);
        uint64_t valA = stack_pop_uint64(cv);
        uint64_t valB = stack_pop_uint64(cv);
        #ifdef DEBUG
            printf("DEBUG: conditional jump A(0x" PRINTLONG ")==B(0x" PRINTLONG ") if true, jump to 0x" PRINTLONG "  \n",valA,valB,value_at_address);
        #endif
        if(valA==valB)
        {
            cv->instruction_pointer = ( value_at_address + (uint64_t)cv->central_memory );
        }
    }
    else if(instruction==STAPEL_INSTRUCTION_JUMP_MORE)
    {
        add_instruction_pointer_uint8(cv);
        uint64_t value_at_address =  grab_next_instruction_uint64(cv);
        add_instruction_pointer_uint64(cv);
        uint64_t valA = stack_pop_uint64(cv);
        uint64_t valB = stack_pop_uint64(cv);
        #ifdef DEBUG
            printf("DEBUG: conditional jump A(0x" PRINTLONG ")>B(0x" PRINTLONG ") if true, jump to 0x" PRINTLONG "  \n",valA,valB,value_at_address);
        #endif
        if(valA>valB)
        {
            cv->instruction_pointer = value_at_address;
        }
    }
    else if(instruction==STAPEL_INSTRUCTION_JUMP_LESS)
    {
        add_instruction_pointer_uint8(cv);
        uint64_t value_at_address =  grab_next_instruction_uint64(cv);
        add_instruction_pointer_uint64(cv);
        uint64_t valA = stack_pop_uint64(cv);
        uint64_t valB = stack_pop_uint64(cv);
        #ifdef DEBUG
            printf("DEBUG: conditional jump A(0x" PRINTLONG ")<B(0x" PRINTLONG ") if true, jump to 0x" PRINTLONG "  \n",valA,valB,value_at_address);
        #endif
        if(valA<valB)
        {
            cv->instruction_pointer = value_at_address;
        }
    }
    else if(instruction==STAPEL_INSTRUCTION_RET)
    {
        add_instruction_pointer_uint8(cv);
        #ifdef DEBUG
            printf("DEBUG: return statement  \n");
        #endif
        cv->instruction_pointer = call_stack_pop(cv);
    }
    else if(instruction==STAPEL_INSTRUCTION_POP_64)
    {
        add_instruction_pointer_uint8(cv);
        uint64_t value_at_address =  grab_next_instruction_uint64(cv) + (uint64_t) cv->central_memory;
        add_instruction_pointer_uint64(cv);
        uint64_t val = stack_pop_uint64(cv);
        #ifdef DEBUG
            printf("DEBUG: popping the value of 0x" PRINTLONG " from the stack and put it at 0x" PRINTLONG " \n",val,value_at_address);
        #endif
        ((uint64_t*)value_at_address)[0] = val;
    }
    else if(instruction==STAPEL_INSTRUCTION_PUSH_RAW_ADDR)
    {
        add_instruction_pointer_uint8(cv);
        uint64_t value_at_address =  grab_next_instruction_uint64(cv);
        add_instruction_pointer_uint64(cv);
        #ifdef DEBUG
            printf("DEBUG: reading 64b value of 0x" PRINTLONG " \n",value_at_address);
        #endif
        stack_push_uint64(cv,value_at_address + (uint64_t)cv->central_memory);
    }
    #if STAPEL_HEADER_VERSION > 1
    else if(sth->version>1 && instruction==STAPEL_INSTRUCTION_SYSCALL)
    {
        add_instruction_pointer_uint8(cv);
        uint8_t callid = grab_next_instruction_uint64(cv);
        add_instruction_pointer_uint8(cv);
        #ifdef DEBUG
            printf("DEBUG: call interrupt %d with syscall \n",callid);
        #endif
        if(callid==0)
        {
            #ifdef DEBUG
            printf("DEBUG: syscall to get API version\n");
            #endif
            stack_push_uint64(cv,STAPEL_SYSCALL_VERSION);
        }
        else if(callid==1)
        {
            uint64_t straddr = stack_pop_uint64(cv);
            #ifdef DEBUG
            printf("DEBUG: syscall print string with address: " PRINTLONG "\n",straddr);
            #endif
            printf("%s",(char*)straddr);
        }
        else if(callid==2)
        {
            uint64_t res = getch();
            #ifdef DEBUG
            printf("DEBUG: syscall getchar (does not wait) with " PRINTLONG "\n",res);
            #endif 
            stack_push_uint64(cv,res);
        }
    }
    else if(sth->version>1 && instruction==STAPEL_INSTRUCTION_PUSH_VALUE_8)
    {
        add_instruction_pointer_uint8(cv);
        uint8_t value_at_address = grab_next_instruction_uint8(cv);
        add_instruction_pointer_uint8(cv);
        #ifdef DEBUG
            printf("DEBUG: reading 8b value of 0x%x \n",value_at_address);
        #endif
        stack_push_uint8(cv,value_at_address);
    }
    else if(sth->version>1 && instruction==STAPEL_INSTRUCTION_PUSH_ADDRESS_VALUE_8)
    {
        add_instruction_pointer_uint8(cv);
        uint64_t address_to_get = grab_next_instruction_uint64(cv);
        uint8_t value_at_address = ((uint8_t*)( address_to_get + cv->central_memory ))[0];
        add_instruction_pointer_uint64(cv);
        #ifdef DEBUG
            printf("DEBUG: reading 8b value of 0x%x at 0x" PRINTLONG " \n",value_at_address,address_to_get);
        #endif
        stack_push_uint8(cv,value_at_address);
    }
    else if(sth->version>1 && instruction==STAPEL_INSTRUCTION_POP_8)
    {
        add_instruction_pointer_uint8(cv);
        uint64_t value_at_address =  grab_next_instruction_uint64(cv) + (uint64_t) cv->central_memory;
        add_instruction_pointer_uint64(cv);
        uint8_t val = stack_pop_uint8(cv);
        #ifdef DEBUG
            printf("DEBUG: popping the value of 0x%x from the stack and put it at 0x" PRINTLONG " \n",val,value_at_address);
        #endif
        ((uint8_t*)value_at_address)[0] = val;
    }
    else if(sth->version>1 && instruction==STAPEL_INSTRUCTION_POP_AT_64)
    {
        add_instruction_pointer_uint8(cv);
        uint64_t address_to_get = grab_next_instruction_uint64(cv);
        add_instruction_pointer_uint64(cv);
        uint64_t value_at_address = ((uint64_t*)( address_to_get + cv->central_memory ))[0];
        #ifdef DEBUG
            printf("DEBUG: reading 64b value of 0x" PRINTLONG " at 0x" PRINTLONG " \n",value_at_address,address_to_get);
        #endif
        uint64_t uval = stack_pop_uint64(cv);
        ((uint64_t*)( value_at_address ))[0] = uval;
    }
    else if(sth->version>1 && instruction==STAPEL_INSTRUCTION_POP_AT_8)
    {
        add_instruction_pointer_uint8(cv);
        uint64_t address_to_get = grab_next_instruction_uint64(cv);
        add_instruction_pointer_uint64(cv);
        uint64_t value_at_address = ((uint64_t*)( address_to_get + cv->central_memory ))[0];
        #ifdef DEBUG
            printf("DEBUG: reading 8b value of 0x" PRINTLONG " at 0x" PRINTLONG " \n",value_at_address,address_to_get);
        #endif
        uint8_t uval = stack_pop_uint8(cv);
        ((uint8_t*)( value_at_address ))[0] = uval;
    }
    else if(sth->version>1 && instruction==STAPEL_INSTRUCTION_PUSH_VALUE_16)
    {
        add_instruction_pointer_uint8(cv);
        uint16_t value_at_address = grab_next_instruction_uint16(cv);
        add_instruction_pointer_uint16(cv);
        #ifdef DEBUG
            printf("DEBUG: reading 16b value of 0x%x \n",value_at_address);
        #endif
        stack_push_uint16(cv,value_at_address);
    }
    else if(sth->version>1 && instruction==STAPEL_INSTRUCTION_PUSH_ADDRESS_VALUE_16)
    {
        add_instruction_pointer_uint8(cv);
        uint64_t address_to_get = grab_next_instruction_uint64(cv);
        uint16_t value_at_address = ((uint16_t*)( address_to_get + cv->central_memory ))[0];
        add_instruction_pointer_uint64(cv);
        #ifdef DEBUG
            printf("DEBUG: reading 16b value of 0x%x at 0x" PRINTLONG " \n",value_at_address,address_to_get);
        #endif
        stack_push_uint16(cv,value_at_address);
    }
    else if(sth->version>1 && instruction==STAPEL_INSTRUCTION_POP_16)
    {
        add_instruction_pointer_uint8(cv);
        uint64_t value_at_address =  grab_next_instruction_uint64(cv) + (uint64_t) cv->central_memory;
        add_instruction_pointer_uint64(cv);
        uint16_t val = stack_pop_uint16(cv);
        #ifdef DEBUG
            printf("DEBUG: popping the value of 0x%x from the stack and put it at 0x" PRINTLONG " \n",val,value_at_address);
        #endif
        ((uint16_t*)value_at_address)[0] = val;
    }
    else if(sth->version>1 && instruction==STAPEL_INSTRUCTION_POP_AT_16)
    {
        add_instruction_pointer_uint8(cv);
        uint64_t address_to_get = grab_next_instruction_uint64(cv);
        add_instruction_pointer_uint64(cv);
        uint64_t value_at_address = ((uint64_t*)( address_to_get + cv->central_memory ))[0];
        #ifdef DEBUG
            printf("DEBUG: reading 8b value of 0x" PRINTLONG " at 0x" PRINTLONG " \n",value_at_address,address_to_get);
        #endif
        uint16_t uval = stack_pop_uint16(cv);
        ((uint16_t*)( value_at_address ))[0] = uval;
    }
    #endif 
    else
    {
        printf("FATAL: Invalid instruction\n");
		#ifndef STAPELOS
        exit(EXIT_FAILURE);
        #endif
        return 0;
    }
		return 1;
}

StapelMultitaskingInstance* insert_stapel_cardridge(void* memoryregion)
{
    StapelMultitaskingInstance *cardridge = (StapelMultitaskingInstance*) &multitaskingarea[multitaskingmax++];
    cardridge->central_memory = memoryregion;
    cardridge->stack = malloc(STAPEL_LIMITATION_STACK*sizeof(uint64_t));
  	cardridge->call_stack = malloc(STAPEL_LIMITATION_STACK*sizeof(uint64_t));
	StapelFileHeader* sth = (StapelFileHeader*)cardridge->central_memory;
    #ifdef DEBUG
        printf("DEBUG: beginning of central memory: 0x" PRINTLONG " \n",(uint64_t)sth);
        printf("DEBUG: size of the stapelfileheader: %ld \n",sizeof(StapelFileHeader));
        printf("DEBUG: signature: %c %c \n",sth->signature[0],sth->signature[1]);
        printf("DEBUG: version: 0x" PRINTLONG " \n",sth->version);
        printf("DEBUG: architecture: 0x%x \n",sth->architecture);
    #endif

    if(!(sth->signature[0]==STAPEL_HEADER_SIGNATURE_A&&sth->signature[1]==STAPEL_HEADER_SIGNATURE_B))
    {
        printf("FATAL: Invalid signature \n");
        return 0;
    }

    if(sth->version>STAPEL_HEADER_VERSION)
    {
        printf("FATAL: Invalid version \n");
        return 0;
    }

    if(sth->version!=STAPEL_HEADER_VERSION)
    {
        printf("Warning: Version mismatch \n");
    }

    if(sth->architecture!=STAPEL_HEADER_ARCHITECTURE)
    {
        printf("FATAL: Invalid architecture \n");
        return 0;
    }

    cardridge->instruction_pointer = 0;
    cardridge->stack_pointer = 0;
    cardridge->call_stack_pointer = 0;

    cardridge->instruction_pointer     = (uint64_t) ( cardridge->central_memory + sizeof(StapelFileHeader) );
    cardridge->stack_pointer           = (uint64_t) cardridge->stack;
    cardridge->call_stack_pointer      = (uint64_t) cardridge->call_stack;

	return cardridge;
}

#ifdef WASM
StapelMultitaskingInstance* defproc = NULL;
int handle_default_next_instruction(){
    return handle_next_instruction(defproc);
}
#endif 

#ifndef STAPELOS
int main(int argc , char** argv)
{
    #ifdef PROGRAM
        enable_terminal_features();
    #endif 
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
        return EXIT_FAILURE;
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
        return EXIT_FAILURE;
    }

    fseek(targetfile,0,SEEK_END);
    uint64_t filesize = ftell(targetfile);
    #ifdef DEBUG
    printf("DEBUG: The filesize of \"%s\" is " PRINTLONG " bytes \n",argv[1],filesize);
    #endif
    void *central_memory = malloc(filesize);

    fseek(targetfile,0,SEEK_SET);
    fread(central_memory,filesize,1,targetfile);

    #ifdef DEBUG
    printf("DEBUG: Trying to close \"%s\" \n",argv[1]);
    #endif
    fclose(targetfile);

    StapelMultitaskingInstance* cv = insert_stapel_cardridge(central_memory);
    if(cv==NULL)
    {
        return EXIT_FAILURE;
    }
    #ifdef WASM
    defproc = cv;
    #endif 

    #ifndef WASM
    while(handle_next_instruction(cv));
    #endif 

    return EXIT_SUCCESS;
}
#endif 
