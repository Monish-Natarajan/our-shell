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
    if(command == "exit"){
        exit(0);
    }

    vector<char *> args;
    int fInRedirect=0, fOutRedirect=0;
    getArgs((char *)command.c_str(), args, fInRedirect, fOutRedirect);

    if(args.size()==0) return;

	// Forking 
	pid_t pid = fork();

    if(pid==-1){
        cerr << "Failed To Fork!\n";
        return;
    }
    else if(pid==0){ // child process

        if (fInRedirect != 0) {
            int in = open(args[fInRedirect], O_RDONLY);
            dup2(in, 0);
            close(in);
        }
        if (fOutRedirect != 0) {
            int out = open(args[fOutRedirect], O_WRONLY | O_TRUNC | O_CREAT, 0644);
            dup2(out, 1);
            close(out);
        }
        char **args_ptr = &args[0];
		if (execvp(args[0], args_ptr) < 0) {
			cerr << "Error in executing command\n";
		}
		exit(0);
    }
    else{
        wait(NULL); // wait for the child to finish
        return;
    }
    

}