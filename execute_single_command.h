
int get_cpu_util(pid_t pid) {
    string line;
    string path = "/proc/" + to_string(pid) + "/stat";
    ifstream stat_stream(path);
    if (!stat_stream.is_open()) {
        cerr << "Error: failed to open file '" << path << "'" << endl;
    }
    getline(stat_stream, line);
    istringstream line_stream(line);

    // The CPU utilization is stored as the 14th and 15th values in the line.
    // The memory utilization is stored as the 22nd value in the line.
    long long utime, stime, start_time, uptime;
    for (int i = 0; i < 23; ++i) {
        if (i == 13)
            line_stream >> utime;
        else if (i == 14)
            line_stream >> stime;
        else if (i == 22) {
            line_stream >> start_time;
            // cout << "reading start time " << start_time << "\n";
        } else
            line_stream.ignore(numeric_limits<streamsize>::max(), ' ');
    }

    string path2 = "/proc/uptime";
    ifstream stat_stream2(path2);
    if (!stat_stream2.is_open()) {
        cerr << "Error: failed to open file '" << path2 << "'" << endl;
    }
    getline(stat_stream2, line);
    istringstream line_stream2(line);
    line_stream2 >> uptime;
    uptime *= 100;    // converting to clock ticks
    double cpu_util = (stime + utime) * 100.0 / (uptime - start_time);

    // Print the results.
    // cout << "utime and stime " << utime << " + " << stime << endl;
    // cout << "Start Time: " << start_time << endl;
    // cout << "Uptime: " << uptime << " " << endl;
    // cout << "CPU Utilization: " << cpu_util << "\n";
    return cpu_util;
}

vector<string> list_dir(string path) {
    vector<string> list_dirs;
    DIR *d;
    struct dirent *dir;
    d = opendir(path.c_str());
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            list_dirs.push_back(dir->d_name);
        }
        closedir(d);
    }
    return list_dirs;
}

int get_heur(pid_t pid) {
    int heur = 0, child_util;
    vector<string> list_dirs;
    string path = "/proc/" + to_string(pid) + "/task";
    list_dirs = list_dir(path);

    for (int i = 0; i < list_dirs.size(); i++) {
        if (i == 0 || i == 1) continue;
        string tid = list_dirs[i];
        // cout << tid << " ";
        path = "/proc/" + to_string(pid) + "/task/" + tid + "/children";
        ifstream stat_stream(path);
        if (!stat_stream.is_open()) {
            cerr << "Error: failed to open file '" << path << "'" << endl;
            return 2;
        }
        string line;
        getline(stat_stream, line);
        istringstream line_stream(line);
        pid_t cpid;

        while (!line_stream.eof()) {
            line_stream >> cpid;
            if (cpid <= 0) continue;
            child_util = get_cpu_util(cpid);
            cout << "child " << cpid << " utilization " << child_util << "%\n";
            heur += child_util;
        }
    }
    return heur;
}







char *nextArg(char *&stringp) {
    while (*stringp == ' ' || *stringp == '\t') stringp++;
    if (*stringp == '\0') return NULL;
    char *arg = stringp;

    if (*arg == '"') {
        arg++;
        stringp++;
        while (1) {    //*stringp != '"' && *stringp != '\0'){
            if (*stringp == '\\') {
                if (*(stringp + 1) == '"' || *(stringp + 1) == '\'' || *(stringp + 1) == '\\') {
                    strcpy(stringp, stringp + 1);
                }
                stringp++;
            }
            if (*stringp == '"') break;
            if (*stringp == '\0') break;
            stringp++;
        }
        if (*stringp == '"') *stringp++ = '\0';
        return arg;
    }

    if (*arg == '\'') {
        arg++;
        stringp++;
        while (1) {    //*stringp != '"' && *stringp != '\0'){
            if (*stringp == '\\') {
                if (*(stringp + 1) == '\'' || *(stringp + 1) == '"' || *(stringp + 1) == '\\') {
                    strcpy(stringp, stringp + 1);
                }
                stringp++;
            }
            if (*stringp == '\'') break;
            if (*stringp == '\0') break;
            stringp++;
        }
        if (*stringp == '\'') *stringp++ = '\0';
        return arg;
    }

    while (*stringp != ' ' && *stringp != '\t' && *stringp != '\0')
        stringp++;

    if (*stringp != '\0') *stringp++ = '\0';
    return arg;
}

