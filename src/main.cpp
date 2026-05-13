#include <iostream>
#include <string>
#include <sstream>
#include <unordered_set>
#include <unistd.h>
#include <vector>
#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <readline/readline.h>
#include <dirent.h>

using namespace std;

string is_exec(string cmd){

    string path  = getenv("PATH");
    vector<string>directories;
    string tmp;
    string pth = "";

    for(auto c : path){
      if(c==':'){
        directories.push_back(tmp);
        tmp.clear();
      }else{
          tmp.push_back(c);
      }
    }
    directories.push_back(tmp);

    bool chk = false;

    for(auto &d : directories){
      string cur_pth = d;
      cur_pth.push_back('/');
      cur_pth += cmd;
      if(access(cur_pth.c_str(), X_OK) == 0){
          return cur_pth;
      }
    }

    return pth;

}

vector<string>tokenize( string &command){
    vector<string>v;
    string tmp;

    bool squotes = false;
    bool dquotes = false;

    for (int i = 0; i < command.size(); i++)
    {
      if(command[i] != '\'' && squotes){
         tmp.push_back(command[i]);
      }else if(command[i] != '"' && dquotes){
            if(i+1 < command.size() && command[i] == '\\' && (command[i+1]=='\\' || command[i+1]=='"')){
                tmp.push_back(command[i+1]);
                i++;
                continue;
            }
        tmp.push_back(command[i]);
      }else if(command[i]== '\'' && !squotes){
        squotes = true;
      }else if(command[i] == '"' && !dquotes){
        dquotes = true;
      }else if( command[i] == '\'' && squotes){
        squotes = false;
      }else if(command[i] == '"' && dquotes){
        dquotes = false;
      }else if(command[i] == ' '){
          if(!tmp.empty()){
              v.push_back(tmp);
              tmp.clear();
            }
      }else if(command[i]== '\\'){
         if(i+1 < command.size()){
            tmp.push_back(command[i+1]);
            i++;
         }

      }else{
           tmp.push_back(command[i]);
      }
    }

    if(!tmp.empty())v.push_back(tmp);

    return v;
    
}

string parse_redirection(vector<string>&args){

  if(args[args.size()-2] == ">" || args[args.size()-2] == "1>"){
      string f = args.back();
      args.pop_back();
      args.pop_back();

      return f;
  }

  return "";
  
}

string parse_append(vector<string>&args){

    if(args[args.size()-2] == ">>" || args[args.size()-2] == "1>>"){
      string f = args.back();
      args.pop_back();
      args.pop_back();

      return f;
  }

  return "";

}

string parse_err_redirection(vector<string>&args){

  if(args[args.size()-2] == "2>" ){
      string f = args.back();
      args.pop_back();
      args.pop_back();

      return f;
  }

  return "";
  
}

string parse_error_append(vector<string>&args){

    if(args[args.size()-2] == "2>>" ){
      string f = args.back();
      args.pop_back();
      args.pop_back();

      return f;
  }

  return "";

}

void apply_redirection(string ofname, string efname){
  if(!ofname.empty()){
         int fd = open(ofname.c_str(),O_WRONLY | O_CREAT | O_TRUNC,0644);
         dup2(fd, 1);
         close(fd);
  }

    if(!efname.empty()){
         int fd = open(efname.c_str(),O_WRONLY | O_CREAT | O_TRUNC,0644);
         dup2(fd, 2);
         close(fd);
  }
}

void restore_redirection(int f_saved, int e_saved){
    dup2( f_saved, 1);
    dup2(e_saved, 2);
    close(f_saved);
    close(e_saved);
}

void apply_append_redirection(string oa_name, string ea_name){
  if(!oa_name.empty()){
         int fd = open(oa_name.c_str(),O_WRONLY | O_CREAT | O_APPEND,0644);
         dup2(fd, 1);
         close(fd);
  }

    if(!ea_name.empty()){
         int fd = open(ea_name.c_str(),O_WRONLY | O_CREAT | O_APPEND,0644);
         dup2(fd, 2);
         close(fd);
  }
}

unordered_set<string> builtin_list = {
    "echo",
    "type",
    "exit",
    "pwd",
    "cd"
};

unordered_set<string>executables;


