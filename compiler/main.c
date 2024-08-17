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
    SourceFileLineToken* connected_token;
} CompiledTreeValue;

SourceFileLine* sourcefile_begin = NULL;
SourceFileLine* sourcefile_now = NULL;

CompiledTreeValue* targetfile_begin = NULL;
CompiledTreeValue* targetfile_now = NULL;

int find_location_of_label(char* label){
    int innercount = 0;
    CompiledTreeValue* looplater = targetfile_begin;
    while(1)
    {
        if(looplater->label_defined!=NULL&&strcmp(label,looplater->label_defined)==0)
        {
            return innercount;
        }
        innercount += looplater->size;
        if(looplater->next==NULL)
        {
            break;
        }
        looplater = looplater->next;
    }
    return -1;
}

int grammar_error_in_token(SourceFileLineToken* token,char* message)
{
    printf("FATAL: An error occured while compiling:\n");
    printf("       File   :   %s\n",token->master->filename);
    printf("       LineNo :   %d\n",token->master->line);
    printf("       Line   :   %s\n",token->master->content);
    printf("       TokenAt:   %d\n",token->token_id);
    printf("       Token  :   %s\n",token->token);
    printf("       Message:   %s\n",message);
    return EXIT_FAILURE;
}

int grammar_error_in_line(SourceFileLine* token,char* message)
{
    printf("FATAL: An error occured while compiling:\n");
    printf("       File   :   %s\n",token->filename);
    printf("       LineNo :   %d\n",token->line);
    printf("       Line   :   %s\n",token->content);
    printf("       Message:   %s\n",message);
    return EXIT_FAILURE;
}

char *label_defined = NULL;