// gets arguments from a single command
void getArgs(char *stringp, vector<char *> &args, int &fInRedirect, int &fOutRedirect) {
    while (1) {
        char *arg = nextArg(stringp);
        if (arg == NULL)
            break;
        if (strlen(arg) == 0)
            continue;
        else if (strcmp(arg, "&") == 0) {
            BACKGROUND_FLAG = 1;
        } else {
            int i = 0, j = 0;
            // check for i/o redirection(s) in extracted tokens
            while (arg[j] != '\0') {
                if (arg[j] == '<') {
                    if (i != j) {
                        arg[j] = '\0';
                        args.push_back(arg + i);
                    }
                    fInRedirect = args.size();
                    i = j + 1;
                } else if (arg[j] == '>') {
                    if (i != j) {
                        arg[j] = '\0';
                        args.push_back(arg + i);
                    }
                    fOutRedirect = args.size();
                    i = j + 1;
                }
                j++;
            }
            if (i != j) {
                char *word = arg + i;
                vector<char *> substitutes = substitute(word);
                for (char *substitute : substitutes)
                    args.push_back(substitute);
            }
        }
    }
}

// Function to execute pwd
void executePwd() {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("Error: Unable to get current working directory\n");
    }
}

// Function to get pids of processes having file open or holding lock over file
void get_pids(string filepath, vector<pid_t> &pids) {
    string ex = "lsof -t " + filepath;
    FILE *fp = popen(ex.c_str(), "r");
    if (fp == NULL) {
        perror("Error running lsof");
        return;
    }
    int p;
    while (fscanf(fp, "%d", &p) == 1) {
        pids.push_back(p);
    }
    pclose(fp);
}

// Function to execute delep **delete with extreme prejudice**
void execeuteDelep(vector<char *> &args) {
    if (args.size() != 2) {
        perror("Syntax error: Usage: delep <<filepath>>\n");
        return;
    }
    vector<pid_t> pids;
    get_pids(args[1], pids);
    bool consent = 0;
    if (pids.empty()) {
        printf("No process found with open file: %s\n", args[1]);
        if (remove(args[1]) != 0)
            perror("Error deleting file\n");
        else
            printf("%s deleted succesfully!\n", args[1]);
    } else {
        printf("The following processes have the file open or are holding a lock:\n");
        for (auto &p : pids) printf("%d ", p);
        char ans[MAX_INPUT];
        printf("\nDo you want to kill all these processes and delete file? (yes/no): ");
        scanf("%s", ans);
        if (strcmp(ans, "yes") == 0) {
            for (auto &p : pids) {
                // Function to kill process with given pid
                kill(p, SIGKILL);
            }
            if (remove(args[1]) != 0)
                perror("Error deleting file\n");
            else
                printf("%s deleted succesfully!\n", args[1]);
        }
    }
}

// Function to execute sb **squash bug**
void executeSb(vector<char *> &args) {
    if (args.size() < 2 || args.size() > 3) {
        perror("Syntax error: Usage: sb <<pid>> [--suggest]\n");
        return;
    }
    int pid = atoi(args[1]);
    bool suggest = 0;
    if (args.size() == 3)
        suggest = 1;
    string path, status;
    pid_t ppid, ptid, mlw_pid = -1;
    // ppid -> id of parent process, ptid -> id of controlling terminal

    for (int i = 0; i < 3; i++) {
        // open /proc/[pid]/stat
        path = "/proc/" + to_string(pid) + "/stat";
        ifstream stat_stream(path);
        if (!stat_stream.is_open()) {
            cerr << "Error: failed to open file '" << path << "'" << endl;
            return;
        }

        string line;
        getline(stat_stream, line);
        istringstream line_stream(line);

        for (int i = 0; i < 7; ++i) {
            if (i == 2) line_stream >> status;
            if (i == 3) line_stream >> ppid;
            if (i == 6)
                line_stream >> ptid;
            else
                line_stream.ignore(numeric_limits<streamsize>::max(), ' ');
        }
        cout << "gen  " << i << " pid: " << pid << " status : " << status << endl;

        if (suggest == 1) {
            int heur = get_heur(pid);
            cout << "heuristic " << heur << " status " << status << "\n";
            if (heur > ALERT_LIMIT && status == "S")    // can replace with diff checker_fn()
            {
                mlw_pid = pid;
                break;
            }
        }
        pid = ppid;
    }
    if (suggest == 1) {
        if (mlw_pid > 0)
            cout << "Detected Malware PID : " << mlw_pid << "\n";
        else
            cout << "Malware not found \n";
    }
}

