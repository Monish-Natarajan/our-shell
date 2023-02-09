#include <dirent.h>
#include <fcntl.h>
#include <readline/readline.h>
#include <signal.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <wait.h>

#include <deque>
#include <iostream>
#include <vector>

#define RESET "\x1B[0m"
#define BOLD "\x1B[1m"
#define GREEN "\033[1;32m"
#define BLUE "\033[1;34m"
#define MAXCMDLEN 1024
#define HISTSIZE 1000

using namespace std;

#include "global_variables.h"
#include "wildcards.h"

const char *printPrompt();            // Function to print the prompt
void check_background_processes();    // Function to check if any background process has finished

// Function to handle the SIGINT signal (Ctrl+C) and print the prompt after that
void sig_handler_prompt(int signum) {
    string tmpc = rl_line_buffer;
    rl_replace_line("", 0);
    rl_redisplay();
    rl_point = rl_end;
    printf("%s%s^C", printPrompt(), tmpc.c_str());
    printf("\n%s", printPrompt());
}

// Function to handle the SIGINT signal (Ctrl+C) and not print the prompt after that
void sig_handler_no_prompt(int signum) {
    printf("\n");
}

void sig_handler_ctrl_Z(int signum) {
    if (current_waiting_process != -1) {
        BACKGROUND_FLAG = 1;
        background_processes.push_back(make_pair(current_waiting_process, command));
        printf("\n[%ld] %d\n", background_processes.size(), current_waiting_process);
        fflush(stdout);
    }
    for (auto &pr : background_processes) {
        if (pr.first == -1)
            continue;    // process already finished (waitpid() was called
        kill(pr.first, SIGCONT);
    }
}

#include "execute_single_command.h"
#include "getcmd.h"

int main() {
    signal(SIGTSTP, sig_handler_ctrl_Z);    // Ctrl+Z

    fptr = fopen(".history", "a");    // used a option to create a file if doesn't exist
    fclose(fptr);
    fptr = fopen(".history", "r+");    // used r+ option to open file for r/w
    if (!fptr) {
        printf("history couldn't be accesed\n");
    }
    char str[MAXCMDLEN];
    while (fgets(str, MAXCMDLEN, fptr)) {
        int len = strlen(str);
        if (str[len - 1] == '\n')
            str[len - 1] = '\0';
        hist.push_back(str);
    }
    fseek(fptr, 0, SEEK_SET);
    bool f = 0;
    while (true) {
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
const char *printPrompt() {
    check_background_processes();
    getcwd(curr_working_dir, sizeof(curr_working_dir));
    sprintf(prompt, "%s%sSHELL++:%s%s$ %s", BOLD, GREEN, BLUE, curr_working_dir, RESET);
    fflush(stdout);
    return prompt;
}

// Function to check if any background process has finished
void check_background_processes() {
    for (int i = 0; i < background_processes.size(); i++) {
        if (background_processes[i].first == -1)
            continue;    // process already finished (waitpid() was called
        int status;
        pid_t pid = waitpid(background_processes[i].first, &status, WNOHANG);
        if (pid > 0) {
            printf("[%d] Done\t\t", i + 1);
            printf("%s\n", background_processes[i].second.c_str());
            fflush(stdout);
            background_processes[i].first = -1;
        }
    }
    while (background_processes.size() && background_processes.back().first == -1)
        background_processes.pop_back();
}