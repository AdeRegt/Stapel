#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/include/general.h"

typedef struct{
    char* filename;
    int line;
    int tokencount;
    char* content;
    void *tokens;
    void *next;
} SourceFileLine;

typedef struct{
    char* token;
    SourceFileLine *master;
    int token_id;
    void *next;
} SourceFileLineToken;

typedef struct{
    uint64_t value;
    int size;
    char* label_required;
    int requires_attention;
    char* label_defined;
    void* next;
} CompiledTreeValue;

SourceFileLine* sourcefile_begin = NULL;
SourceFileLine* sourcefile_now = NULL;

CompiledTreeValue* targetfile_begin = NULL;
CompiledTreeValue* targetfile_now = NULL;

void grammar_error_in_token(SourceFileLineToken* token,char* message)
{
    printf("FATAL: An error occured while compiling:\n");
    printf("       File   :   %s\n",token->master->filename);
    printf("       LineNo :   %d\n",token->master->line);
    printf("       Line   :   %s\n",token->master->content);
    printf("       TokenAt:   %d\n",token->token_id);
    printf("       Token  :   %s\n",token->token);
    printf("       Message:   %s\n",message);
    exit(EXIT_FAILURE);
}

void grammar_error_in_line(SourceFileLine* token,char* message)
{
    printf("FATAL: An error occured while compiling:\n");
    printf("       File   :   %s\n",token->filename);
    printf("       LineNo :   %d\n",token->line);
    printf("       Line   :   %s\n",token->content);
    printf("       Message:   %s\n",message);
    exit(EXIT_FAILURE);
}

char *label_defined = NULL;

void add_compiled_tree_value(uint64_t value, int size, char* label_required, int requires_attention)
{
    CompiledTreeValue* uv = (CompiledTreeValue*) malloc(sizeof(CompiledTreeValue));
    uv->label_defined = label_defined;
    uv->label_required = label_required;
    uv->next = NULL;
    uv->requires_attention = requires_attention;
    uv->size = size;
    uv->value = value;
    if(targetfile_begin==NULL)
    {
        targetfile_begin = uv;
    }
    if(targetfile_now!=NULL)
    {
        targetfile_now->next = uv;
    }
    targetfile_now = uv;
    label_defined = NULL;
}

