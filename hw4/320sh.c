#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "320sh.h"
#include <dirent.h>

// Assume no input line will be longer than 1024 bytes
#define MAX_INPUT_2 1024

char *newLineChar = "\n";
char *exitCommand = "exit\n";
char *pwdCommand = "pwd\n";
char *cdOnly = "cd ";
char *cdHome = "cd\n";
char *cdDash = "cd -\n";
char *cdDot = "cd .\n";
char *cdDotDot = "cd ..\n";

char *cmd_history = "history\n";
char *clear_history = "clear-history\n";
//char  current_dir[MAX_INPUT];
char  current_dir[MAX_INPUT_2];

char *left_b = "[";
char *right_b = "] ";
char *prompt = "320sh> ";
char  totalHeader[MAX_INPUT_2 + 13];

FILE *HistoryFile ;

//int HistoryFileDescriptor;

char *set = "set ";
char *echo = "echo ";
char *dollarSign = "$";
char *questionSign = "?";

char space[2] = " ";

char *debugFlag = "-d";
int debugLocation;
int debugStat;
int exitStat;

int fileLocation;

char *leftArrow = "<";
char *rightArrow = ">";
char *pipeSymbol = "|";



int charCounter = 1;

node *A = NULL;
node *current;
node *current_his;
int finished = 0;

char cmdWithoutFrontSpace[MAX_INPUT_2];

char *ls = "ls";
char *star = "*";

char dot[2] = ".";

// 0 means down, 1 means up, -1 not pressed
int prev_dir = DIR_UNINIT;


