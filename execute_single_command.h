// gets arguments from a single command
void getArgs(char *stringp, vector<char *> &args){
    while(1){
        char *arg = strsep(&stringp, " ");
        if(arg == NULL) break;
        if(strlen(arg)==0) continue;
        args.push_back(arg);
    }

}

// Function to execute a single commands
void executeSingleCommand(string command){
    if(command == "exit"){
        exit(0);
    }

    vector<char *> args;
    getArgs((char *)command.c_str(), args);

    if(args.size()==0) return;

	// Forking 
	pid_t pid = fork();

    if(pid==-1){
        fprintf(stderr, "Failed To Fork!\n");
        return;
    }
    else if(pid==0){ // child process
        char **args_ptr = &args[0];
		if (execvp(args[0], args_ptr) < 0) {
			fprintf(stderr, "Error in executing command\n");
		}
		exit(0);
    }
    else{
        wait(NULL); // wait for the child to finish
        return;
    }
    

}