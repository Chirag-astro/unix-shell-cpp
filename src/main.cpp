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
      cur_pth += cmd;
      if(access(cur_pth.c_str(), X_OK) == 0){
          return cur_pth;
      }
    }

    return pth;

}

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  // TODO: Uncomment the code below to pass the first stage
  string command;
  unordered_set<string>builtin_commands = {"echo", "type", "exit"};  

  while(true){
    std::cout << "$ ";
    
    getline(cin, command);
    if(command.empty())continue;
    string cmd;
    vector<string>args;
    stringstream ss(command);
    
    string token;

    while (ss >> token)
    {
      args.push_back(token);
    }
    
    
    // ss >> cmd >> arg;
    if(args[0] == "exit"){
      break;
    }else if( args[0] == "echo"){
      cout << command.substr(5) <<"\n";
    }else if(cmd == "type"){
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