int main (int argc, char ** argv, char **envp) {

  finished = 0;

  char cmd[MAX_INPUT_2];

  signal (SIGTTIN, SIG_IGN);
  
  //make a new list

  HistoryFile = fopen("History.txt","r");
  if(HistoryFile != NULL) {
    read_in_history(HistoryFile);
  }

     /* check for debug flag is there first */
  int debugFlagCounter = 0;
  while(debugFlagCounter < argc){
      /* check to see if the argument is the flag */
    if(strcmp(debugFlag, argv[debugFlagCounter]) == 0){
        /* set the debugstat to 1 so we know we are in debug */
      debugStat = 1;
      debugLocation = debugFlagCounter;
    }
    debugFlagCounter++;
  }

    /* location of file */
  if(debugLocation == 1){
    fileLocation = 2;
  }
  else
  {
      /* debug location is 2 */
    fileLocation = 1;
  }


  if(argc > 1){
      /* this is prob a file, so you want to try fopen */

    FILE *file;

    // printf("fileLocation: %d\n", fileLocation);

      /* open file to read */
    file = fopen(argv[fileLocation], "r");

    if(file != NULL){
      // printf("FILE OPENED \n");

      char cmdLine[MAX_INPUT_2];

      while(fgets(cmdLine, MAX_INPUT_2, file) != NULL)
      {
          /* check if first char is # */
        if(cmdLine[0] == '#'){
            /* comment line, ignore. Go to next line */
        }
        else{
            /* not a comment, go do the function */
          cmdCheck(cmdLine, envp);

            /* clear the buffer space with the line */
          int counterClear = 0;
          while(counterClear != MAX_INPUT_2){
              /* clear up the buffer */
            cmdLine[counterClear] = '\0';
            counterClear++;
          }
        }
      }

        /* close the file */
      fclose(file);
      exitStat = 0;
    }
    else
    {
        /* set exitstat error */
      exitStat = -1;
    }

  }
  else
  {
      /* do nothing */
  }

    /* put the current working directory into the path */
  char* path = getenv("PATH");
  char * inBetween = ":";
  getcwd(current_dir, MAX_INPUT_2);
    /* concatenate and then set it */
  int pathLength = strlen(path);
  int cwdLength = strlen(current_dir);
  int inBetweenLength = strlen(inBetween);

  char pathDirect[pathLength+cwdLength+inBetweenLength];
  strcpy(pathDirect, path);
  strcat(pathDirect, inBetween);
  strcat(pathDirect, current_dir);

  setenv("PATH", pathDirect, 1);

    /* NOW THIS IS THE SHELL RUNNING. YOU WANT TO PRESS ENTER FOR ANYTHING */
  
  while (!finished) {

    char *cursor;
    char last_char;
    int rv;
    int count;
    
    getcwd(current_dir, MAX_INPUT_2);
    
    strcpy(totalHeader, left_b);
    strcat(totalHeader, current_dir);
    strcat(totalHeader, right_b);
    strcat(totalHeader, prompt);

    rv = write(1, totalHeader, strlen(totalHeader));
    if (!rv) { 
      finished = 1;
      break;
    }

    // read and parse the input
    for(rv = 1, count = 0, 
      cursor = cmd, last_char = 1;
      rv 
      && (++count < (MAX_INPUT_2-1))
      && (last_char != '\n');
      cursor++) { 

      rv = read(0, cursor, 1);

    if(*cursor == '\033'){
      //write(1, "hello", 1);

      read(0, cursor, 1); // [

        read(0, cursor, 1);

        if(*cursor == 'A'){
          // up
          if(current_his != NULL){
            if(prev_dir == DIR_DOWN) {
              // See if we can move it back
              if(current_his->link != NULL) {
                current_his = current_his->link;
              }
            }
            char *reset = "\r\033[K";
            // erase line
            write(1, reset, strlen(reset));
            // rewrite prompt
            write(1, totalHeader, strlen(totalHeader));
            // Redraw line with new command
            write(1, current_his->data, strlen(current_his->data));
            // Copy history command into command buffer
            strcpy(cmd, current_his->data);
            cursor = cmd + strlen(cmd) - 1;
            // Move current history backwards by one
            if(current_his->prev != NULL) {
              current_his = current_his->prev;
            }
          } else {
            *cursor = '\0';
            cursor--;
          }
          prev_dir = DIR_UP;
          // Skip the rest of the loop
          continue;
        }
        else if(*cursor == 'B'){
          // down
          if(current_his != NULL){
             if(prev_dir == DIR_UP) {
              // See if we can move it back
              if(current_his->link != NULL) {
                current_his = current_his->link;
              }
            }
            // Move current history forward by one
            if(current_his->link != NULL) {
              current_his = current_his->link;
            }
            char *reset = "\r\033[K";
            // erase line
            write(1, reset, strlen(reset));
            // rewrite prompt
            write(1, totalHeader, strlen(totalHeader));
            // Redraw line with new command
            write(1, current_his->data, strlen(current_his->data));
            // Copy history command into command buffer
            strcpy(cmd, current_his->data);
            cursor = cmd + strlen(cmd) - 1;
          } else {
            *cursor = '\0';
            cursor--;
          }
          prev_dir = DIR_DOWN;
          // Skip the rest of the loop
          continue;

        }
        else if(*cursor == 'C'){
      // right
          /* move cursor forward one */
          rv++;

        }
        else if(*cursor == 'D'){
      //left

          if(charCounter == 1){
            //dont move
          }
          else{
            /* decrease counter */
            charCounter--;
            /* move cursor back one */
            write(1, "\b", 1);

          }

        }
        else{

    //do nothing

        }

      }
      else
      {

        /* check if its a backspace character */
        if(*cursor == 127 || *cursor == 8){ 

          if(charCounter == 1){
          //dont move
          }
          else{
          /* call delete */
            printf("\b\033[K");
          }

        }
        else{
          /* writing into the launcher */
          write(1, cursor, 1);
          charCounter++;
        }

      }

      last_char = *cursor;
    } 
    *cursor = '\0';

    if (!rv) { 
      finished = 1;
      break;
    }

    /* REMEMBER!:  get rid of the spaces up front! */

    cmdCheck(cmd, envp);
    charCounter = 1;
}

if(HistoryFile != NULL) {
  fclose(HistoryFile);  
}

HistoryFile = fopen("History.txt","w+");

print_list_to_file(A, HistoryFile);
free_list(A);
fclose(HistoryFile);
return 0;

}





