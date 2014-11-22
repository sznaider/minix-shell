#include "parser.h"
static Parser g_parser;

void parser_parse_line(const char* input)
{
    g_parser.parsed_line = parseline((char*)input);
    g_parser.pipeline = 0;
    g_parser.command = 0;

    Pipeline pipeline;
    Command* command;
    while ((pipeline = parser_get_pipeline()))
    {
        while ((command = parser_get_command()))
        {
            command->argc = 0;
            for (int i = 0; command->argv[i] != NULL; i++)
                ++command->argc; 

            command->input.type  = STREAM_STDIN;
            command->output.type = STREAM_STDOUT;

            if (parser_has_next_command())
            {
//                printf("%s has next\n", command->argv[0]);
                command->output.type = STREAM_PIPE_IN;
            }
            if (parser_has_prev_command())
            {
  //              printf("%s has prev\n", command->argv[0]);
                command->input.type  = STREAM_PIPE_OUT;
            }
            
            for (int i = 0; command->redirs[i] != NULL; i++)
            {
                Redirection* r = command->redirs[i];

                if (IS_RAPPEND(r->flags))
                {
                    command->output.type = STREAM_FILE_APPEND;
                    command->output.file = r->filename;
                } 
                else if (IS_ROUT(r->flags))
                {
                    command->output.type = STREAM_FILE;
                    command->output.file = r->filename;
                }
                else if (IS_RIN(r->flags))
                {
                    command->input.type = STREAM_FILE;
                    command->input.file = r->filename;
                }
            }
            parser_next_command();
        }
        parser_next_pipeline();
    }
    g_parser.pipeline = 0;
    g_parser.command = 0;
}

Pipeline parser_get_pipeline()
{
    return g_parser.parsed_line->pipelines[g_parser.pipeline];
}

Command* parser_get_command()
{
    return parser_get_pipeline()[g_parser.command];
}

void parser_next_pipeline()
{
    if (parser_get_pipeline())
    {
        g_parser.command = 0;
        g_parser.pipeline++;
    }
}

void parser_next_command()
{
    if (parser_get_command())
        g_parser.command++;
}

bool parser_has_next_command()
{
    return parser_get_command() && parser_get_pipeline()[g_parser.command + 1];
}

bool parser_has_prev_command()
{
    return g_parser.command && parser_get_pipeline()[g_parser.command - 1];
}