void add_compiled_tree_value(uint64_t value, int size, char* label_required, int requires_attention,SourceFileLineToken* connected_token)
{
    CompiledTreeValue* uv = (CompiledTreeValue*) malloc(sizeof(CompiledTreeValue));
    uv->label_defined = label_defined;
    uv->label_required = label_required;
    uv->next = NULL;
    uv->requires_attention = requires_attention;
    uv->size = size;
    uv->value = value;
    uv->connected_token = connected_token;
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

void fixstring(char* str)
{
    int sz = strlen(str);
    int changes = 0;
    int pointer = 0;
    while(sz!=pointer){
        char deze = str[pointer];
        if(changes&&(pointer+1)!=sz){
            str[pointer] = str[pointer+1];
        }else if(changes){
            str[pointer] = 0;
        }else if(str[pointer]=='\\'){
            changes = 1;
            switch(str[pointer+1]){
                case '\\':
                    str[pointer] = '\\';
                    changes = 2;
                    break;
                case 'n':
                    str[pointer] = '\n';
                    break;
                case 't':
                    str[pointer] = '\t';
                    break;
            }
        }
        pointer++;
    }
    if(changes == 1){
        fixstring(str);
    }
}

int main(int argc,char** argv)
{

    //
    // parse all the options
    char* inputfile = NULL;
    char* outputfile = NULL;
    uint8_t debug = 0;
    for(int i = 1 ; i < argc ; i++)
    {
        char *command = argv[i];
        if(strcmp(command,"--input")==0)
        {
            i++;
            command = argv[i];
            inputfile = command;
        }
        else if(strcmp(command,"--output")==0)
        {
            i++;
            command = argv[i];
            outputfile = command;
        }
        else if(strcmp(command,"--verbose")==0)
        {
            debug = 1;
        }
        else
        {
            printf("ERROR: Unrecognised option %s !\n",command);
            return EXIT_FAILURE;
        }
    }

    if(inputfile==NULL)
    {
        printf("ERROR: Missing inputfile !\n");
        return EXIT_FAILURE;
    }

    if(outputfile==NULL)
    {
        printf("ERROR: Missing outputfile !\n");
        return EXIT_FAILURE;
    }

    FILE* file = fopen(inputfile,"r");
    if(!file)
    {
        printf("ERROR: Unable to open inputfile !\n");
        return EXIT_FAILURE;
    }

    
    //
    // converts the files into lines
    char *buffer = NULL;
    char *bufferbuffer = NULL;
    int linenumber = 1;
    int wordcount = 0;
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
        buffer = calloc(1,wordcount + 1);
        memcpy(buffer,bufferbuffer,wordcount);
        free(bufferbuffer);
        ((char*)buffer)[wordcount] = 0;
        if(u=='\n')
        {
            if(debug)
            {
                printf("DEBUG: parsed line \"%s\" \n",buffer);
            }
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
            wordcount = 0;
        }
        else
        {
            ((char*)buffer)[wordcount] = u;
            ((char*)buffer)[wordcount+1] = 0;
            wordcount++;
        }
    }

    if(buffer!=NULL&&wordcount)
    {
        if(debug)
        {
            printf("DEBUG: parsed line \"%s\" \n",buffer);
        }
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
    wordcount = 0;

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
        wordcount = 0;
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
                buffer = calloc(1,wordcount + 1);
                memcpy(buffer,bufferbuffer,wordcount);
                free(bufferbuffer);
                ((char*)buffer)[wordcount] = 0;
                // ignore tabs
                if(t=='\t')
                {
                    continue;
                }
                // spaces could be a token splitter
                if(t==' '&&buffer!=NULL&&wordcount==0&&is_string==0)
                {
                    continue;
                }
                else if(t==' '&&buffer!=NULL&&wordcount>0&&is_string==0)
                {
                    goto gohere;
                }
                // "" could be a token splitter
                else if(t=='"')
                {
                    if(is_string)
                    {
                        fixstring(buffer);
                        is_string = 0;
                    }
                    else
                    {
                        is_string = 1;
                    }
                    if(is_string==1&&wordcount==0)
                    {
                        continue;
                    }
                    goto gohere;
                }
                else 
                {
                    ((char*)buffer)[wordcount] = t;
                    ((char*)buffer)[wordcount + 1] = 0;
                    wordcount++;
                }
                continue;
                SourceFileLineToken *tok;
                gohere:
                tok = (SourceFileLineToken*) calloc(1,sizeof(SourceFileLineToken));
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
                if(debug){
                    printf("DEBUG: parsed token \"%s\" \n",buffer);
                }
                buffer = NULL;
                wordcount = 0;

            }
            if(buffer!=NULL&&strlen(buffer)>0)
            {
                if(is_string==1)
                {
                    return grammar_error_in_line(loopnow,"String is not closed!");
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
                if(debug)
                {
                    printf("DEBUG: parsed token \"%s\" \n",buffer);
                }
                buffer = NULL;
                wordcount = 0;
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

    if(debug)
    {
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
    }

    //
    // setup header
    // signature
    add_compiled_tree_value(STAPEL_HEADER_SIGNATURE_A, sizeof(uint8_t) ,NULL, 0, NULL);
    add_compiled_tree_value(STAPEL_HEADER_SIGNATURE_B, sizeof(uint8_t) ,NULL, 0, NULL);
    // version
    add_compiled_tree_value(STAPEL_HEADER_VERSION, sizeof(uint64_t) ,NULL, 0, NULL);
    // architecture
    add_compiled_tree_value(STAPEL_HEADER_ARCHITECTURE, sizeof(uint8_t) ,NULL, 0, NULL);

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
                add_compiled_tree_value(STAPEL_INSTRUCTION_EXIT,sizeof(uint8_t),NULL,0, tok);
            }
            else if(strcmp(tok->token,"push")==0)
            {
                if(tok->next==NULL)
                {
                    #if STAPEL_HEADER_VERSION > 1
                        return grammar_error_in_token(tok,"\"address\",\"64value_at\",\"64value\",\"8value_at\",\"8value\",\"16value_at\" or \"16value\" expected after push statement");
                    #else 
                        return grammar_error_in_token(tok,"\"address\",\"value_at\" or \"value\" expected after push statement");
                    #endif 
                }
                tok = tok->next;
                if(strcmp(tok->token,"address")==0)
                {
                    if(tok->next==NULL)
                    {
                        return grammar_error_in_token(tok,"variable required after \"push address\" statement");
                    }
                    tok = tok->next;
                    add_compiled_tree_value(STAPEL_INSTRUCTION_PUSH_RAW_ADDR,sizeof(uint8_t),NULL,0, tok);
                    add_compiled_tree_value(0,sizeof(uint64_t),tok->token,1, tok);
                }
                #if STAPEL_HEADER_VERSION > 1
                else if(strcmp(tok->token,"64value_at")==0)
                {
                    if(tok->next==NULL)
                    {
                        return grammar_error_in_token(tok,"integer value required after \"push value_at\" statement");
                    }
                    tok = tok->next;
                    add_compiled_tree_value(STAPEL_INSTRUCTION_PUSH_ADDRESS_VALUE_64,sizeof(uint8_t),NULL,0, tok);
                    add_compiled_tree_value(0,sizeof(uint64_t),tok->token,1, tok);
                }
                else if(strcmp(tok->token,"64value")==0)
                {
                    if(tok->next==NULL)
                    {
                        return grammar_error_in_token(tok,"integer value required after \"push value\" statement");
                    }
                    tok = tok->next;
                    add_compiled_tree_value(STAPEL_INSTRUCTION_PUSH_VALUE_64,sizeof(uint8_t),NULL,0, tok);
                    add_compiled_tree_value(atoi(tok->token),sizeof(uint64_t),NULL,0, tok);
                }
                else if(strcmp(tok->token,"8value_at")==0)
                {
                    if(tok->next==NULL)
                    {
                        return grammar_error_in_token(tok,"integer value required after \"push 8value_at\" statement");
                    }
                    tok = tok->next;
                    add_compiled_tree_value(STAPEL_INSTRUCTION_PUSH_ADDRESS_VALUE_8,sizeof(uint8_t),NULL,0, tok);
                    add_compiled_tree_value(0,sizeof(uint64_t),tok->token,1, tok);
                }
                else if(strcmp(tok->token,"8value")==0)
                {
                    if(tok->next==NULL)
                    {
                        return grammar_error_in_token(tok,"integer value required after \"push 8value\" statement");
                    }
                    tok = tok->next;
                    add_compiled_tree_value(STAPEL_INSTRUCTION_PUSH_VALUE_8,sizeof(uint8_t),NULL,0, tok);
                    add_compiled_tree_value(atoi(tok->token),sizeof(uint8_t),NULL,0, tok);
                }
                else if(strcmp(tok->token,"16value_at")==0)
                {
                    if(tok->next==NULL)
                    {
                        return grammar_error_in_token(tok,"integer value required after \"push 16value_at\" statement");
                    }
                    tok = tok->next;
                    add_compiled_tree_value(STAPEL_INSTRUCTION_PUSH_ADDRESS_VALUE_16,sizeof(uint8_t),NULL,0, tok);
                    add_compiled_tree_value(0,sizeof(uint64_t),tok->token,1, tok);
                }
                else if(strcmp(tok->token,"16value")==0)
                {
                    if(tok->next==NULL)
                    {
                        return grammar_error_in_token(tok,"integer value required after \"push 16value\" statement");
                    }
                    tok = tok->next;
                    add_compiled_tree_value(STAPEL_INSTRUCTION_PUSH_VALUE_16,sizeof(uint8_t),NULL,0, tok);
                    add_compiled_tree_value(atoi(tok->token),sizeof(uint16_t),NULL,0, tok);
                }
                #else 
                else if(strcmp(tok->token,"value_at")==0)
                {
                    if(tok->next==NULL)
                    {
                        return grammar_error_in_token(tok,"integer value required after \"push value_at\" statement");
                    }
                    tok = tok->next;
                    add_compiled_tree_value(STAPEL_INSTRUCTION_PUSH_ADDRESS_VALUE_64,sizeof(uint8_t),NULL,0, tok);
                    add_compiled_tree_value(0,sizeof(uint64_t),tok->token,1, tok);
                }
                else if(strcmp(tok->token,"value")==0)
                {
                    if(tok->next==NULL)
                    {
                        return grammar_error_in_token(tok,"integer value required after \"push value\" statement");
                    }
                    tok = tok->next;
                    add_compiled_tree_value(STAPEL_INSTRUCTION_PUSH_VALUE_64,sizeof(uint8_t),NULL,0, tok);
                    add_compiled_tree_value(atoi(tok->token),sizeof(uint64_t),NULL,0, tok);
                }
                #endif 
                else
                {
                    return grammar_error_in_token(tok,"\"address\",\"value_at\" or \"value\" expected");
                }
            }
            else if(strcmp(tok->token,"debug")==0)
            {
                add_compiled_tree_value(STAPEL_INSTRUCTION_DEBUG,sizeof(uint8_t),NULL,0, tok);
            }
            else if(strcmp(tok->token,"add")==0)
            {
                add_compiled_tree_value(STAPEL_INSTRUCTION_ADD,sizeof(uint8_t),NULL,0, tok);
            }
            else if(strcmp(tok->token,"sub")==0)
            {
                add_compiled_tree_value(STAPEL_INSTRUCTION_SUB,sizeof(uint8_t),NULL,0, tok);
            }
            else if(strcmp(tok->token,"mul")==0)
            {
                add_compiled_tree_value(STAPEL_INSTRUCTION_MUL,sizeof(uint8_t),NULL,0, tok);
            }
            else if(strcmp(tok->token,"div")==0)
            {
                add_compiled_tree_value(STAPEL_INSTRUCTION_DIV,sizeof(uint8_t),NULL,0, tok);
            }
            else if(strcmp(tok->token,"call")==0)
            {
                if(tok->next==NULL)
                {
                    return grammar_error_in_token(tok,"expected variable name after \"call\" statement");
                }
                tok = tok->next;
                add_compiled_tree_value(STAPEL_INSTRUCTION_CALL,sizeof(uint8_t),NULL,0, tok);
                add_compiled_tree_value(0,sizeof(uint64_t),tok->token,1, tok);
            }
            else if(strcmp(tok->token,"jump")==0)
            {
                #if STAPEL_HEADER_VERSION > 1
                    if(tok->next==NULL)
                    {
                        return grammar_error_in_token(tok,"expected reason after \"jump\" statement (directly or equals)");
                    }
                    tok = tok->next;
                    if(strcmp(tok->token,"directly")==0)
                    {
                        if(tok->next==NULL)
                        {
                            return grammar_error_in_token(tok,"expected variable name after \"jump directly\" statement");
                        }
                        tok = tok->next;
                        add_compiled_tree_value(STAPEL_INSTRUCTION_JUMP,sizeof(uint8_t),NULL,0, tok);
                        add_compiled_tree_value(0,sizeof(uint64_t),tok->token,1, tok);
                    }
                    else if(strcmp(tok->token,"equals")==0)
                    {
                        if(tok->next==NULL)
                        {
                            return grammar_error_in_token(tok,"expected variable name after \"jump equals\" statement");
                        }
                        tok = tok->next;
                        add_compiled_tree_value(STAPEL_INSTRUCTION_JUMP_EQUALS,sizeof(uint8_t),NULL,0, tok);
                        add_compiled_tree_value(0,sizeof(uint64_t),tok->token,1, tok);
                    }
                    else
                    {
                        return grammar_error_in_token(tok,"The command \"jump\" should be followed by \"directly\" or \"equals\"");
                    }
                #else 
                    if(tok->next==NULL)
                    {
                        return grammar_error_in_token(tok,"expected variable name after \"jump\" statement");
                    }
                    tok = tok->next;
                    add_compiled_tree_value(STAPEL_INSTRUCTION_JUMP,sizeof(uint8_t),NULL,0, tok);
                    add_compiled_tree_value(0,sizeof(uint64_t),tok->token,1, tok);
                #endif 
            }
            else if(strcmp(tok->token,"int")==0)
            {
                add_compiled_tree_value(STAPEL_INSTRUCTION_INT,sizeof(uint8_t),NULL,0, tok);
            }
            else if(strcmp(tok->token,"ret")==0)
            {
                add_compiled_tree_value(STAPEL_INSTRUCTION_RET,sizeof(uint8_t),NULL,0, tok);
            }
            else if(strcmp(tok->token,"pop")==0)
            {
                #if STAPEL_HEADER_VERSION > 1
                if(tok->next==NULL)
                {
                    return grammar_error_in_token(tok,"expected  \"64value\",\"8value\",\"64value_at\",\"16value\",\"16value_at\" or \"8value_at\" after pop statement");
                }
                tok = tok->next;
                if(strcmp(tok->token,"64value")==0)
                {
                    if(tok->next==NULL)
                    {
                        return grammar_error_in_token(tok,"integer value required after \"push value\" statement");
                    }
                    tok = tok->next;
                    add_compiled_tree_value(STAPEL_INSTRUCTION_POP_64,sizeof(uint8_t),NULL,0, tok);
                    add_compiled_tree_value(0,sizeof(uint64_t),tok->token,1, tok);
                }
                else if(strcmp(tok->token,"8value")==0)
                {
                    if(tok->next==NULL)
                    {
                        return grammar_error_in_token(tok,"integer value required after \"push 8value\" statement");
                    }
                    tok = tok->next;
                    add_compiled_tree_value(STAPEL_INSTRUCTION_POP_8,sizeof(uint8_t),NULL,0, tok);
                    add_compiled_tree_value(0,sizeof(uint64_t),tok->token,1, tok);
                }
                else if(strcmp(tok->token,"16value")==0)
                {
                    if(tok->next==NULL)
                    {
                        return grammar_error_in_token(tok,"integer value required after \"push 16value\" statement");
                    }
                    tok = tok->next;
                    add_compiled_tree_value(STAPEL_INSTRUCTION_POP_16,sizeof(uint8_t),NULL,0, tok);
                    add_compiled_tree_value(0,sizeof(uint64_t),tok->token,1, tok);
                }
                if(strcmp(tok->token,"64value_at")==0)
                {
                    if(tok->next==NULL)
                    {
                        return grammar_error_in_token(tok,"integer value required after \"push 64value_at\" statement");
                    }
                    tok = tok->next;
                    add_compiled_tree_value(STAPEL_INSTRUCTION_POP_AT_64,sizeof(uint8_t),NULL,0, tok);
                    add_compiled_tree_value(0,sizeof(uint64_t),tok->token,1, tok);
                }
                else if(strcmp(tok->token,"8value_at")==0)
                {
                    if(tok->next==NULL)
                    {
                        return grammar_error_in_token(tok,"integer value required after \"push 8value_at\" statement");
                    }
                    tok = tok->next;
                    add_compiled_tree_value(STAPEL_INSTRUCTION_POP_AT_8,sizeof(uint8_t),NULL,0, tok);
                    add_compiled_tree_value(0,sizeof(uint64_t),tok->token,1, tok);
                }
                else if(strcmp(tok->token,"16value_at")==0)
                {
                    if(tok->next==NULL)
                    {
                        return grammar_error_in_token(tok,"integer value required after \"push 16value_at\" statement");
                    }
                    tok = tok->next;
                    add_compiled_tree_value(STAPEL_INSTRUCTION_POP_AT_16,sizeof(uint8_t),NULL,0, tok);
                    add_compiled_tree_value(0,sizeof(uint64_t),tok->token,1, tok);
                }
                #else 
                if(tok->next==NULL)
                {
                    return grammar_error_in_token(tok,"expected variable name after \"pop\" statement");
                }
                tok = tok->next;
                add_compiled_tree_value(STAPEL_INSTRUCTION_POP_64,sizeof(uint8_t),NULL,0, tok);
                add_compiled_tree_value(0,sizeof(uint64_t),tok->token,1, tok);
                #endif 
            }
            else if(strcmp(tok->token,"label")==0)
            {
                if(tok->next==NULL)
                {
                    return grammar_error_in_token(tok,"name of label expected after \"label\" statement");
                }
                tok = tok->next;
                label_defined = tok->token;
            }
            else if(strcmp(tok->token,"dump")==0)
            {
                if(tok->next==NULL)
                {
                    return grammar_error_in_token(tok,"type \"text\" or \"number\" expected after dump");
                }
                tok = tok->next;
                if(strcmp(tok->token,"text")==0)
                {
                    if(tok->next==NULL)
                    {
                        return grammar_error_in_token(tok,"text value expected");
                    }
                    tok = tok->next;
                    int strl = strlen(tok->token);
                    for(int i = 0 ; i < strl ; i++)
                    {
                        add_compiled_tree_value(((uint8_t*)tok->token)[i],sizeof(uint8_t),NULL,0, tok);
                    }
                    add_compiled_tree_value(0,sizeof(uint8_t),NULL,0, tok);
                }
                else if(strcmp(tok->token,"number")==0)
                {
                    if(tok->next==NULL)
                    {
                        return grammar_error_in_token(tok,"number value expected");
                    }
                    tok = tok->next;
                    add_compiled_tree_value(atoi(tok->token),sizeof(uint64_t),NULL,0, tok);
                }
                else
                {
                    return grammar_error_in_token(tok,"\"text\" or \"number\" expected");
                }
            }
            #if STAPEL_HEADER_VERSION > 1
            else if(strcmp(tok->token,"syscall")==0)
            {
                if(tok->next==NULL)
                {
                    return grammar_error_in_token(tok,"expected number after \"syscall\" statement");
                }
                tok = tok->next;
                add_compiled_tree_value(STAPEL_INSTRUCTION_SYSCALL,sizeof(uint8_t),NULL,0, tok);
                add_compiled_tree_value(atoi(tok->token),sizeof(uint8_t),NULL,0, tok);
            }
            #endif 
            else
            {
                return grammar_error_in_token(tok,"Cannot understand token");
            }
            if(tok->next!=NULL)
            {
                return grammar_error_in_token(tok,"Garbage after this token");
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
    CompiledTreeValue* looplater = targetfile_begin;
    while(1)
    {
        if(looplater->requires_attention==1)
        {
            if(debug)
            {
                printf("DEBUG: line requires attention %s \n",looplater->label_required);
            }
            int r = find_location_of_label(looplater->label_required);
            if(r==-1)
            {
                return grammar_error_in_token(looplater->connected_token,"Unable to find token");
            }
            looplater->value = r;
            looplater->requires_attention = 0;
        }
        if(looplater->next==NULL)
        {
            break;
        }
        looplater = looplater->next;
    }

    //
    // Build file
    FILE* targetfile = fopen(outputfile,"w");
    looplater = targetfile_begin;
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

    return EXIT_SUCCESS;
}