// Function to execute a single commands
void executeSingleCommand(string command) {
    vector<char *> args;
    int fInRedirect = 0, fOutRedirect = 0;
    getArgs((char *)command.c_str(), args, fInRedirect, fOutRedirect);

    if (args.size() == 0)
        return;
    else if (strcmp(args[0], "exit") == 0 || strcmp(args[0], "cd") == 0)
        // Called from child(in case of pipe), so not useful
        exit(EXIT_SUCCESS);
    // handle pwd from shell
    else if (strcmp(args[0], "pwd") == 0) {
        executePwd();
        exit(EXIT_SUCCESS);
    }
    // handle delep from shell
    else if (strcmp(args[0], "delep") == 0) {
        execeuteDelep(args);
        exit(EXIT_SUCCESS);
    }
    // handle sb (squash bug) from shell
    else if (strcmp(args[0], "sb") == 0) {
        executeSb(args);
        exit(EXIT_SUCCESS);
    }

    if (fInRedirect != 0) {
        // open input file
        int in = open(args[fInRedirect], O_RDONLY);
        if (in == -1) {
            cerr << "Error opening file: " << args[fInRedirect] << endl;
            exit(1);
        }
        // copy file_desc to STDIN
        dup2(in, STDIN_FILENO);
        close(in);
    }
    if (fOutRedirect != 0) {
        // open output file
        int out = open(args[fOutRedirect], O_WRONLY | O_TRUNC | O_CREAT, 0644);
        if (out == -1) {
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
    if (execvp(args[0], args_ptr) < 0) {
        cerr << "Error in executing command" << endl;
    }
}

// Function to change directory
void executeCD(vector<char *> &args) {
    if (args.size() < 2) {
        cerr << "Error: cd: missing argument. Usage: cd <directory>" << endl;
        return;
    } else if (args.size() > 2) {
        cerr << "Error: cd: too many arguments. Usage: cd <directory>" << endl;
        return;
    }

    if (chdir(args[1]) != 0) {
        cerr << "Error: unable to change directory to \"" << args[1] << "\"" << endl;
        return;
    }
}

int execute_our_command(string command) {
    vector<char *> args;
    int fInRedirect = 0, fOutRedirect = 0;
    getArgs((char *)command.c_str(), args, fInRedirect, fOutRedirect);

    // handle exit from shell
    if (strcmp(args[0], "exit") == 0) {
        while (!hist.empty()) {
            fprintf(fptr, "%s\n", hist.front().c_str());
            hist.pop_front();
        }
        fclose(fptr);
        printf("exit\n");
        exit(0);
    }
    // handle cd from shell
    else if (strcmp(args[0], "cd") == 0) {
        executeCD(args);
        return 1;
    }
    return 0;
}

void execute(string command) {
    int len = command.size();
    while (--len >= 0) {
        if (command[len] == '|') {
            pid_t pid = fork();
            if (pid == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            }

            if (pid == 0) {
                BACKGROUND_FLAG = 0;

                int pipe_fds[2];
                pid_t pid;

                if (pipe(pipe_fds) == -1) {
                    perror("pipe");
                    exit(EXIT_FAILURE);
                }

                pid = fork();
                if (pid == -1) {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }

                if (pid == 0) {
                    BACKGROUND_FLAG = 0;

                    // 1st process
                    close(pipe_fds[0]);
                    dup2(pipe_fds[1], STDOUT_FILENO);
                    close(pipe_fds[1]);
                    execute(command.substr(0, len));
                    exit(EXIT_SUCCESS);
                }

                else {
                    // 2nd process

                    close(pipe_fds[1]);
                    dup2(pipe_fds[0], STDIN_FILENO);
                    close(pipe_fds[0]);

                    executeSingleCommand(command.substr(len + 1));
                    exit(EXIT_SUCCESS);
                }
            } else {
                // parent process
                // close(pipe_fds[1]);
                // close(pipe_fds[0]);
                if (!BACKGROUND_FLAG) {
                    current_waiting_process = pid;
                    while (!BACKGROUND_FLAG) {
                        int chek = waitpid(pid, NULL, WNOHANG);
                        if (chek == pid) {
                            break;
                        }
                    }
                    current_waiting_process = -1;
                    int status = execute_our_command(command.substr(len + 1));
                } else {
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
    if (pid == -1) {
        cerr << "Failed To Fork!" << endl;
        return;
    } else if (pid == 0) {
        BACKGROUND_FLAG = 0;
        executeSingleCommand(command);
        exit(EXIT_SUCCESS);
    } else {
        if (!BACKGROUND_FLAG) {
            current_waiting_process = pid;
            while (!BACKGROUND_FLAG) {
                int chek = waitpid(pid, NULL, WNOHANG);
                if (chek == pid) {
                    break;
                }
            }
            current_waiting_process = -1;
            int status = execute_our_command(command.substr(len + 1));
        } else {
            background_processes.push_back(make_pair(pid, command));
            printf("[%ld] %d\n", background_processes.size(), pid);
            fflush(stdout);
        }
        return;
    }
}

void parseCommand(string &command) {
    BACKGROUND_FLAG = 0;

    while (!command.empty() && (command.back() == ' ' || command.back() == '\t' || command.back() == '\n'))
        command.pop_back();

    if (command.empty())
        return;

    // find first occurance of '&' in command
    size_t found = command.find('&');
    if (found != string::npos) {
        if (found != command.size() - 1) {
            cerr << "Syntax error: tokens found after '&'" << endl;
            return;
        } else {
            command.pop_back();
            BACKGROUND_FLAG = 1;
        }
    }
    execute(command);
}

