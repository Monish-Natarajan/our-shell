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

void printPrompt(); // Function to print the prompt
void check_background_processes(); // Function to check if any background process has finished
// Function to handle the SIGINT signal (Ctrl+C) and print the prompt after that
void sig_handler_prompt(int signum){ printf("\n"); printPrompt(); }
// Function to handle the SIGINT signal (Ctrl+C) and not print the prompt after that
void sig_handler_no_prompt(int signum){ printf("\n"); }

void sig_handler_ctrl_Z(int signum){
    if(current_waiting_process != -1){
        BACKGROUND_FLAG = 1;
        background_processes.push_back(make_pair(current_waiting_process, "DUMMY"));
        printf("[%ld] %d\n",background_processes.size(), current_waiting_process);
        fflush(stdout);
        current_waiting_process = -1;
    }
}


#include "execute_single_command.h"





int main(){
    signal(SIGTSTP, sig_handler_ctrl_Z);
    while(true){
        vector ar = background_processes;
        printPrompt();
        signal(SIGINT, sig_handler_prompt);
        string command;
        getline(cin, command);
        //printf("Input taken: %s\n", command.c_str()); fflush(stdout);
        signal(SIGINT, sig_handler_no_prompt);
        parseCommand(command);
        fflush(stdout);
    }
    
    return 0;

}

// Function to print the prompt
void printPrompt(){
    check_background_processes();
    usleep(1000); // 1ms sleep
    getcwd(curr_working_dir, sizeof(curr_working_dir));
    for (auto&i:background_processes) {
        cout << i.first << " " << i.second << endl;
    }
    cout << "our-shell:" << curr_working_dir << "$ ";
    fflush(stdout);
}

// Function to check if any background process has finished
void check_background_processes(){
    for(int i=0; i<background_processes.size(); i++){
        if(background_processes[i].first==-1) continue; // process already finished (waitpid() was called
        int status;
        pid_t pid = waitpid(background_processes[i].first, &status, WNOHANG);
        if(pid>0){
            printf("[%d] Done\t\t", i+1);
            printf("%s\n", background_processes[i].second.c_str());
            fflush(stdout);
            background_processes[i].first = -1;
        }
    }
    while(background_processes.size() && background_processes.back().first==-1) 
        background_processes.pop_back();
}