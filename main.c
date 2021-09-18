#define _GNU_SOURCE


#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h> 


// is foreground only mode 
int ground = 0;


// struct to store command 
struct inCommand {
     char* name;
     char** argv;
     int numArgs;
     int id;
     int status; 
     bool ground;
     char* input;
     char* out;
     struct inCommand* next; 
};



// Reference to Canvas modules exploration: Process API 
// returns status text based on cildstatus

void getStatus(int statusd)
{
     if (WIFEXITED(statusd))
     {
          printf("exit val:  %d\n", WEXITSTATUS(statusd));
          fflush(stdout);
     }
     else
     {
          printf("terminated because of signal:  %d\n", WTERMSIG(statusd));
          fflush(stdout);
     }
}


// prints one command to the console. Mostly used for testing.
void printCommand(struct inCommand* current) {
     printf("name: %s\n", current->name);
     fflush(stdout);
     int i = 0;
     for (i = 0; i < current->numArgs; i++) {                        // print each argument individualy 
          printf("argument: %s\n", current->argv[i]);
          fflush(stdout);
     }
     if (current->input != NULL) {
          printf("input: %s\n", current->input);                    // chech if files are not null -> print them 
          fflush(stdout);
     }
     if (current->out != NULL) {
          printf("Output: %s\n", current->out);
          fflush(stdout);
     }
     printf("ground: %d\n", current->ground);
     fflush(stdout);
}


// Prints column to the console. That is it. Nothing else except printung Column. 
void printCol() {
     printf(": ");                                               
     fflush(stdout);
}


// adds one Command to the linked list if the process is started on background 

struct inCommand* addNewBackground(struct inCommand *head, struct inCommand *current) {
     struct inCommand* loop; 
     loop = head; 
     struct inCommand*  ex = malloc(sizeof(struct inCommand));                       // alocate new mem 
     ex->name = current->name;
     ex->id = current->id;                                                           // probably, I should have created a function for this part 
     ex->next = NULL;
     ex->status = current->status; 
     ex->ground = current->ground;
     if (loop->next == NULL) {                                                       // loop through the list 
          head->next = ex;
          printf("started process: %d\n", loop->next->id);
          return head; 
     }
     else {
          while (loop->next->next != NULL) {
               loop = loop->next;                                                       // add element 
          }
      
          loop->next->next = ex;
          printf("started process: %d\n", loop->next->next->id);
     }
     return head; 
}



// prints all processes in the linked list 
// was used mostly for testing and debuging 
void printBackground(struct inCommand* head) {
     struct inCommand* loop;
     loop = head->next;
   
     if (head->next != NULL) {
         
          while (loop->next != NULL) {                                                // iter through the list 

            
               printf("%d\n", loop->id);                                              // print every element 
               fflush(stdout);
               loop = loop->next;
          }
          printf("%d\n", loop->id);
          fflush(stdout);
          
     }
}


// delete all processes which are no longer running on the background 
struct inCommand* deleteFinished(struct inCommand* head) {

     struct inCommand* loop;
     if (head->next == NULL) {                                                      //check for empty list 
          return head; 
     }
     
     if (head->next->next == NULL) {
          if (waitpid(head->next->id, &(head->next->status), WNOHANG) != 0) {          // check first element 
               printf("finished: %d\n", head->next->id);
               getStatus(head->next->status);                                       // print status of the deleted process 
               head->next = NULL; 
               return head;
          }
     }
     loop = head;
     while (loop->next->next != NULL) {
          if (waitpid(loop->next->id, &(loop->next->status), WNOHANG) != 0) {           // check the whole list 
               printf("finished: %d\n", loop->next->id);
               getStatus(head->next->status);
               loop->next = loop->next->next;
               break;
          } 
          loop = loop->next; 
     }
     if (waitpid(loop->next->id, &(loop->next->status), WNOHANG) != 0) {                 // check the last element 
          printf("finished: %d\n", head->next->id);
          getStatus(head->next->status);
          loop->next = NULL;                                                            
     }
     return head;
}


