#ifndef STAPELOS
#include <stdio.h>
#include <stdlib.h>
#else
#include "../../kernel/include/string.h"
#include "../../kernel/include/memory.h"
#define printf printk
#endif
#include <stdint.h>
#include "include/general.h"

int multitaskingmax = 0;
StapelMultitaskingInstance multitaskingarea[10];

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

void stack_push(StapelMultitaskingInstance* cv,uint64_t value)
{
    ((uint64_t*)cv->stack_pointer)[0] = value;
    cv->stack_pointer += sizeof(uint64_t);
}

uint64_t stack_pop(StapelMultitaskingInstance* cv)
{
    cv->stack_pointer -= sizeof(uint64_t);
    uint64_t oldval = ((uint64_t*)cv->stack_pointer)[0];
    ((uint64_t*)cv->stack_pointer)[0] = 0;
    return oldval;
}

uint8_t grab_next_instruction(StapelMultitaskingInstance* cv)
{
    return ((uint8_t*)cv->instruction_pointer)[0];
}

uint64_t grab_next_argument(StapelMultitaskingInstance* cv)
{
    return ((uint64_t*)cv->instruction_pointer)[0];
}

void add_instruction_pointer_uint8(StapelMultitaskingInstance* cv)
{
    cv->instruction_pointer += sizeof(uint8_t);
}

void add_instruction_pointer_uint64(StapelMultitaskingInstance* cv)
{
    cv->instruction_pointer += sizeof(uint64_t);
}

