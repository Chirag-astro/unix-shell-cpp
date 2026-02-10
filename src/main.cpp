#include <iostream>
#include <string>
#include <sstream>
#include <unordered_set>
#include <unistd.h>
#include <vector>
#include <cstdlib>
using namespace std;

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
    string cmd, arg;
    stringstream ss(command);
    ss >> cmd >> arg;
    if(cmd == "exit"){
      break;
    }else if( cmd == "echo"){
      cout << command.substr(5) <<"\n";
    }else if(cmd == "type"){
       if(builtin_commands.find(arg)!= builtin_commands.end()){
          cout << arg << " is a shell builtin\n";
       }else{
          string path  = getenv("PATH");
          vector<string>dir;
          string tmp;
          for(auto c : path){
            if(c ==':'){
              dir.push_back(tmp);
              tmp.clear();
            }else{
              tmp.push_back(c);
            }
          }
          dir.push_back(tmp);
          bool chk=false;
          for(auto &d : dir){
            string cur_path = d;
            cur_path.push_back('/');
            cur_path += arg;
            if(access(cur_path.c_str(), X_OK)==0){
              chk = true;
              cout << arg <<" is "<< cur_path<<"\n";
              break;
            }
          }
          if(!chk)
          cout << arg<<": not found\n";
       }
    }else{
      cout << cmd<<": command not found\n";
    }
  }
}