// replace $$ with -> id (inid) 
char* replace(char* input, int inid) {

     int l = snprintf(NULL, 0, "%d", inid);
     char* sPid = malloc((l + 1) * sizeof(char));
     snprintf(sPid, l + 1, "%d", inid);                                                      // alocate memory for id 

     char* ret = malloc((l + 1 + strlen(input)) * sizeof(char));
     int i;
     int k = 0;
     for (i = 0; i < l + strlen(input); i++) {
          if (input[i] == '$' && input[i + 1] == '$') {                                      // concat into string 
               int j = 0;
               for (j; j < l; j++) {
                    ret[i + j] = sPid[j];
               }
               k = 2;
               i = i + 2;
          }
          else {
               ret[i+k] = input[i];                                                           // copy the rest of the string
          }
          
     }

    // printf("ret: %s\n", ret);
 
          return ret;                                                                          // return the result 
    
}


// parse input into command struct. 
void parseCommand(struct inCommand* current, char* in, ssize_t s) {
     s--;
     char* save ;                                                                              // pointers for cut 
     char* tok ;
  
     
     int numArgs = 0;
     int i = 0;
     bool noArgs = 0; 
     if (false) {

     }
     else {
          for (i = 0; i < strlen(in); i++) {                                                                // check number of arguments based on >< and &
               if (in[i] == '<' || in[i] == '>' || (in[i] == '&' && i == (strlen(in) -1))) {
                    numArgs--;
                    break;
               }
               if (in[i] == ' ') {                                                                          // if space ++ 
                   
                    numArgs++;
               }
               if (in[i] == '\n') {                                                                         // if end of the file -> ++ 
                    numArgs++;
                    break;
               }
          }
     }
     int p;    
  
     tok = strtok_r(in, " ", &save);                                                                          // get ( 0 -> first spacer) 
     char* store = calloc(strlen(save), sizeof(char));
     strcpy(store, save);
     current->name = calloc(strlen(tok), sizeof(char));                                                      // copy into name
   
     strcpy(current->name, tok);
     current->numArgs = numArgs;
    
     fflush(stdout);
     
     current->argv = calloc(numArgs, sizeof(char*));
     
     for (i = 0; i < numArgs; i++) {
          
          tok = strtok_r(NULL, " ", &save);                                                                   // copy all arguments into the array 
        
          current->argv[i] = calloc(strlen(tok) + 1, sizeof(char));
          strcpy(current->argv[i], tok);
     }

     while (1) {
          tok = strtok_r(NULL, " ", &save);                                                                  // check for input, output and other files 
          if (tok == NULL) {
               break;
          }
          if (tok[0] == '&') {                                                                                    // background 
               if (ground == 1) {
                    current->ground = 0;
               }
               else {
                    current->ground = 1;
               }
               
               break;
          }
          if (tok[0] == '<') {                                                                                    // input files 
               tok = strtok_r(NULL, " ", &save);
               current->input = calloc(strlen(tok) + 1, sizeof(char));
               strcpy(current->input, tok);
          }
          if (tok[0] == '>') {                                                                                     // output files 
               tok = strtok_r(NULL, " ", &save);
               current->out = calloc(strlen(tok) + 1, sizeof(char));
               strcpy(current->out, tok);
          }
          
     }

     fflush(stdout);
}


// execute NONDEFAULT comands (cd, status, exit) 
bool executeComands(char* in, int status, struct inCommand* head) {        
     if (strcmp(in, "") == 0 || strcmp(in, "\n") == 0) {                                              // check for enter 
          return true; 
     }
     if (in[0] == '#') {                                                                               // check for comment 
          return true; 
     }
     if (strncmp(in, "exit", 4) == 0 && strlen(in) == 4) {                                              // check for exit 
          // kill each process here
          if (head->next != NULL) {
               struct inCommand* loop = head->next;                                                      // kill all processes 
               while (loop->next != NULL) {
                    struct inCommand* c = loop;
                    loop = loop->next;
                    kill(c->id, 1);
               }
               kill(loop->id, 1);
          }
          printf("exit\n");                                                                            // exit file 
          fflush(stdout);
          exit(0);
          return true;
     }
     if (strncmp(in, "cd", 2) == 0) {                                                                     // cd command 
        
          if (strlen(in) == 3 || strlen(in) == 2) {
               chdir(getenv("HOME"));                                                                  // just cd command (change to HOME) 
               return true; 
          }
          else {
               char* ptr;                                                                              // cd + path 
               char* tok = strtok_r(in, " ", &ptr);
               tok = strtok_r(NULL, "\n", &ptr);
               if (strncmp(tok, "./", 2) == 0) {                                                       // if it is relative 
                    chdir(tok);
                    return true;
               }
               else {
                    char* ext = "./";                                                                  // add ./ if it is not 
                    char* result = malloc(3 + strlen(tok));
                    strcpy(result, ext);
                    strcat(result, tok);
                    chdir(result);
                    return true; 
               }
          
          }
     }
     if (strncmp(in, "status", 6) == 0 && strlen(in) == 6) {                                                 // if status -> print status 
          getStatus(status);
          fflush(stdout);
          return true; 
     }
     return false; 
}


