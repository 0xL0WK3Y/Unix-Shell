#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

char** parse_buffer(char* buffer);
char* read_line();
int execute_arguments(char** args, int exec_value);
int cd_command(char** args, int check_value);
int start_process(char** args, int exec_value);
void file_write(char* name, char* buffer);
void file_overwrite(char* name, char* buffer);
void tokenize_pipe(char* buffer);
void pipe_func(char** buffer, char** p_buffer);