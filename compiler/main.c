#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct{
    char* filename;
    int line;
    char* content;
    void *next;
} SourceFileLine;

SourceFileLine* sourcefile_begin = NULL;
SourceFileLine* sourcefile_now = NULL;

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
    // converts the files into stuff
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

    #ifdef DEBUG
    printf("DEBUG: Time to show what we got!\n");
    SourceFileLine* loopnow = sourcefile_begin;
    while(1)
    {
        printf("DEBUG:\n\tfile:\t%s\n\tline:\t%d\n\tcode:\t\"%s\"\n\n",loopnow->filename,loopnow->line,loopnow->content);
        if(loopnow->next==NULL)
        {
            break;
        }
        loopnow = loopnow->next;
    }
    #endif 

    exit(EXIT_SUCCESS);
}