// command to change output path 
int changePathInput(struct inCommand* current) {
     if (current->input == NULL) {                                                // no file to read from 
         // printf("No file to open\n");
          fflush(stdout);
          return 0; 
     }
     else {
          int callBack = open(current->input, O_RDONLY);                         // open the file 
          if (callBack == -1) {                                                  // if failed 
               perror("Error, no such file\n");
               return 1;
          }
          else {
               int r = dup2(callBack, 0);                                        // dup into new location 
               if (r == -1) {
                    return 1;
               }
               fcntl(callBack, F_SETFD, FD_CLOEXEC);
               return 0;
          }
     }
}


// similar function but for output file 
int changePathOut(struct inCommand* current) {
     if (current->out == NULL) {                                                 // if no file, skip 
          //printf("no file to open\n");
          fflush(stdout);
          return 0;
     }
     else {
          int callBack = open(current->out, O_WRONLY | O_CREAT | O_TRUNC, 0644);               // open the file, if there is no file -> create it 
          if (callBack == -1) {
               perror("Error, no such file\n");                                                // error handling 
               fflush(stdout);
               return 1; 
          }
          else {
               int r = dup2(callBack, 1);                                                        // dup into new location 
               if (r == -1) {
                    return 1; 
               }
               fcntl(callBack, F_SETFD, FD_CLOEXEC);
               return 0; 
          }
     }
}


// create command in format acceptable to execute function 
char** createCommand(struct inCommand* ex, int numArgs) {
     char** arguments = calloc(numArgs, sizeof(char*));
     int j;
     arguments[0] = calloc(strlen(ex->name), sizeof(char));                                        // add the name 
     strcpy(arguments[0], ex->name);                                                                  
     if (ex->numArgs == 0) {                                                                
          arguments[1] = calloc(1, sizeof(char));                                                 // if there are no arguments -> add 0 
          arguments[1] = NULL;
     }
     else {
          for (j = 0; j < ex->numArgs; j++) {                                                     // otherwise, feel up the arguments from the struct 
               arguments[j + 1] = calloc(strlen(ex->argv[j]), sizeof(char));
               strcpy(arguments[j + 1], ex->argv[j]);
          }
     }
     arguments[numArgs - 1] = calloc(1, sizeof(char));
     arguments[numArgs - 1] = NULL; 
     int i;
     for (i = 0; i < numArgs; i++) {                                                                 // was used for debugging 
          if (arguments[i] == NULL) {
             //  printf("NULL");
          }
          else {
               //printf("%s", arguments[i]);
          }
     }
     fflush(stdout);
     return arguments;
}

// reference: build based on Exploration: Signal Handling API 
    // function to handle ctrl + Z
void handle_SIGINT(int signo) {
     
     if (ground == 1) {
          char* message = "Exiting foreground only mode\n";                                               // switch to only foreground 
          write(STDOUT_FILENO, message, 30);
          ground = 0; 
     }
     else {
          
          char* message = "Entering foreground-only mode (& is now ignored)\n";                           // back to normal 
          write(STDOUT_FILENO, message, 50);
          ground = 1;
     }
}