void escape(){
  /* way to escape with exit */
  printf("Exiting. \n");
  // exit(3);
  finished = 1;
}

int pwd(){
  /* where am i? */
  char tempTransferArray[MAX_INPUT_2];

  getcwd(tempTransferArray, MAX_INPUT_2);

  write(1, tempTransferArray, strnlen(tempTransferArray, MAX_INPUT_2));
  
  write(1, newLineChar, strnlen(newLineChar, sizeof(newLineChar)));

  exitStat = 0;

  return exitStat;
}




int cd(){
  getcwd(current_dir, MAX_INPUT_2);
  char* home = getenv("HOME");
  cdAddress(home);

  exitStat = 0;
  return exitStat;

}



int cdLast(){
  getcwd(current_dir, MAX_INPUT_2);
  char *prev = getenv("OLDPWD");
  cdAddress(prev);

  exitStat = 0;
  return exitStat;
}




int cdAddress(char *cmdValue){
  getcwd(current_dir, MAX_INPUT_2);

  char tempTransferArray[MAX_INPUT_2];
  getcwd(tempTransferArray, MAX_INPUT_2);

  remove_newline(cmdValue);
  setenv("OLDPWD", tempTransferArray, 1);
  int result = chdir(cmdValue);
  getcwd(tempTransferArray, MAX_INPUT_2);
  setenv("PWD", tempTransferArray, 1);
  
  if(result == -1){

    printf("%s - '%s'\n", "invalid path",cmdValue);
  }

  exitStat = result;
  return exitStat;

}

void remove_newline(char *str_path){
  if(str_path != NULL){
    int last = strlen(str_path)-1;
    if (str_path[last] == '\n'){
      str_path[last] = '\0';
    } 
  }
}

pid_t Fork(void){
  pid_t pid =0;

  /*
   * Check to see if there was an error spawning
   */
   if((pid = fork()) < 0){
    printf("Fork Error. \n");
    exitStat = 1;
  }  

  return pid;
}


int run_program(char *cmdValue, char **environ){

  if(cmdValue !=NULL){
    remove_newline(cmdValue);
    // write(1, cmdValue, strlen(cmdValue));
    // write(1, "\n", 1);
    char *addr_tok[512];
    char *tokenized;

    int counter=0;
    tokenized = strtok (cmdValue, space);
    while (tokenized !=NULL){
      addr_tok[counter] = tokenized;
      counter++;
      tokenized = strtok(NULL,space);
    }
    addr_tok[counter] = NULL;
    pid_t pid = Fork();

    if(pid == 0){
      /* child process */
      execvpe(addr_tok[0], addr_tok, environ);
      // should never get here...
      printf("Exec failed\n");
      exitStat = 1;
      exit(EXIT_FAILURE);
    }
    else
    {
      /* parent process */
      int status;
      waitpid(pid, &status, 0);
      exitStat = WEXITSTATUS(status);
    }
  }

  //exitStat = 0;
  return exitStat;

}

int setVar(char * cmdValue){

  char *token;
  char *tokenArray[MAX_INPUT_2/2];

  /* remove newline */
  remove_newline(cmdValue);

  /* tokenize */
  token = strtok(cmdValue, space);

  int tokenCounter = 0;
  while(token != NULL){
    /* while there are tokens, put in array */
    tokenArray[tokenCounter] = token;
    tokenCounter++;
    token = strtok(NULL, space);
  }
  tokenArray[tokenCounter] = NULL;

  /* setenv(location, setItem, 1) */
  exitStat = setenv(tokenArray[1], tokenArray[3], 1);

  return exitStat;
}

