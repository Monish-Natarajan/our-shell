#include <iostream>
#include <deque>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <wait.h>
#include <fcntl.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>

#define RESET "\x1B[0m"
#define BOLD "\x1B[1m"
#define CLR "\033[1;32m"
#define MAXCMDLEN 1024
#define HISTSIZE 1000

using namespace std;
#include "global_variables.h"
#include "execute_single_command.h"
deque<string> hist;
string command;

string getcmd()
{
    termios old_tio, new_tio;
    tcgetattr(STDIN_FILENO, &old_tio);
    new_tio = old_tio;
    new_tio.c_lflag &= ~ICANON;
    new_tio.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
    int cm = 0, size = hist.size();
    command = "";
    hist.push_back(command);
    char ch;
    while (1)
    {
        ch = getchar();
        if (ch == '\033')
        {
            getchar();
            ch = getchar();
            switch (ch)
            {
            case 'A':
                if (cm < size)
                {
                    hist.at(size - cm) = command;
                    cm++;
                    int s = command.size();
                    while (s--)
                        cout << "\b \b";
                    command = hist.at(size - cm);
                    cout << command;
                }
                break;
            case 'B':
                if (cm)
                {
                    hist.at(size - cm) = command;
                    cm--;
                    int s = command.size();
                    while (s--)
                        cout << "\b \b";
                    command = hist.at(size - cm);
                    cout << command;
                }
                break;
                // case 'H':
            }
        }
        else if (ch == 127 && command.size())
        {
            command.pop_back();
            cout << "\b \b";
        }
        else if (ch != '\n')
        {
            command.push_back(ch);
            cout << ch;
        }
        else
        {
            printf("\n");
            hist.at(size) = command;
            if (command.size() == 0 || (size && hist.at(size - 1) == hist.at(size)))
                hist.pop_back();
            break;
        }
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
    return command;
}

void printPrompt(); // Function to print the prompt
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
    printf("^C\n");
    command = "";
}

int main()
{
    FILE *fptr = fopen(".history.txt", "r+");
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
        printPrompt();
        signal(SIGINT, sig_handler_prompt);
        // string command;
        // getline(cin, command);
        command = getcmd();
        if (command == "exit")
        {
            while (!hist.empty())
            {
                fprintf(fptr, "%s\n", hist.front().c_str());
                hist.pop_front();
            }
        }
        signal(SIGINT, sig_handler_no_prompt);
        execute(command);
    }
    // while (!hist.empty())
    // {
    //     fprintf(fptr, "%s\n", hist.front().c_str());
    //     hist.pop_front();
    // }
    fclose(fptr);
    return 0;
}

// Function to print the prompt
void printPrompt()
{
    getcwd(curr_working_dir, sizeof(curr_working_dir));
    printf("%s", CLR);
    printf("%s", BOLD);
    cout << "our-shell:" << curr_working_dir << "$ ";
    printf("%s", RESET);
    fflush(stdout);
}