#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sstream>
#include <vector>

using namespace std;

const int MAX_ARGS = 10;
const int MAX_LEN = 256;

vector<string> tokenize(string input) {
    stringstream ss(input);
    vector<string> tokens;
    string token;
    while (ss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

int main() {
    while (true) {
        cout<<"$ ";

        string input;
        getline(cin, input);
        vector<string> args = tokenize(input);
        
        char *argv[MAX_ARGS];
        for (int i = 0; i < args.size(); i++) {
            argv[i] = (char*) args[i].c_str();
        }
        argv[args.size()] = NULL;

        pid_t pid = fork();
        if (pid == 0) {
            execvp(argv[0], argv);
            perror("execvp failed");
            return 1;
        } else if (pid > 0) {
            int status;
            wait(&status);
        } else {
            perror("fork failed");
            return 1;
        }
    }

    return 0;
}
