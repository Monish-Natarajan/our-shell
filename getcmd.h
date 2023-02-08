int cm, sz;

int up_arrow_function(int value, int count)
{
    if (cm < sz)
    {
        hist.at(sz - cm) = rl_line_buffer;
        cm++;
        rl_replace_line(hist.at(sz - cm).c_str(), 0);
        rl_point = rl_end;
        rl_redisplay();
    }
    return 0;
}

int down_arrow_function(int value, int count)
{
    if (cm)
    {
        hist.at(sz - cm) = rl_line_buffer;
        cm--;
        rl_replace_line(hist.at(sz - cm).c_str(), 0);
        rl_point = rl_end;
        rl_redisplay();
    }
    return 0;
}

string getcmd()
{
    cm = 0;
    sz = hist.size();
    hist.push_back("");
    rl_command_func_t up_arrow_function, down_arrow_function;
    rl_bind_keyseq("\033[A", up_arrow_function);
    rl_bind_keyseq("\033[B", down_arrow_function);
    input = readline(printPrompt());

    command = input;
    hist.at(sz) = command;
    if (sz && hist.at(sz - 1) == command)
        hist.pop_back();
    free(input);

    return command;
}