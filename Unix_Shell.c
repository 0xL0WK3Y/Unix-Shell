#include "Unix_Shell.h"

int main(int argc, char** argv)
{
    char* buffer; 
    char** b_args; 
    int status;

    char* c_input[128]; 

    char* user;

    while(1)
    {
        user = getenv("USER");
        printf("%s@L0WK3Y>", user);

        buffer = read_line();

        if(strcmp(buffer, "exit") == 0)
        {
            return 0;
        }

        b_args = parse_buffer(buffer); 
        status = execute_arguments(b_args,0);

        free(buffer);
        buffer = NULL;

        free(b_args);
        b_args = NULL;
    }


    return 0;
}

char** parse_buffer(char* buffer)
{
    int buffer_size = 64;
    int position = 0;
    int i = 0;
    int j = 0;

    char* name = calloc(sizeof(char), strlen(buffer));
    char** tokens = calloc(sizeof(char*), 64);
    char* token;

    if(!tokens)
    {
        fprintf(stderr, "Allocation Error\n");
        exit(EXIT_FAILURE);
    }
    if(!name)
    {
        fprintf(stderr, "Allocation Error\n");
        exit(EXIT_FAILURE);
    }

    for(i = 0; i < strlen(buffer); i++)
    {
        if(buffer[i] == '>' && buffer[i + 1] == '>')
        {
            while(i + 3 < strlen(buffer))
            {
                name[j] = buffer[i + 3];
                j++;
                i++;
            }
            buffer[strlen(buffer) - j -4] = '\0';
            file_write(name, buffer);
            tokens[0] = NULL;
            return tokens;
        }

        if(buffer[i] == '>' && buffer[i + 1] == ' ')
        {
            while(i + 2 < strlen(buffer))
            {
                name[j] = buffer[i + 3];
                j++;
                i++;
            }
            buffer[strlen(buffer) - j - 4] = '\0';
            file_overwrite(name, buffer);
            tokens[0] = NULL;
            return tokens;
        }
        
        if(buffer[i] == '|')
        {
            tokenize_pipe(buffer);
            tokens[0] = NULL;
            return tokens;
        }
    }

    token = strtok(buffer, " ");

    while(token != NULL)
    {
        tokens[position] = token;
        position++;

        if(position >= buffer_size)
        {
            buffer_size += 64;
            tokens = realloc(tokens, buffer_size * sizeof(char*));

            if(!tokens)
            {
                fprintf(stderr, "Allocation Error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, " ");
    }

    tokens[position] = NULL;
    return tokens;
}

char* read_line()
{
    int buffer_size = 1024;
    int position = 0;
    char* buffer = calloc(sizeof(char) * buffer_size, 1);
    int chr;

    if(!buffer)
    {
        fprintf(stderr, "Allocation Error\n");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        chr = getchar();

        if(chr == EOF || chr == '\n')
        {
            buffer[position] = '\0';
            return buffer;
        }
        else
        {
            buffer[position] = chr;
        }
        
        position++;

        if(position >= buffer_size)
        {
            buffer_size += 1024;
            buffer = realloc(buffer, buffer_size);
 
            if(!buffer)
            {
                fprintf(stderr, "Allocation Error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

int execute_arguments(char** args, int exec_value)
{

    if (args[exec_value] == NULL)
    {
        return 1;
    }

    if (strcmp(args[exec_value], "cd") == 0)
    {
        return cd_command(args, 1);
    }

    if (strcmp(args[exec_value], "exit") == 0)
    {
        exit(EXIT_SUCCESS);
    }

    return start_process(args, exec_value);

}

int cd_command(char** args, int check_value)
{
    if(args[check_value] == NULL)
    {
        fprintf(stderr, "Error changing directory!\n");
    }
    else
    {
        if(chdir(args[check_value]) != 0)
        {
            perror("Error changing directory!\n");
        }
    }

    return 1;
}

int start_process(char** args, int exec_value)
{
    pid_t pid;

    pid = fork();

    if(!pid)
    {
        if(execvp(args[exec_value], args) == -1)
        {
            perror("A process error occured!\n");
            return 0;
        }

        exit(EXIT_FAILURE);
    }
    else if(pid < 0)
    {
        perror("Forking process failed!\n");
        return 0;
    }
    else
    {
        waitpid(pid, NULL, 0);
    }
    
    return 1;
}

void file_write(char* name, char* buffer)
{
    
     char** args;
    int status;
    int save_out;
    int outfd = open(name, O_RDWR | O_CREAT | O_APPEND, 0600);

    if(outfd < 0)
    {
        fprintf(stderr, "Failed opening %s", name);
        return;
    }

    save_out = dup(STDOUT_FILENO);

    if(dup2(outfd, STDOUT_FILENO) < 0)
    {
        fprintf(stderr, "Failed while directing stream.\n");
        return;
    }

    args = parse_buffer(buffer);
    status = execute_arguments(args,0);

    fflush(stdout);
    close(outfd);

    dup2(save_out, STDOUT_FILENO);

    close(save_out);
}

void file_overwrite(char* name, char* buffer)
{
    char** args;
    int status;
    int save_out;
    int outfd = open(name, O_RDWR | O_CREAT | O_TRUNC, 644);

    if(outfd < 0)
    {
        fprintf(stderr, "Failed opening %s", name);
        return;
    }

    save_out = dup(STDOUT_FILENO);

    if(dup2(outfd, STDOUT_FILENO) < 0)
    {
        fprintf(stderr, "Failed while directing stream.\n");
        return;
    }

    args = parse_buffer(buffer);
    status = execute_arguments(args,0);

    fflush(stdout);
    close(outfd);

    dup2(save_out, STDOUT_FILENO);

    close(save_out);
}

void tokenize_pipe(char* buffer)
{
    char* token;
    int buffer_size = 64;
    char** tokens = calloc(buffer_size * sizeof(char*), 1);
    char** p_tokens = calloc(buffer_size * sizeof(char*),1);
    int position = 0;
    int pipe_index = 0;
    int stop_flag = 0;

    token = strtok(buffer, " "); 
    while(token != NULL)
    {
        if(strcmp(token, "|") != 0)
        {
            if(!stop_flag)
            {
                tokens[position] = token;
                position++;
            }
            else
            {
                p_tokens[pipe_index] = token;
                pipe_index++;
            }
            
        }
        else
        {
            stop_flag = 1;
        }

        if(position >= buffer_size || pipe_index >= buffer_size)
        {
            buffer_size = 64;
            tokens = realloc(tokens, buffer_size * sizeof(char*));
            p_tokens = realloc(p_tokens, buffer_size * sizeof(char*));

            if(!tokens || !p_tokens)
            {
                fprintf(stderr, "Allocation Error\n");
                exit(EXIT_FAILURE);
            }
        }
        
        token = strtok(NULL, " ");
    }

    tokens[position] = NULL;
    p_tokens[pipe_index] = NULL;

    pipe_func(tokens, p_tokens);
}

void pipe_func(char** buffer, char** p_buffer)
{
    int pipefd[2];
    pid_t p1;
    int save_output;

    if(pipe(pipefd) < 0)
    {
        printf("Piping process failed");
        return;
    }

    p1 = fork();

    if(!p1)
    {
        save_output = dup(STDOUT_FILENO);
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);

        if(start_process(buffer, 0) == 0)
        {
            perror("An error occured!\n");
            return;
        }
        close(pipefd[1]);
        dup2(save_output, STDOUT_FILENO);
        close(save_output);
    }
    else if(p1 < 0)
    {
        printf("Forking process failed!\n");
        return;
    }
    else
    {
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);

        if(start_process(p_buffer, 0) == 0)
        {
            fprintf(stderr, "Failed to execute the next command.\n");
            return;
        }
        
        waitpid(p1, NULL, 0);
    }
    
}