int main(int argc,char** argv)
{
    
    //
    // show debugging information
    #ifdef DEBUG
    printf("DEBUG: Stapel compiler is compiled in debug mode!\n");
    printf("DEBUG: boot: argc: %d \n",argc);
    for(int i = 0 ; i < argc ; i++)
    {
        printf("DEBUG: arg[%d] : %s \n",i+1,argv[i]);
    }
    #endif 

    //
    // parse all the options
    char* inputfile = NULL;
    char* outputfile = NULL;
    for(int i = 1 ; i < argc ; i++)
    {
        char *command = argv[i];
        #ifdef DEBUG
        printf("DEBUG: Trying to parse instruction \"%s\" \n",command);
        #endif 
        if(strcmp(command,"--input")==0)
        {
            i++;
            command = argv[i];
            #ifdef DEBUG
            printf("DEBUG: Assigning inputfile to \"%s\" \n",command);
            #endif 
            inputfile = command;
        }
        else if(strcmp(command,"--output")==0)
        {
            i++;
            command = argv[i];
            #ifdef DEBUG
            printf("DEBUG: Assigning outputfile to \"%s\" \n",command);
            #endif 
            outputfile = command;
        }
        else
        {
            printf("ERROR: Unrecognised option %s !\n",command);
            exit(EXIT_FAILURE);
        }
    }

    if(inputfile==NULL)
    {
        printf("ERROR: Missing inputfile !\n");
        exit(EXIT_FAILURE);
    }

    if(outputfile==NULL)
    {
        printf("ERROR: Missing outputfile !\n");
        exit(EXIT_FAILURE);
    }

    FILE* file = fopen(inputfile,"r");
    if(!file)
    {
        printf("ERROR: Unable to open inputfile !\n");
        exit(EXIT_FAILURE);
    }

    
    //
    // converts the files into lines
    char *buffer = NULL;
    char *bufferbuffer = NULL;
    int linenumber = 1;
    while(1)
    {
        char u = getc(file);
        if(u==EOF)
        {
            break;
        }
        if(buffer==NULL)
        {
            buffer = calloc(1,0);
        }
        bufferbuffer = buffer;
        buffer = calloc(1,strlen(bufferbuffer) + 1);
        memcpy(buffer,bufferbuffer,strlen(bufferbuffer));
        free(bufferbuffer);
        ((char*)buffer)[strlen(buffer)] = 0;
        if(u=='\n')
        {
            #ifdef DEBUG
            printf("DEBUG: parsed line \"%s\" \n",buffer);
            #endif 
            SourceFileLine* tg = (SourceFileLine*) calloc(1,sizeof(SourceFileLine));
            tg->content = buffer;
            tg->filename = inputfile;
            tg->line = linenumber;
            tg->next = NULL;
            tg->tokens = NULL;
            if(sourcefile_begin==NULL)
            {
                sourcefile_begin = tg;
            }
            if(sourcefile_now!=NULL)
            {
                sourcefile_now->next = tg;
            }
            sourcefile_now = tg;
            buffer = NULL;
            linenumber += 1;
        }
        else
        {
            ((char*)buffer)[strlen(buffer)] = u;
        }
    }

    if(buffer!=NULL&&strlen(buffer))
    {
        #ifdef DEBUG
        printf("DEBUG: parsed line \"%s\" \n",buffer);
        #endif 
        SourceFileLine* tg = (SourceFileLine*) calloc(1,sizeof(SourceFileLine));
        tg->content = buffer;
        tg->filename = inputfile;
        tg->line = linenumber;
        tg->next = NULL;
        tg->tokens = NULL;
        if(sourcefile_begin==NULL)
        {
            sourcefile_begin = tg;
        }
        if(sourcefile_now!=NULL)
        {
            sourcefile_now->next = tg;
        }
        sourcefile_now = tg;
        buffer = NULL;
        linenumber += 1;
    }

    fclose(file);

    // 
    // converts lines into tokens
    buffer = NULL;
    bufferbuffer = NULL;
    SourceFileLine* loopnow = sourcefile_begin;
    while(1)
    {
        int is_string = 0;
        SourceFileLineToken *tok_last = NULL;
        int tokencount = 1;
        if(strlen(loopnow->content))
        {
            for(int i = 0 ; i < strlen(loopnow->content); i++)
            {
                char t = ((char*)loopnow->content)[i];
                if(buffer==NULL)
                {
                    buffer = calloc(1,0);
                }
                bufferbuffer = buffer;
                buffer = calloc(1,strlen(bufferbuffer) + 1);
                memcpy(buffer,bufferbuffer,strlen(bufferbuffer));
                free(bufferbuffer);
                ((char*)buffer)[strlen(buffer)] = 0;
                // ignore tabs
                if(t=='\t')
                {
                    continue;
                }
                // spaces could be a token splitter
                if(t==' '&&buffer!=NULL&&strlen(buffer)==0&&is_string==0)
                {
                    continue;
                }
                else if(t==' '&&buffer!=NULL&&strlen(buffer)>0&&is_string==0)
                {
                    goto gohere;
                }
                // "" could be a token splitter
                else if(t=='"')
                {
                    if(is_string)
                    {
                        is_string = 0;
                    }
                    else
                    {
                        is_string = 1;
                    }
                    if(is_string==1&&strlen(buffer)==0)
                    {
                        continue;
                    }
                    goto gohere;
                }
                else 
                {
                    ((char*)buffer)[strlen(buffer)] = t;
                }
                continue;
                gohere:
                SourceFileLineToken *tok = (SourceFileLineToken*) calloc(1,sizeof(SourceFileLineToken));
                tok->master = loopnow;
                tok->token = buffer;
                tok->token_id = tokencount++;
                if(loopnow->tokens==NULL)
                {
                    loopnow->tokens = tok;
                }
                else
                {
                    tok_last->next = tok;
                }
                tok_last = tok;
                #ifdef DEBUG
                printf("DEBUG: parsed token \"%s\" \n",buffer);
                #endif 
                buffer = NULL;

            }
            if(buffer!=NULL&&strlen(buffer)>0)
            {
                if(is_string==1)
                {
                    grammar_error_in_line(loopnow,"String is not closed!");
                }
                SourceFileLineToken *tok = (SourceFileLineToken*) calloc(1,sizeof(SourceFileLineToken));
                tok->master = loopnow;
                tok->token = buffer;
                tok->token_id = tokencount++;
                if(loopnow->tokens==NULL)
                {
                    loopnow->tokens = tok;
                }
                else
                {
                    tok_last->next = tok;
                }
                tok_last = tok;
                #ifdef DEBUG
                printf("DEBUG: parsed token \"%s\" \n",buffer);
                #endif 
                buffer = NULL;
            }
            loopnow->tokencount = tokencount-1;
        }
        else
        {
            loopnow->tokencount = 0;
        }
        if(loopnow->next==NULL)
        {
            break;
        }
        loopnow = loopnow->next;
    }

    #ifdef DEBUG
    printf("DEBUG: Time to show what we got!\n");
    loopnow = sourcefile_begin;
    while(1)
    {
        printf("DEBUG:\n\tfile\t\t:\t%s\n\tline\t\t:\t%d\n\tcode\t\t:\t\"%s\"\n\ttoken count\t:\t%d\n",loopnow->filename,loopnow->line,loopnow->content,loopnow->tokencount);
        printf("\ttokens\t\t:\n");
        if(loopnow->tokencount)
        {
            SourceFileLineToken *tok = loopnow->tokens;
            while(1)
            {
                printf("\t\t%i\t:\t%s\n",tok->token_id,tok->token);
                if(tok->next==NULL)
                {
                    break;
                }
                tok = tok->next;
            }
        }
        printf("\n");
        if(loopnow->next==NULL)
        {
            break;
        }
        loopnow = loopnow->next;
    }
    #endif 

    //
    // setup header
    // signature
    add_compiled_tree_value(0x53, sizeof(uint8_t) ,NULL, 0);
    add_compiled_tree_value(0x54, sizeof(uint8_t) ,NULL, 0);
    // version
    add_compiled_tree_value(1, sizeof(uint64_t) ,NULL, 0);
    // architecture
    add_compiled_tree_value(2, sizeof(uint8_t) ,NULL, 0);

    //
    // check for grammar mistakes
    loopnow = sourcefile_begin;
    while(1)
    {
        if(loopnow->tokencount>0)
        {
            SourceFileLineToken *tok = loopnow->tokens;
            if(strcmp(tok->token,"exit")==0)
            {
                add_compiled_tree_value(STAPEL_INSTRUCTION_EXIT,sizeof(uint8_t),NULL,0);
            }
            else if(strcmp(tok->token,"push")==0)
            {
                if(tok->next==NULL)
                {
                    grammar_error_in_token(tok,"\"address\",\"value_at\" or \"value\" expected after push statement");
                }
                tok = tok->next;
                if(strcmp(tok->token,"address")==0)
                {
                    if(tok->next==NULL)
                    {
                        grammar_error_in_token(tok,"variable required after \"push address\" statement");
                    }
                    tok = tok->next;
                    add_compiled_tree_value(STAPEL_INSTRUCTION_PUSH_RAW_ADDR,sizeof(uint8_t),NULL,0);
                    add_compiled_tree_value(0,sizeof(uint64_t),tok->token,1);
                }
                else if(strcmp(tok->token,"value_at")==0)
                {
                    if(tok->next==NULL)
                    {
                        grammar_error_in_token(tok,"integer value required after \"push value_at\" statement");
                    }
                    tok = tok->next;
                    add_compiled_tree_value(STAPEL_INSTRUCTION_PUSH_ADDRESS_VALUE,sizeof(uint8_t),NULL,0);
                    add_compiled_tree_value(0,sizeof(uint64_t),tok->token,1);
                }
                else if(strcmp(tok->token,"value")==0)
                {
                    if(tok->next==NULL)
                    {
                        grammar_error_in_token(tok,"integer value required after \"push value\" statement");
                    }
                    tok = tok->next;
                    add_compiled_tree_value(STAPEL_INSTRUCTION_PUSH_VALUE,sizeof(uint8_t),NULL,0);
                    add_compiled_tree_value(atoi(tok->token),sizeof(uint64_t),NULL,1);
                }
                else
                {
                    grammar_error_in_token(tok,"\"address\",\"value_at\" or \"value\" expected");
                }
            }
            else if(strcmp(tok->token,"debug")==0)
            {
                add_compiled_tree_value(STAPEL_INSTRUCTION_DEBUG,sizeof(uint8_t),NULL,0);
            }
            else if(strcmp(tok->token,"add")==0)
            {
                add_compiled_tree_value(STAPEL_INSTRUCTION_ADD,sizeof(uint8_t),NULL,0);
            }
            else if(strcmp(tok->token,"sub")==0)
            {
                add_compiled_tree_value(STAPEL_INSTRUCTION_SUB,sizeof(uint8_t),NULL,0);
            }
            else if(strcmp(tok->token,"mul")==0)
            {
                add_compiled_tree_value(STAPEL_INSTRUCTION_MUL,sizeof(uint8_t),NULL,0);
            }
            else if(strcmp(tok->token,"div")==0)
            {
                add_compiled_tree_value(STAPEL_INSTRUCTION_DIV,sizeof(uint8_t),NULL,0);
            }
            else if(strcmp(tok->token,"call")==0)
            {
                if(tok->next==NULL)
                {
                    grammar_error_in_token(tok,"expected variable name after \"call\" statement");
                }
                tok = tok->next;
                add_compiled_tree_value(STAPEL_INSTRUCTION_CALL,sizeof(uint8_t),NULL,0);
                add_compiled_tree_value(0,sizeof(uint64_t),tok->token,1);
            }
            else if(strcmp(tok->token,"jump")==0)
            {
                if(tok->next==NULL)
                {
                    grammar_error_in_token(tok,"expected variable name after \"jump\" statement");
                }
                tok = tok->next;
                add_compiled_tree_value(STAPEL_INSTRUCTION_JUMP,sizeof(uint8_t),NULL,0);
                add_compiled_tree_value(0,sizeof(uint64_t),tok->token,1);
            }
            else if(strcmp(tok->token,"int")==0)
            {
                add_compiled_tree_value(STAPEL_INSTRUCTION_INT,sizeof(uint8_t),NULL,0);
            }
            else if(strcmp(tok->token,"ret")==0)
            {
                add_compiled_tree_value(STAPEL_INSTRUCTION_RET,sizeof(uint8_t),NULL,0);
            }
            else if(strcmp(tok->token,"pop")==0)
            {
                if(tok->next==NULL)
                {
                    grammar_error_in_token(tok,"expected variable name after \"pop\" statement");
                }
                tok = tok->next;
                add_compiled_tree_value(STAPEL_INSTRUCTION_POP,sizeof(uint8_t),NULL,0);
                add_compiled_tree_value(0,sizeof(uint64_t),tok->token,1);
            }
            else if(strcmp(tok->token,"label")==0)
            {
                if(tok->next==NULL)
                {
                    grammar_error_in_token(tok,"name of label expected after \"label\" statement");
                }
                tok = tok->next;
                label_defined = tok->token;
            }
            else
            {
                grammar_error_in_token(tok,"Cannot understand token");
            }
            if(tok->next!=NULL)
            {
                grammar_error_in_token(tok,"Garbage after this token");
            }
        }
        if(loopnow->next==NULL)
        {
            break;
        }
        loopnow = loopnow->next;
    }

    //
    // Do we need to link something?

    //
    // Build file
    FILE* targetfile = fopen(outputfile,"w");
    CompiledTreeValue* looplater = targetfile_begin;
    while(1)
    {
        fwrite((void*)&looplater->value,1,looplater->size,targetfile);
        if(looplater->next==NULL)
        {
            break;
        }
        looplater = looplater->next;
    }
    fclose(targetfile);

    exit(EXIT_SUCCESS);
}