int echoFunction(char *cmdValue){

  char *token;
  char *tokenArray[MAX_INPUT_2/2];
  char *tokenWithOutDollarSign;

  /* remove new line */
  remove_newline(cmdValue);

  /* tokenize */
  token = strtok(cmdValue, space);

  int tokenCounter = 0;
  while(token != NULL){
    /* while there are tokens, put in array */
    tokenArray[tokenCounter] = token;
    tokenCounter++;
    token = strtok(NULL, space);
  }
  tokenArray[tokenCounter] = NULL;

  if(strncmp(tokenArray[1], dollarSign, strlen(dollarSign)) == 0){
    /* if there is a $, then you want to check the environment to see if there is this var and print out the stuff */

    /* get rid of the dollar sign with tokenizer*/
    tokenWithOutDollarSign = strtok(tokenArray[1], dollarSign);

    if(strncmp(tokenWithOutDollarSign, questionSign, strlen(questionSign)) == 0){
       /* if it's a ? then you want to print out the last exit stat */
      printf("%d\n", exitStat);
      exitStat = 0;
    }
    else
    {
      /* check environ for var*/
      char* varToGet = getenv(tokenWithOutDollarSign);
      if(varToGet == NULL){
        /* error */
        printf("The environment variable after the $ sign is not valid.\n");
        exitStat = 1;
      }
      else
      {
        /* print out what is in the environment variable */
        printf("%s\n", varToGet);
        exitStat = 0;
      }
    }
  }
  else
  { 
      /* echo does two things, if there is no $, just print out the command again */
    printf("%s\n", tokenArray[1] );
    exitStat = 0;
  }
  return exitStat;
}

