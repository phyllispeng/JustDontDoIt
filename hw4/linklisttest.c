#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define MAX_INPUT 1024
char *cmd = "ls idk if this works or not sad";
char *fake_cmd = "cd, cd..";
char *lol = "toilet sucker";

struct node_list{
  
  char *data;
  struct node_list *link;
};
typedef struct node_list node;
node *A;
node *current;

void free_list(node* ptr);

int main (){
  //Create an empty link list , A is the beginning of the list
  A = NULL;
  // pointer point at the current node
 // node *current;
  //make a space in the memory point the current pointer to the node
  current = (node*)malloc (sizeof(node));
  //put the cmd value into the node
  // current->data
  (*current).data = cmd;
  //set the node as the last node in the list which points to NULL
  (*current).link = NULL;
  //connect the beginning of the list to the node just made
  A = current;

  //make another node and connect it with the prev node
  //point the current pointer to the new node, make a new node
  current = (node*)malloc(sizeof(node));
  (*current).data = fake_cmd;
  (*current).link = NULL;
  //
  A->link = current;


  //make a new pointer to find the end of the list
  //let the new pointer point at the beginning of the list
  //print out the list
  node *current1 = A;

  while(current1 != NULL){
    //if the pointer is not point at the end of the list
    //then update the pointer position
    printf("%s\n", current1->data);

    current1 =  (*current1).link;
  }
  //print ends here


  current = (node*)malloc(sizeof(node));
  (*current).data = lol;
  (*current).link = NULL;

  current1 = A;
  // Insert new node
  while(current1->link != NULL){
    current1 = current1->link;
  }
  // Got to the tail
  current1->link = current;

  current1 = A;

  while(current1 != NULL){
    //if the pointer is not point at the end of the list
    //then update the pointer position
    printf("%s\n", current1->data);

    current1 =  (*current1).link;
  }
  free_list(A);

return 0;


}

void free_list(node* ptr) {
  if(ptr != NULL) {
    free_list(ptr->link);
    free(ptr);
  }
}