#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#define MAX 1024
#define MAX_ARGS 20

int main(int argc, char *argv[])
{
  int opt; // to select option 
  int usec = 10000000; // to stop the program for a while
  int limit = 0; // how many times to iterate the program  
  int rc = 0,lrc= -1; // return code and last return code 
  char *s = malloc(sizeof(char));
  char sTemp[200];
  int cflag = 0,tflag = 0;
// int optopt - When getopt encounters an unknown option character or an option with a missing required argument, 
//  it stores that option character in this variable. You can use this for providing your own diagnostic messages.

// const char* options -The options argument is a string that specifies the option characters that are valid for 
//  this program. An option character in this string can be followed by a colon (‘:’) to indicate that it takes a 
//  required argument. If an option character is followed by two colons (‘::’), its argument is optional; this is
// a GNU extension.

  // printf("argc:%d\n",argc );
  if(argc <= 1){
    // printf("There is neither option nor command.\n");
    exit(EXIT_FAILURE);
  }

//#############################################################################################
  while((opt = getopt(argc,argv,"+t:i:l:c")) != -1){
    switch(opt){
      // case for the 't'    
      case 't':
        tflag = 1;
        s = optarg;        
      break; 
      // case for the 'i'
      case 'i':
        usec = atoi(optarg)*1000;
        // printf("interval:%d\n",usec );
        if(usec < 0){
          // printf("interval negative.\n");
          exit(EXIT_FAILURE);
        }
        if(usec == 0){
          // printf("interval null.\n");
          exit(EXIT_FAILURE);
        }
        // printf("loading...\n");
        // usleep(usec*1000);
      break;
      // case for the 'l'      
      case 'l':
        limit = atoi(optarg);
        if(limit < 0){
          // printf("limit is negative.\n");
          exit(EXIT_FAILURE);
        }
        // printf("limit:%d\n",limit );
      break;
      // case for the 'c'
      case 'c':
        cflag = 1;
      break;
      // case for the error 
      case '?':   
/*        if(optopt == 't')
          printf("Please,give the correct form of time.\n");*/
      break; 
    }

  } // end of while

//#############################################################################################
//  if the command is not given...

  if(argv[optind] == NULL){
    // printf("command is not given.\n");
    exit(EXIT_FAILURE);
  }

  // the process of concatinating strings which are given after the options
  char* args[MAX_ARGS];
  // printf("argc:%d\noptind:%d\n",argc,optind );
  int j=0;
  for(int i=optind;i<argc;i++,j++){
    args[j] = argv[i];
    // printf("args[%d]:%s\n",j,args[j] );
  } 
  args[j] = NULL;

//#############################################################################################


  // the process of main work - executing command ...
  int status;
  int tube[2]; // tube[0] --> for using read end, tube[1] --> for using write end
  char* bufTemp = malloc(MAX*sizeof(char));

  for(int k=0; limit == 0 || k<limit;k++){
    pipe(tube);
    pid_t pid = fork();
    if(pid == -1)
      exit(EXIT_FAILURE);
    else if(pid == 0){ // child process...
      
      close(tube[0]); // we don't need to read anything
      dup2(tube[1],1); // duplicating tube[1] with stdout
      close(tube[1]);
        
      execvp(args[0],args);
      // exit(EXIT_SUCCESS);
    }
    else{ // parent process...
      close(tube[1]); // if we don't close tube[1], read(...) function will wait for the input data...
      int n=0,i=0;
      int count = 1;
      char* buf = malloc(MAX*sizeof(char)); 
      while((n=read(tube[0],&buf[i],sizeof(char))) > 0){ // to read data from tube[0] to buffer
        i+=n;
        if(i >= MAX*count){
          count+=1;
          buf = realloc(buf,MAX*count*sizeof(char));
        }
      }
      buf[i] = '\0';  // to print correctly
      close(tube[0]);
      
      if(tflag != 0){
        time_t t = time(NULL);
        strftime(sTemp,sizeof(sTemp),s,localtime(&t));
        fprintf(stdout, "%s\n",sTemp );
        // printf("%s\n",sTemp );
      }
      
      // in case of any modification 
      
      if(memcmp(buf,bufTemp,i)){ // if new and old case are not equal to each-other
        // printf("%s",buf );
        // write(1,buf,i);
        fwrite(buf,i,1,stdout);
/*        for(int j=0;j<MAX*count;j++)
          bufTemp[j] = buf[j];*/
        free(bufTemp); // to free the old buff
        bufTemp = malloc(MAX*count*sizeof(char)); // reallocate again
        bufTemp = buf; // to point the old buffer to the new one
        bufTemp[i] = '\0'; // printining correctly
      }
      
      // modification of exit code
      wait(&status);  // waiting for the child...
      rc = WEXITSTATUS(status);
      if(rc != lrc && cflag == 1){ // if -c option used and return code doesn't change
        // sprintf(exitPrint,"exit %d",rc);
        // printf("\n");
        // write(1,exitPrint,sizeof(exitPrint)); // wrv = write return value
        // printf("exit %d\n",rc );
        fprintf(stdout,"exit %d\n",rc);
      }
      lrc = rc; // to hold the last return code 

      // if(k == limit-1) // in the last iteration,do not need to wait for extra 10 seconds...
      //   continue;
      usleep(usec);
    }

  }

//#############################################################################################
  return 0;
}


// there is a problem with getopt when you don't give -t option everything crushes...

// limite 0 verende qaydalara diqqet ele


// genhtml LCOVOUTPUT.info --output-directory CODE_COVERAGE
// lcov --capture --directory . --output-file LCOVOUTPUT.info