int cmdCheck(char *cmd, char **envp){

  /* check for redirection support */
  char *leftArr;
  char *rightArr;
  char *pipeSym;
  creat_history(&A, cmd);
  /* check for left arrow */
  leftArr = strchr(cmd, '<');
  /* check for right arrow */
  rightArr = strchr(cmd, '>');
  /* check for pipe */
  pipeSym = strchr(cmd, '|');
  
  if((leftArr != NULL) || (rightArr != NULL) || (pipeSym != NULL)){
    /* you know you need to do redirection */
    /* tokenize the elements */
    char *token;
    char *tokenArray[MAX_INPUT_2/2];
    //char *token_space;

  /* remove newline */
    remove_newline(cmd);

  /* tokenize */
    token = strtok(cmd, "<|>");

    int tokenCounter = 0;
    while(token != NULL){
    /* while there are tokens, put in array */
      tokenArray[tokenCounter] = token;
      tokenCounter++;
      token = strtok(NULL, "<|>");
    }
    tokenArray[tokenCounter] = NULL;

    // Remove space on tokens
    tokenCounter = 0;
    while(tokenArray[tokenCounter] != NULL) {
      // token_space = strtok(tokenArray[tokenCounter], " ");
      trimString(&tokenArray[tokenCounter]);
      tokenCounter++;
    }

  /* after tokenize, you want to do the redirection */
  /* put this in a loop to loop through the array? */
    int pid = Fork();
    int fileDescriptor;
    int fileDescriptor1;
    if(pid == 0){
    /* child process */

      if(rightArr != NULL){
        char *file = tokenArray[1];
        if(leftArr != NULL) {
          // both left and right redirects are set
          file = tokenArray[2];
        } 

        // delete the file if it exists
        unlink(file); 
        // create the output file
        fileDescriptor = creat(file, 0644);
        dup2(fileDescriptor, STDOUT_FILENO);
        close(fileDescriptor);
        rightArr = NULL;
      }
      if(leftArr != NULL){
        fileDescriptor1 = open(tokenArray[1], O_RDONLY);
        dup2(fileDescriptor1, STDIN_FILENO);
        close(fileDescriptor1);
        leftArr = NULL;
      }
      // Build a new command array
      char *addr_tok[512];
      char *tokenized;
      int counter=0;
      tokenized = strtok (tokenArray[0], space);
      while (tokenized !=NULL){
        addr_tok[counter] = tokenized;
        counter++;
        tokenized = strtok(NULL,space);
      }
      addr_tok[counter] = NULL;
      // Now execute the command
      if(execvpe(addr_tok[0], addr_tok, envp) == -1) {
        // should never get here.....
        printf("Failed to execute %s\n", addr_tok[0]);
        exit(EXIT_FAILURE);
      }
    }
    else{
    /* parent process */
      int childStat;
      waitpid(pid, &childStat, 0);
      // printf("exit: %d\n", childStat);
      exitStat = WEXITSTATUS(childStat);
      // if(childStat != 0){
      //   exitStat = 1;
      // }
      // else{
      //   exitStat = 0;
      // }
    }
  }

  else if(strcmp(cmd, exitCommand) == 0){

      /* debugStat is on */
    if(debugStat == 1){
      fprintf(stderr,"Running %s\n", cmd);
    }

        /* print out the result of the action if in debugStat */
    if(debugStat == 1){
      fprintf(stderr,"Ended exit and returned %d\n", 0);
    }
    

      /* escape */
      escape();
  }

  else if(strcmp(cmd, pwdCommand) == 0){

      /* debugStat is on */
    if(debugStat == 1){
      fprintf(stderr,"Running %s\n", cmd);
    }

      /* pwd command */
    exitStat = pwd();
  }

  else if(strcmp(cmd, cdHome) == 0){

      /* debugStat is on */
    if(debugStat == 1){
      fprintf(stderr,"Running %s\n", cmd);
    }

      /*cd command, but which one?*/
    // printf("Comparison for the home is reached \n");
    exitStat = cd();
  }

  else if(strcmp(cmd, cdDash) == 0){

      /* debugStat is on */
    if(debugStat == 1){
      fprintf(stderr,"Running %s\n", cmd);
    }

    // printf("Comparison for cd last is reached\n");
    exitStat = cdLast();
  }
  else if(strcmp(cmd, cmd_history) == 0){
    if(debugStat == 1){
      fprintf(stderr,"Running %s\n", cmd);
    }

    // printf("Comparison for history is reached\n");
    exitStat = write_on_screen(A);
  }
  else if(strcmp(cmd, clear_history) == 0){
    if(debugStat == 1){
      fprintf(stderr,"Running %s\n", cmd);
    }

    // printf("Comparison for clear history is reached\n");

    exitStat = clean_history(A);
  }

  else if(strncmp(cmd, cdOnly, strlen(cdOnly)) == 0){

        /* debugStat is on */
    if(debugStat == 1){
      fprintf(stderr,"Running %s\n", cmd);
    }

    char input_path[MAX_INPUT_2];
    strcpy(input_path, cmd+3);
    printf("%s\n",input_path);
    // printf("CD ONLY REACHED\n");
    exitStat = cdAddress(input_path);

  }
  else if(strncmp(cmd, set, strlen(set)) == 0){

    // printf("Set reached\n");
      /* debugStat is on */
    if(debugStat == 1){
      fprintf(stderr,"Running %s\n", cmd);
    }

    exitStat = setVar(cmd);

  }
  else if(strncmp(cmd, echo, strlen(echo)) == 0){

    // printf("Echo reached!\n");
      /* debugStat is on */
    if(debugStat == 1){
      fprintf(stderr, "Running %s\n", cmd);
    }

    exitStat = echoFunction(cmd);

  }
  else if (strncmp(cmd, ls, strlen(ls)) == 0){

    /* if there is an ls, you want to check the cmd to see if there is a * */
    if(strstr(cmd, star) != NULL){
      /* this is where you call the globbing function then */
      exitStat = glob(cmd);

    }
    else{
      /* do nothing */
      exitStat = run_program(cmd, envp);
    }

  }
  else{
      /* executable programs */

      /* file? */
      /* debugStat is on */
    if(debugStat == 1){

      fprintf(stderr,"Running %s\n", cmd);

    }
    
    exitStat = run_program(cmd, envp);
  }

  /* print out the result of the action if in debugStat */
  if(debugStat == 1){
    fprintf(stderr,"Ended %s and returned %d\n", cmd, exitStat);
  }

  return exitStat;
}



