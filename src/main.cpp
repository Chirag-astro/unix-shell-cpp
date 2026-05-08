#include <iostream>
#include <string>
#include <sstream>
#include <unordered_set>
#include <unistd.h>
#include <vector>
#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>

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
            if(i+1 < command.size() && (command[i+1]=='\\' || command[i+1]=='"')){
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

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  // TODO: Uncomment the code below to pass the first stage
  string command;
  unordered_set<string>builtin_commands = {"echo", "type", "exit", "pwd", "cd"};  

  while(true){
    std::cout << "$ ";
    
    getline(cin, command);
    if(command.empty())continue;
    vector<string>args = tokenize(command);
    
    // ss >> cmd >> arg;
    if(args[0] == "exit"){
      break;
    }else if( args[0] == "echo"){
        for (int i = 1; i < args.size(); i++)
        {
           cout << args[i] <<" ";
        }
        cout<<"\n";
        
    }else if(args[0] == "type"){
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
    }else if(args[0] == "pwd"){
        char cwd[1024];
        if(getcwd(cwd, sizeof(cwd)) != NULL){
           cout << cwd<<"\n";
        }
    
    }else if(args[0]=="cd"){
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
                  execvp(pth.c_str(),&c_args[0]  );
                  perror("execvp");
                  exit(1);

                }else{
                  waitpid(pid, nullptr, 0);
                }

                
           }else{
              cout << args[0]<<": command not found\n";

           }

    }
  }
}
