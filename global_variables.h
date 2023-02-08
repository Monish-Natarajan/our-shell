char curr_working_dir[1024]; // Current working directory
char prompt[1124];
int BACKGROUND_FLAG;                              // Flag to check if the command is to be run in background
vector<pair<pid_t, string>> background_processes; // Vector to store the PIDs of background processes
deque<string> hist;
string command;
char *input;
FILE *fptr;