void trimString(char **string) {
  if(string != NULL && *string != NULL) {
    // Check the front of the string for space characters
    while(**string == ' ') {
      *string = *string + 1;
    }
    // Now check the end of the string for spaces
    size_t length = strlen(*string) - 1;
    char *end_str = *string + length;
    while(*end_str == ' ') {
      *end_str = '\0';
      end_str--;
    }
  }
}

int creat_history(node **A_link, char *cmd){
  int success = 0;
  if(A_link != NULL) {
    node *n = *A_link;
    char *cmd_cpy = malloc(strlen(cmd) + 1);
    strcpy(cmd_cpy, cmd);
    remove_newline(cmd_cpy);
    if(strlen(cmd_cpy) > 0) {
      if(n == NULL){
        //new started list
        current = (node *)malloc(sizeof(node));
        current -> data = cmd_cpy;
        current -> link = NULL;
        current -> prev = NULL;
        // Assign the new head pointer
        *A_link = current;
      } else {
        //the list is created 
        current = (node *)malloc(sizeof(node));
        current -> data = cmd_cpy;
        current -> link = NULL;
        // insert at the end of the list
        node *ptr = n;
        while(ptr->link != NULL) {
          ptr = ptr->link;
        }
        ptr->link = current;
        current->prev = ptr; 
      }
      current_his = current;
    }
    prev_dir = DIR_UNINIT;
    success = 1;
  }

  return success;
}

int write_on_screen(node *A_link){
   node *current1 = A_link;
   
    while (current1 != NULL){
      
      printf("%s\n",current1->data);
      current1 = (*current1).link;
    }
   
   return 0;
}


int clean_history(node *A_link){
   free_list(A_link);
   A=NULL;
   return 0;
}

void free_list(node *ptr){
  if(ptr != NULL){
    free_list(ptr->link);
    free(ptr);
  }
}

int print_list_to_file(node *A_link, FILE *file){

   node *current1 = A_link;
   
    while (current1 !=NULL ){
      
      //printf("%s\n",current1->data);
     fprintf(file, "%s\n",current1->data);
     current1 = (*current1).link;
    }
    return 0;
}

int read_in_history(FILE *file){
  char result[MAX_INPUT];
  while(fgets(result, MAX_INPUT, file) != NULL){
    creat_history(&A, result);
  }
  return 1;
}

int backspace(char cmdToBackspace[], char totalHeader[]){

  write(1, "\b", 1);
  write(1, " ", 1);

  write(1, "\b", 1);
  charCounter--;     

  return 0;
}

int glob(char *cmdValue){

  int found = 0;

    /* have the cmd value, want to tokenize to get the extension */
  remove_newline(cmdValue);
  char *tokenArray[512];
  char *tokenized;

  int counter=0;
  tokenized = strtok (cmdValue, star);
  while (tokenized !=NULL){
    tokenArray[counter] = tokenized;
    counter++;
    tokenized = strtok(NULL,star);
  }
  tokenArray[counter] = NULL;

    /* now element one of the array contains the extension that you wanted */

  DIR *fd;
  struct dirent *returnHere;

    //only searches in the current directory
  getcwd(current_dir, MAX_INPUT_2);

  fd = opendir(current_dir);
  if(fd == 0 || strstr(tokenArray[1], dot) == NULL || tokenArray[1] == NULL){
    printf("Bad file extension.\n");
  }
  else{
    while(((returnHere = readdir(fd))!= NULL)){
      if(strstr(returnHere -> d_name, tokenArray[1]) != NULL){
        printf("%s\n", returnHere ->d_name ); 
        found++;
      }

    }
  }

  if(found == 0){
    printf("No file was found with this extension.\n");
  }

  return 0;
}
