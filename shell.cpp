#include <iostream>
#include <deque>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <wait.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
#include <readline/readline.h>

#define RESET "\x1B[0m"
#define BOLD "\x1B[1m"
#define GREEN "\033[1;32m"
#define BLUE "\033[1;34m"
#define MAXCMDLEN 1024
#define HISTSIZE 1000

using namespace std;
#include "global_variables.h"
#include "getcmd.h"
#include "wildcards.h"

void printPrompt();                // Function to print the prompt
void check_background_processes(); // Function to check if any background process has finished
// Function to handle the SIGINT signal (Ctrl+C) and print the prompt after that
void sig_handler_prompt(int signum)
{
    printf("^C\n");
    command = "";
    printPrompt();
}
// Function to handle the SIGINT signal (Ctrl+C) and not print the prompt after that
void sig_handler_no_prompt(int signum)
{
    printf("\n");
    command = "";
}

#include "execute_single_command.h"

int main()
{
    fptr = fopen(".history", "a"); // used a option to create a file if doesn't exist
    fclose(fptr);
    fptr = fopen(".history", "r+"); // used r+ option to open file for r/w
    if (!fptr)
    {
        printf("history couldn't be accesed\n");
    }
    char str[MAXCMDLEN];
    while (fgets(str, MAXCMDLEN, fptr))
    {
        int len = strlen(str);
        if (str[len - 1] == '\n')
            str[len - 1] = '\0';
        hist.push_back(str);
    }
    fseek(fptr, 0, SEEK_SET);
    while (true)
    {
        vector ar = background_processes;
        printPrompt();
        signal(SIGINT, sig_handler_prompt);
        command = getcmd();
        signal(SIGINT, sig_handler_no_prompt);
        parseCommand(command);
        fflush(stdout);
    }
    fclose(fptr);
    return 0;
}

// Function to print the prompt
void printPrompt()
{
    check_background_processes();
    usleep(1000); // 1ms sleep
    getcwd(curr_working_dir, sizeof(curr_working_dir));
    printf("%s", BOLD);
    printf("%s", GREEN);
    cout << "our-shell:";
    printf("%s", BLUE);
    cout << curr_working_dir << "$ ";
    printf("%s", RESET);
    fflush(stdout);
}

// Function to check if any background process has finished
void check_background_processes()
{
    for (int i = 0; i < background_processes.size(); i++)
    {
        if (background_processes[i].first == -1)
            continue; // process already finished (waitpid() was called
        int status;
        pid_t pid = waitpid(background_processes[i].first, &status, WNOHANG);
        if (pid > 0)
        {
            printf("[%d] Done\t\t", i + 1);
            printf("%s\n", background_processes[i].second.c_str());
            fflush(stdout);
            background_processes[i].first = -1;
        }
    }
    while (background_processes.size() && background_processes.back().first == -1)
        background_processes.pop_back();
}