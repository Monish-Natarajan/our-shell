char *nextArg(char *&stringp){
    
    while(*stringp == ' ' || *stringp == '\t') stringp++;
    if(*stringp == '\0') return NULL;
    char *arg = stringp;

    if(*arg=='"') {
        arg++;
        stringp++;
        while(*stringp != '"' && *stringp != '\0') stringp++;
        if(*stringp == '"') *stringp++ = '\0';
        return arg;
    }

    if(*arg=='\'') {
        arg++;
        stringp++;
        while(*stringp != '\'' && *stringp != '\0') stringp++;
        if(*stringp == '\'') *stringp++ = '\0';
        return arg;
    }

    while(*stringp != ' ' && *stringp != '\t' && *stringp != '\0') 
        stringp++;

    if(*stringp != '\0') *stringp++ = '\0';
    return arg;
}

// gets arguments from a single command
void getArgs(char *stringp, vector<char *> &args, int &fInRedirect, int &fOutRedirect)
{
    while (1)
    {
        char *arg = nextArg(stringp);
        if (arg == NULL)
            break;
        if (strlen(arg) == 0)
            continue;
        else if (strcmp(arg, "&") == 0)
        {
            BACKGROUND_FLAG = 1;
        }
        else
        {
            int i = 0, j = 0;
            // check for i/o redirection(s) in extracted tokens
            while (arg[j] != '\0')
            {
                if (arg[j] == '<')
                {
                    if (i != j)
                    {
                        arg[j] = '\0';
                        args.push_back(arg + i);
                    }
                    fInRedirect = args.size();
                    i = j + 1;
                }
                else if (arg[j] == '>')
                {
                    if (i != j)
                    {
                        arg[j] = '\0';
                        args.push_back(arg + i);
                    }
                    fOutRedirect = args.size();
                    i = j + 1;
                }
                j++;
            }
            if (i != j)
            {
                char *word = arg + i;
                vector<char *> substitutes = substitute(word);
                for (char *substitute : substitutes)
                    args.push_back(substitute);
            }
        }
    }
}
 
// Function to change directory
void executeCD(vector<char *> &args)
{
    if (args.size() < 2)
    {
        cerr << "Error: cd: missing argument. Usage: cd <directory>" << endl;
        return;
    }
    else if (args.size() > 2)
    {
        cerr << "Error: cd: too many arguments. Usage: cd <directory>" << endl;
        return;
    }

    if (chdir(args[1]) != 0)
    {
        cerr << "Error: unable to change directory to \"" << args[1] << "\"" << endl;
        return;
    }
}

// Function to execute a single commands
void executeSingleCommand(string command)
{
    vector<char *> args;
    int fInRedirect = 0, fOutRedirect = 0;
    getArgs((char *)command.c_str(), args, fInRedirect, fOutRedirect);

    if (args.size() == 0)
        return;
    else if (strcmp(args[0], "exit") == 0 || strcmp(args[0], "cd") == 0) // Called from child(in case of pipe), so not useful
        exit(EXIT_SUCCESS);

    if (fInRedirect != 0)
    {
        // open input file
        int in = open(args[fInRedirect], O_RDONLY);
        if (in == -1)
        {
            cerr << "Error opening file: " << args[fInRedirect] << endl;
            exit(1);
        }
        // copy file_desc to STDIN
        dup2(in, STDIN_FILENO);
        close(in);
    }
    if (fOutRedirect != 0)
    {
        // open output file
        int out = open(args[fOutRedirect], O_WRONLY | O_TRUNC | O_CREAT, 0644);
        if (out == -1)
        {
            cerr << "Error opening file: " << args[fInRedirect] << endl;
            exit(1);
        }
        // copy file_desc to STDOUT
        dup2(out, STDOUT_FILENO);
        close(out);
    }

    args.push_back(NULL);
    char **args_ptr = &args[0];
    // Execute arguments
    if (execvp(args[0], args_ptr) < 0)
    {
        cerr << "Error in executing command" << endl;
    }
}

