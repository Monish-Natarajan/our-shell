#include <iostream>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <wait.h>
#include <fcntl.h>
#include <signal.h>
#include <dirent.h>


using namespace std;

#include "global_variables.h"
#include "wildcards.h"
vector<pid_t> background_processes; // Vector to store the PIDs of background processes

void printPrompt(); // Function to print the prompt
void check_background_processes(); // Function to check if any background process has finished
// Function to handle the SIGINT signal (Ctrl+C) and print the prompt after that
void sig_handler_prompt(int signum){ printf("\n"); printPrompt(); }
// Function to handle the SIGINT signal (Ctrl+C) and not print the prompt after that
void sig_handler_no_prompt(int signum){ printf("\n"); }


#include "execute_single_command.h"





int main(){
    while(true){
        vector ar = background_processes;
        printPrompt();
        signal(SIGINT, sig_handler_prompt);
        string command;
        getline(cin, command);
        //printf("Input taken: %s\n", command.c_str()); fflush(stdout);
        signal(SIGINT, sig_handler_no_prompt);
        execute(command);
        fflush(stdout);
    }
    
    return 0;

}

// Function to print the prompt
void printPrompt(){
    check_background_processes();
    getcwd(curr_working_dir, sizeof(curr_working_dir));
    cout << "our-shell:" << curr_working_dir << "$ ";
    fflush(stdout);
}

// Function to check if any background process has finished
void check_background_processes(){
    for(int i=0; i<background_processes.size(); i++){
        if(background_processes[i]==-1) continue; // process already finished (waitpid() was called
        int status;
        pid_t pid = waitpid(background_processes[i], &status, WNOHANG);
        if(pid>0){
            printf("[%d] Done\n", i+1);
            fflush(stdout);
            background_processes[i] = -1;
        }
    }
    while(background_processes.size() && background_processes.back()==-1) 
        background_processes.pop_back();
}