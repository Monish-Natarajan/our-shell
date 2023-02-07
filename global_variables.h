char curr_working_dir[1024]; // Current working directory
int BACKGROUND_FLAG; // Flag to check if the command is to be run in background
vector<pair<pid_t, string>> background_processes; // Vector to store the PIDs of background processes