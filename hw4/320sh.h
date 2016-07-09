#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#define DIR_UP 1
#define DIR_DOWN 0
#define DIR_UNINIT -1

struct node_list{
  char *data;
  struct node_list *link;
  struct node_list *prev;
};
typedef struct node_list node;

void escape();

int pwd();

int cd();

int cdLast();

int cdAddress(char * cmdValue);

void remove_newline(char *str_path);

pid_t Fork(void);

int run_program(char *cmdValue, char **environ);

int setVar(char * cmdValue);

int echoFunction(char *cmdValue);

int cmdCheck(char *cmd, char **envp);

void trimString(char **string);

int creat_history(node **A_link, char *cmd );

int write_on_screen(node *A_link);

int clean_history(node *A_link);

void free_list(node *ptr);

int print_list_to_file(node *A_link, FILE *file);

int backspace(char *cmdToBackspace, char *totalHeader);

int glob(char *cmdValue);

int read_in_history(FILE *file);
