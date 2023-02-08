bool match(char *pattern, char *str)
{
    // If we reach at the end of both strings, we are done
    if (*pattern == '\0' && *str == '\0')
        return true;

    // Make sure to eliminate consecutive '*'
    if (*pattern == '*')
    {
        while (*(pattern + 1) == '*')
            pattern++;
    }

    // Make sure that the characters after '*' are present
    // in str string. This function assumes that the
    // pattern string will not contain two consecutive '*'
    if (*pattern == '*' && *(pattern + 1) != '\0' && *str == '\0')
        return false;

    // If the pattern string contains '?', or current
    // characters of both strings match
    if (*pattern == '?' || *pattern == *str)
        return match(pattern + 1, str + 1);

    // If there is *, then there are two possibilities
    // a) We consider current character of str string
    // b) We ignore current character of str string.
    if (*pattern == '*')
        return match(pattern + 1, str) || match(pattern, str + 1);
    return false;
}

vector<char *> get_filenames(char *pattern)
{
    DIR *dir;
    struct dirent *ent;
    vector<char *> filenames;

    if ((dir = opendir(".")) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            char *fname = (char *)malloc(sizeof(char) * (strlen(ent->d_name) + 1));
            strcpy(fname, ent->d_name);
            bool is_matching = match(pattern, fname);
            if (is_matching)
            {
                filenames.push_back(fname);
            }
            else
            {
                free(fname);
            }
        }
        closedir(dir);
    }
    else
    {
        cerr << "Error reading current directory" << endl;
        return {};
    }

    return filenames;
}

bool contains_wildcard(const string &string)
{
    for (char c : string)
    {
        if (c == '*' || c == '?')
        {
            return true;
        }
    }
    return false;
}

// substitutes wildcards in the pattern with the actual filenames
vector<char *> substitute(char *pattern)
{
    if (!contains_wildcard(pattern))
    {
        char *temp = (char *)malloc(sizeof(char) * (strlen(pattern) + 1));
        strcpy(temp, pattern);
        return {temp};
    }
    vector<char *> filenames = get_filenames(pattern);
    return filenames;
}