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
        else if (ch == 127)
        {
            if (command.size())
            {
                command.pop_back();
                cout << "\b \b";
            }
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