#include <iostream>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <wait.h>
#include <fcntl.h>
#include <signal.h>


using namespace std;

#include "global_variables.h"
#include "execute_single_command.h"



void printPrompt(); // Function to print the prompt
// Function to handle the SIGINT signal (Ctrl+C) and print the prompt after that
void sig_handler_prompt(int signum){ printf("\n"); printPrompt(); }
// Function to handle the SIGINT signal (Ctrl+C) and not print the prompt after that
void sig_handler_no_prompt(int signum){ printf("\n"); }

int main(){
    while(true){
        printPrompt();
        signal(SIGINT, sig_handler_prompt);
        string command;
        getline(cin, command);
        signal(SIGINT, sig_handler_no_prompt);
        executeSingleCommand(command);
    }
    
    return 0;

}

// Function to print the prompt
void printPrompt(){
    getcwd(curr_working_dir, sizeof(curr_working_dir));
    cout << "our-shell:" << curr_working_dir << "$ ";
    fflush(stdout);
}