int handle_next_instruction(StapelMultitaskingInstance* cv)
{
    uint8_t instruction = grab_next_instruction(cv);
    #ifdef DEBUG
    printf("DEBUG: address: 0x%lx instruction: 0x%x \n",cv->instruction_pointer,instruction);
    #endif
    if(instruction==STAPEL_INSTRUCTION_EXIT)
    {
			return 0;
    }
    else if(instruction==STAPEL_INSTRUCTION_PUSH_ADDRESS_VALUE)
    {
        add_instruction_pointer_uint8(cv);
        uint64_t address_to_get = grab_next_argument(cv);
        uint64_t value_at_address = ((uint64_t*)( address_to_get + cv->central_memory ))[0];
        add_instruction_pointer_uint64(cv);
        #ifdef DEBUG
            printf("DEBUG: reading value of 0x%lx at 0x%lx \n",value_at_address,address_to_get);
        #endif
        stack_push(cv,value_at_address);
    }
    else if(instruction==STAPEL_INSTRUCTION_PUSH_VALUE)
    {
        add_instruction_pointer_uint8(cv);
        uint64_t value_at_address = grab_next_argument(cv);
        add_instruction_pointer_uint64(cv);
        #ifdef DEBUG
            printf("DEBUG: reading value of 0x%lx \n",value_at_address);
        #endif
        stack_push(cv,value_at_address);
    }
    else if(instruction==STAPEL_INSTRUCTION_DEBUG)
    {
        printf("----START DEBUGGING----\n");
        printf("address: 0x%lx instruction: 0x%x \n",cv->instruction_pointer,instruction);
        printf("\nstack:\n");
        uint64_t internalstack = (uint64_t) cv->stack;
        while(1)
        {
            printf("- address: 0x%lx value:%lx \n",internalstack,((uint64_t*)internalstack)[0]);
            if(internalstack==cv->stack_pointer)
            {
                break;
            }
            internalstack += sizeof(uint64_t);
        }
        printf("\ncall stack:\n");
        internalstack = (uint64_t) cv->call_stack;
        while(1)
        {
            printf("- address: 0x%lx value:%lx \n",internalstack,((uint64_t*)internalstack)[0]);
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
        uint64_t valA = stack_pop(cv);
        uint64_t valB = stack_pop(cv);
        stack_push(cv,valA+valB);
        add_instruction_pointer_uint8(cv);
    }
    else if(instruction==STAPEL_INSTRUCTION_SUB)
    {
        uint64_t valA = stack_pop(cv);
        uint64_t valB = stack_pop(cv);
        stack_push(cv,valA+valB);
        add_instruction_pointer_uint8(cv);
    }
    else if(instruction==STAPEL_INSTRUCTION_MUL)
    {
        uint64_t valA = stack_pop(cv);
        uint64_t valB = stack_pop(cv);
        stack_push(cv,valA*valB);
        add_instruction_pointer_uint8(cv);
    }
    else if(instruction==STAPEL_INSTRUCTION_DIV)
    {
        uint64_t valA = stack_pop(cv);
        uint64_t valB = stack_pop(cv);
        stack_push(cv,valA/valB);
        add_instruction_pointer_uint8(cv);
    }
    else if(instruction==STAPEL_INSTRUCTION_CALL)
    {
        add_instruction_pointer_uint8(cv);
        uint64_t value_at_address = grab_next_argument(cv);
        #ifdef DEBUG
            printf("DEBUG: calling 0x%lx from 0x%lx \n",value_at_address,cv->instruction_pointer);
        #endif
        add_instruction_pointer_uint64(cv);
        call_stack_push(cv,cv->instruction_pointer);
        cv->instruction_pointer = ( value_at_address + (uint64_t)cv->central_memory );
    }
    else if(instruction==STAPEL_INSTRUCTION_JUMP)
    {
        add_instruction_pointer_uint8(cv);
        uint64_t value_at_address = grab_next_argument(cv);
        #ifdef DEBUG
            printf("DEBUG: jumping 0x%lx from 0x%lx \n",value_at_address,cv->instruction_pointer);
        #endif
        add_instruction_pointer_uint64(cv);
        cv->instruction_pointer = ( value_at_address + (uint64_t)cv->central_memory );
    }
    else if(instruction==STAPEL_INSTRUCTION_INT)
    {
				#ifdef WASM
        	printf("FATAL: Invalid instruction\n");
        	exit(EXIT_FAILURE);
				#elseif STAPELOS
				#else
		      add_instruction_pointer_uint8(cv);
		      uint64_t asmEDI = stack_pop(cv);
		      uint64_t asmESI = stack_pop(cv);
		      uint64_t asmEDX = stack_pop(cv);
		      uint64_t asmECX = stack_pop(cv);
		      uint64_t asmEBX = stack_pop(cv);
		      uint64_t asmEAX = stack_pop(cv);
		      #ifdef DEBUG
		          printf("DEBUG: SYSTEMCALL: eax:0x%lx ebx:0x%lx ecx:0x%lx edx:0x%lx esi:0x%lx edi:0x%lx \n",asmEAX,asmEBX,asmECX,asmEDX,asmESI,asmEDI);
		      #endif
		      void* res = 0;
		      __asm__ __volatile__( "int $0x80" : "=a"(res) : "a"(asmEAX) , "b" (asmEBX), "c" (asmECX), "d" (asmEDX), "S" (asmESI), "D" (asmEDI) );
		      stack_push(cv,(uint64_t)res);
		    #endif
    }
    else if(instruction==STAPEL_INSTRUCTION_JUMP_EQUALS)
    {
        add_instruction_pointer_uint8(cv);
        uint64_t value_at_address =  grab_next_argument(cv);
        add_instruction_pointer_uint64(cv);
        uint64_t valA = stack_pop(cv);
        uint64_t valB = stack_pop(cv);
        #ifdef DEBUG
            printf("DEBUG: conditional jump A(0x%lx)==B(0x%lx) if true, jump to 0x%lx  \n",valA,valB,value_at_address);
        #endif
        if(valA==valB)
        {
            cv->instruction_pointer = value_at_address;
        }
    }
    else if(instruction==STAPEL_INSTRUCTION_JUMP_MORE)
    {
        add_instruction_pointer_uint8(cv);
        uint64_t value_at_address =  grab_next_argument(cv);
        add_instruction_pointer_uint64(cv);
        uint64_t valA = stack_pop(cv);
        uint64_t valB = stack_pop(cv);
        #ifdef DEBUG
            printf("DEBUG: conditional jump A(0x%lx)>B(0x%lx) if true, jump to 0x%lx  \n",valA,valB,value_at_address);
        #endif
        if(valA>valB)
        {
            cv->instruction_pointer = value_at_address;
        }
    }
    else if(instruction==STAPEL_INSTRUCTION_JUMP_LESS)
    {
        add_instruction_pointer_uint8(cv);
        uint64_t value_at_address =  grab_next_argument(cv);
        add_instruction_pointer_uint64(cv);
        uint64_t valA = stack_pop(cv);
        uint64_t valB = stack_pop(cv);
        #ifdef DEBUG
            printf("DEBUG: conditional jump A(0x%lx)<B(0x%lx) if true, jump to 0x%lx  \n",valA,valB,value_at_address);
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
    else if(instruction==STAPEL_INSTRUCTION_POP)
    {
        add_instruction_pointer_uint8(cv);
        uint64_t value_at_address =  grab_next_argument(cv) + (uint64_t) cv->central_memory;
        add_instruction_pointer_uint64(cv);
        uint64_t val = stack_pop(cv);
        #ifdef DEBUG
            printf("DEBUG: popping the value of 0x%lx from the stack and put it at 0x%lx \n",val,value_at_address);
        #endif
        ((uint64_t*)value_at_address)[0] = val;
    }
    else if(instruction==STAPEL_INSTRUCTION_PUSH_RAW_ADDR)
    {
        add_instruction_pointer_uint8(cv);
        uint64_t value_at_address =  grab_next_argument(cv);
        add_instruction_pointer_uint64(cv);
        #ifdef DEBUG
            printf("DEBUG: reading value of 0x%lx \n",value_at_address);
        #endif
        stack_push(cv,value_at_address + (uint64_t)cv->central_memory);
    }
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
    printf("DEBUG: beginning of central memory: 0x%lx \n",(uint64_t)sth);
    printf("DEBUG: grote van stapelfileheader: %ld \n",sizeof(StapelFileHeader));
    printf("DEBUG: signature: %c %c \n",sth->signature[0],sth->signature[1]);
    printf("DEBUG: version: 0x%lx \n",sth->version);
    printf("DEBUG: architecture: 0x%x \n",sth->architecture);
    #endif

    if(!(sth->signature[0]==STAPEL_HEADER_SIGNATURE_A&&sth->signature[1]==STAPEL_HEADER_SIGNATURE_B))
    {
        printf("FATAL: Invalid signature \n");
        return 0;
    }

    if(sth->version!=STAPEL_HEADER_VERSION)
    {
        printf("FATAL: Invalid version \n");
        return 0;
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

#ifndef STAPELOS
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
			exit(EXIT_FAILURE);
		}

		while(1)
    {
        handle_next_instruction(cv);
    }

    exit(EXIT_SUCCESS);
}
#endif 
