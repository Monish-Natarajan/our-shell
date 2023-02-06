

// gets arguments from a single command
void getArgs(char *stringp, vector<char *> &args, int &fInRedirect, int &fOutRedirect){
    while(1){
        char *arg = strsep(&stringp, " \t");
        if(arg == NULL) break;
        if(strlen(arg)==0) continue;
        else    {
            int i=0, j=0;
            // check for i/o redirection(s) in extracted tokens
            while(arg[j]!='\0') {
                if(arg[j]=='<') {
                    if(i!=j) {
                        arg[j] = '\0';
                        args.push_back(arg + i);
                    }
                    fInRedirect = args.size();
                    i=j+1;
                }
                else if(arg[j]=='>') {
                    if(i!=j) {
                        arg[j] = '\0';
                        args.push_back(arg + i);
                    }
                    fOutRedirect = args.size();
                    i=j+1;
                }
                j++;
            }
            if(i!=j) args.push_back(arg + i);
        }
    }
}

// Function to change directory
void executeCD(vector<char *> &args) {
    if (args.size() < 2) {
        cerr << "Error: missing argument. Usage: cd <directory>" << endl;
        return;
    }

    if (chdir(args[1]) != 0) {
        cerr << "Error: unable to change directory to \"" << args[1] << "\"" << endl;
        return;
    }
}

// Function to execute a single commands
void executeSingleCommand(string command){

    vector<char *> args;
    int fInRedirect=0, fOutRedirect=0;
    getArgs((char *)command.c_str(), args, fInRedirect, fOutRedirect);

    if(args.size()==0) return;

    // handle exit from shell
    if(strcmp(args[0], "exit") == 0) {
        exit(0);
    }
    // handle exit from shell
    else if(strcmp(args[0], "cd") == 0) {
        executeCD(args);
        return;
    }

	// fork child to execute 
	pid_t pid = fork();

    if(pid==-1){
        cerr << "Failed To Fork!" << endl;
        return;
    }
    else if(pid==0){ // child process
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

        char **args_ptr = &args[0];
        // Execute arguments
		if (execvp(args[0], args_ptr) < 0) {
			cerr << "Error in executing command" << endl;
		}
		exit(0);
    }
    else{
        wait(NULL); // wait for the child to finish
        return;
    }
}