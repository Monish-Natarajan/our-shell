#include <dirent.h>
#include <string.h>
#include <iostream>
#include <vector>

using namespace std;

bool match( char* pattern, char* str)
{
    // If we reach at the end of both strings, we are done
    if (*pattern == '\0' && *str == '\0')
        return true;
 
    // Make sure to eliminate consecutive '*'
    if (*pattern == '*') {
        while (*(pattern + 1) == '*')
            pattern++;
    }
 
    // Make sure that the characters after '*' are present
    // in str string. This function assumes that the
    // pattern string will not contain two consecutive '*'
    if (*pattern == '*' && *(pattern + 1) != '\0'
        && *str == '\0')
        return false;
 
    // If the pattern string contains '?', or current
    // characters of both strings match
    if (*pattern == '?' || *pattern == *str)
        return match(pattern + 1, str + 1);
 
    // If there is *, then there are two possibilities
    // a) We consider current character of str string
    // b) We ignore current character of str string.
    if (*pattern == '*')
        return match(pattern + 1, str)
               || match(pattern, str + 1);
    return false;
}

vector<char *> get_filenames(char *pattern) {
  DIR *dir;
  struct dirent *ent;
  vector<char *> filenames;

  if ((dir = opendir(".")) != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      bool is_matching = match(pattern, ent->d_name);
      if (is_matching) {
        filenames.push_back(ent->d_name);
      }

    }
    closedir(dir);
  } else {
    cerr << "Error reading current directory" << endl;
    return {};
  }

  return filenames;
}

bool contains_wildcard(const string &string) {
  for(char c : string) {
    if (c == '*' || c == '?') {
      return true;
    }
  }
  return false;
}

vector<char *> substitute(char *pattern){
  if(!contains_wildcard(pattern)){
    return {pattern};
  }
  vector<char *> filenames = get_filenames(pattern);
  return filenames;
}

int main() {
  string pattern;
  cout << "Enter a pattern: ";
  cin >> pattern;
  vector<char *> filenames = substitute((char *)pattern.c_str());
  for (char *filename : filenames) {
    cout << filename << endl;
  }


  return 0;
}
