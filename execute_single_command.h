// gets arguments from a single command
void getArgs(char *stringp, vector<char *> &args, int &fInRedirect, int &fOutRedirect){
    while(1){
        char *arg = strsep(&stringp, " \t");
        if(arg == NULL) break;
        if(strlen(arg)==0) continue;
        else if (strcmp(arg, "<") == 0) fInRedirect = args.size();
        else if (strcmp(arg, ">") == 0) fOutRedirect = args.size();
        else    args.push_back(arg);
    }
    // cout<<fInRedirect<<' '<<fOutRedirect<<'\n';
    // for(auto&a:args) cout<<a<<",";
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