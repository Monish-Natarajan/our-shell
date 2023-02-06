#include <iostream>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <wait.h>


using namespace std;

#include "global_variables.h"
#include "execute_single_command.h"



void printPrompt(); // Function to print the prompt

int main(){

    while(true){
        printPrompt();
        string command;
        getline(cin, command);
        executeSingleCommand(command);
    }
    
    return 0;

}

// Function to print the prompt
void printPrompt(){
    getcwd(curr_working_dir, sizeof(curr_working_dir));
    cout << "our-shell:" << curr_working_dir << "$ ";
}