int main() {
     // reference: build based on Exploration: Signal Handling API 
     struct sigaction ignoreINT = { 0 };                                                      // define handlers 
     struct sigaction actionSigTSTP = { 0 };
     struct sigaction ignoreSIG = { 0 };
     
     
     ignoreINT.sa_handler = SIG_IGN;
     ignoreSIG.sa_handler = SIG_DFL;                                                         // setup handlers 
     actionSigTSTP.sa_handler = handle_SIGINT;
     actionSigTSTP.sa_flags = SA_RESTART;
     sigaction(SIGTSTP, &actionSigTSTP, NULL);
     sigaction(SIGINT, &ignoreINT, NULL);



     char* input;
     char* in = malloc(sizeof(char) * 2048);                                                               // alocate memory for input 
     int savedOut = dup(1);                                                                                // save dups 
     int savedIn = dup(0);
     struct inCommand* head = malloc(sizeof(struct inCommand));                                            // allocate linked list 
     head->next = NULL;
     size_t length = 0; 
     ssize_t lineSize =0;
     int status = 0; 
     bool check;
     
     while (1) {
          struct inCommand* ex = NULL;
          ex = malloc(sizeof(struct inCommand));                                                          // allocate mem for current command 
          ex->name = NULL;
          ex->argv = NULL;
          ex->id = 0;
          ex->ground = 0;
          ex->input = NULL;
          ex->out = NULL;
          ex->numArgs = 0;
          
          //printBackground(head);

          head = deleteFinished(head);                                                                      // check for already finished processes 
          //printBackground(head);
          printCol();                                                                                       // : 
          lineSize = getline(&in, &length, stdin);                                                          // get input 
          int pid = getpid();
          
        
          if (in[strlen(in)-1] == '\n') {                                                                  // remove garbage (almost garbage collector, but in C) 
               input = malloc((strlen(in) - 1) * sizeof(char));
               int v;
               for (v = 0; v < strlen(in) - 1; v++) {                                                         
                    input[v] = in[v];
               }
          }
          else {
               input = malloc((strlen(in)) * sizeof(char));
               int v;
               for (v = 0; v < strlen(in); v++) {
                    input[v] = in[v];
               }
          }
          int p;

          input = replace(input, pid);                                                                // replace $$ with pid 
         
          check = executeComands(input, status, head);                                                  // execute status, cd exit
          if (check == true) {
               continue;                                                                             // back to the beginning if succesful 
          }
          else {
               parseCommand(ex, input, lineSize);                                                     // prase input in command 
               //printCommand(ex);
               status = changePathInput(ex);                                                        // adjust input\output 
               if (status == 1) {
                    continue;
               }
               status = changePathOut(ex);
               if (status == 1) {
                    continue;
               }
               int numArgs;
               if (ex->numArgs == 0) {
                    numArgs = 3;
               }
               else {
                    numArgs = ex->numArgs + 2;
               }

               char** arguments = createCommand(ex, numArgs);                                   // create command aruments           

               if (ex->ground == 0) {
                    ignoreINT.sa_handler = SIG_DFL;
                    sigaction(SIGINT, &ignoreINT, NULL);                                      // change hanlders for foreground 
               }
               else {

               }
               sigaction(SIGTSTP, &ignoreSIG, NULL);
               pid_t forkId = fork();
               ex->id = forkId;                                                                   // for a process 
              
               if (ex->id == -1) {
                    printf("failed\n");                                           
                    fflush(stdout);
               }
               if (ex->id == 0) {                                                                  // execute on parallel process 
                   status = execvp(arguments[0], arguments);
                   
                   exit(1);
               }
               else {
                    sigaction(SIGTSTP, &actionSigTSTP, NULL);                                    // main processs 
                    if (ex->ground == 0) {
                         ignoreINT.sa_handler = SIG_IGN;
                         sigaction(SIGINT, &ignoreINT, NULL);                              // if on foreground wait for process 
                         ex->id = waitpid(ex->id, &status, 0);
                         if (status != 0) {
                              printf("error while executing the command\n");
                              fflush(stdout);
                         }
                    }
                    else {

                         // do not wait for process.
                         //printBackground(head);
                         head = addNewBackground(head, ex);
                         ex->id = waitpid(ex->id, &status, WNOHANG);
                        
                         ex->status = status; 

                    }
               }
               dup2(savedIn, 0);
               dup2(savedOut, 1);
          }
          //printCommand(ex);
          // printf("%s\n", input);
          // fflush(stdout);
     }
     return 0; 
}