char* command_generator(const char* text, int state) {
    static int index;
    static vector<string> matches;

    if(state == 0){
        index = 0;
        matches.clear();

        string prefix(text);

        for(auto &cmd : builtin_list){
            if(cmd.substr(0, prefix.size()) == prefix){
                matches.push_back(cmd);
            }
        }

        for(auto &cmd : executables){
            if(cmd.substr(0, prefix.size()) == prefix){
                matches.push_back(cmd);
            }
        }
    }

    if(index < matches.size()){
        return strdup(matches[index++].c_str());
    }

    return nullptr;
}

char** command_completion(const char* text, int start, int end) {

    rl_attempted_completion_over = 1;

    return rl_completion_matches(text, command_generator);
}


void find_all_executables(){
   string path = getenv("PATH");
   vector<string>dir;
   string tmp;

   for(auto c : path){
    if(c == ':'){
      dir.push_back(tmp);
      tmp.clear();
    }else{
      tmp.push_back(c);
    }
   }
   dir.push_back(tmp);

   for(auto c : dir){
     struct dirent *entry;
     string pth = c;
     DIR *dp = opendir(c.c_str());
      if (dp == NULL) {
        continue;   
    }

     while ((entry = readdir(dp)) != NULL) {
      string cmd =  entry->d_name;
      pth.append(cmd);
      if(access(pth.c_str(),X_OK)==0){
        executables.insert(cmd);
      }

    }
    closedir(dp);

   }

}



int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
   find_all_executables();
  rl_attempted_completion_function = command_completion;

  // TODO: Uncomment the code below to pass the first stage
  string command;
  unordered_set<string>builtin_commands = {"echo", "type", "exit", "pwd", "cd"};  

  while(true){
    char* input = readline("$ ");
    command = string(input);
    free(input);
    if(command.empty())continue;
    vector<string>args = tokenize(command);
    int f_saved  = dup(1);
    int e_saved  = dup(2);
    string ofname = parse_redirection(args);
    string efname = parse_err_redirection(args);
    string oa_name = parse_append(args);
    string ea_name = parse_error_append(args);

    
    // ss >> cmd >> arg;
    if(args[0] == "exit"){
      break;
    }else if( args[0] == "echo"){
        apply_redirection(ofname, efname);
        apply_append_redirection(oa_name, ea_name);
        for (int i = 1; i < args.size(); i++)
        {
           cout << args[i] <<" ";
        }
        cout<<"\n";
        restore_redirection(f_saved, e_saved);
      
        
    }else if(args[0] == "type"){
       apply_redirection(ofname, efname);
        apply_append_redirection(oa_name, ea_name);
       
       if(builtin_commands.find(args[1])!= builtin_commands.end()){
          cout << args[1] << " is a shell builtin\n";
       }else{
            string pth = is_exec(args[1]);
           if(!pth.empty()){
                cout << args[1] <<" is "<< pth<<"\n";
           }else{
             cout << args[1] <<": not found\n";
           }
       }
        restore_redirection(f_saved, e_saved);

    }else if(args[0] == "pwd"){
       apply_redirection(ofname, efname);
        apply_append_redirection(oa_name, ea_name);

        char cwd[1024];
        if(getcwd(cwd, sizeof(cwd)) != NULL){
           cout << cwd<<"\n";
        }
        restore_redirection(f_saved, e_saved);

    
    }else if(args[0]=="cd"){
       apply_redirection(ofname, efname);
        apply_append_redirection(oa_name, ea_name);

      if(args[1] == "~"){
        string home  = getenv("HOME");
        int op = chdir(home.c_str());
        if(op==-1){
        cout << "cd: " << args[1] <<": No such file or directory\n";
            }
        
      }else{
      const char* dir = args[1].c_str();
      int op = chdir(dir);
       if(op==-1){
        cout << "cd: " << args[1] <<": No such file or directory\n";
       }
      }
        restore_redirection(f_saved, e_saved);

    
    }else{
            string pth = is_exec(args[0]);
            vector<char*>c_args;
            for(auto &c : args){
               char* strng = (char*)c.c_str();
               c_args.push_back(strng);
            }
            c_args.push_back(nullptr);

            if(!pth.empty()){
                pid_t pid = fork();

                if(pid == 0){
                  apply_redirection(ofname, efname);
                   apply_append_redirection(oa_name, ea_name);

                  execvp(pth.c_str(),&c_args[0]  );
                  perror("execvp");
                  exit(1);

                }else{
                  waitpid(pid, nullptr, 0);
                }

                
           }else{
                  apply_redirection(ofname, efname);
                  apply_append_redirection(oa_name, ea_name);


              cout << args[0]<<": command not found\n";


           }

    }
  }
}