int execute_our_command(string command)
{
    vector<char *> args;
    int fInRedirect = 0, fOutRedirect = 0;
    getArgs((char *)command.c_str(), args, fInRedirect, fOutRedirect);

    // handle exit from shell
    if (strcmp(args[0], "exit") == 0)
    {
        while (!hist.empty())
        {
            fprintf(fptr, "%s\n", hist.front().c_str());
            hist.pop_front();
        }
        fclose(fptr);
        printf("exit\n");
        exit(0);
    }
    // handle cd from shell
    else if (strcmp(args[0], "cd") == 0)
    {
        executeCD(args);
        return 1;
    }
    return 0;
}

void execute(string command)
{
    int len = command.size();
    while (--len >= 0)
    {
        if (command[len] == '|')
        {

            pid_t pid = fork();
            if (pid == -1)
            {
                perror("fork");
                exit(EXIT_FAILURE);
            }

            if (pid == 0)
            {
                BACKGROUND_FLAG = 0;

                int pipe_fds[2];
                pid_t pid;

                if (pipe(pipe_fds) == -1)
                {
                    perror("pipe");
                    exit(EXIT_FAILURE);
                }

                pid = fork();
                if (pid == -1)
                {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }

                if (pid == 0)
                {
                    BACKGROUND_FLAG = 0;

                    // 1st process
                    close(pipe_fds[0]);
                    dup2(pipe_fds[1], STDOUT_FILENO);
                    close(pipe_fds[1]);
                    execute(command.substr(0, len));
                    exit(EXIT_SUCCESS);
                }

                else
                {

                    // 2nd process

                    close(pipe_fds[1]);
                    dup2(pipe_fds[0], STDIN_FILENO);
                    close(pipe_fds[0]);

                    executeSingleCommand(command.substr(len + 1));
                    exit(EXIT_SUCCESS);
                }
            }
            else
            {

                // parent process
                // close(pipe_fds[1]);
                // close(pipe_fds[0]);
                if (!BACKGROUND_FLAG)
                {
                    current_waiting_process = pid;
                    while(!BACKGROUND_FLAG){
                        int chek = waitpid(pid, NULL, WNOHANG);
                        if(chek == pid){
                            break;
                        }
                    }
                    current_waiting_process = -1;
                    int status = execute_our_command(command.substr(len + 1));
                }
                else
                {
                    background_processes.push_back(make_pair(pid, command));
                    printf("[%ld] %d\n", background_processes.size(), pid);
                    fflush(stdout);
                }

                // if (status == 1)
                //     return;
                return;
            }
        }
    }

    // IF NO PIPE

    // fork child to execute
    pid_t pid = fork();
    if (pid == -1)
    {
        cerr << "Failed To Fork!" << endl;
        return;
    }
    else if (pid == 0)
    {
        BACKGROUND_FLAG = 0;
        executeSingleCommand(command);
        exit(EXIT_SUCCESS);
    }
    else
    {
        if (!BACKGROUND_FLAG)
        {
            current_waiting_process = pid;
            while(!BACKGROUND_FLAG){
                int chek = waitpid(pid, NULL, WNOHANG);
                if(chek == pid){
                    break;
                }
            }
            current_waiting_process = -1;
            int status = execute_our_command(command.substr(len + 1));
        }
        else
        {
            background_processes.push_back(make_pair(pid, command));
            printf("[%ld] %d\n", background_processes.size(), pid);
            fflush(stdout);
        }
        return;
    }
}

void parseCommand(string &command)
{
    BACKGROUND_FLAG = 0;

    while (!command.empty() && (command.back() == ' ' || command.back() == '\t' || command.back() == '\n'))
        command.pop_back();

    if (command.empty())
        return;

    // find first occurance of '&' in command
    size_t found = command.find('&');
    if (found != string::npos)
    {
        if (found != command.size() - 1)
        {
            cerr << "Syntax error: tokens found after '&'" << endl;
            return;
        }
        else
        {
            command.pop_back();
            BACKGROUND_FLAG = 1;
        }
    }
    execute(command);
}