#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

SourceFileLine* sourcefile_begin = NULL;
SourceFileLine* sourcefile_now = NULL;

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
        printf("\n");
        if(loopnow->next==NULL)
        {
            break;
        }
        loopnow = loopnow->next;
    }
    #endif 

    exit(EXIT_SUCCESS);
}