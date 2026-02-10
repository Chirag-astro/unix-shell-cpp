#include <iostream>
#include <string>
#include <unordered_set>
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
    if(command == "exit"){
      break;
    }else if( command.length() >= 5 && command.substr(0,4) == "echo" && command[4] == ' '){
      cout << command.substr(5) <<"\n";
    }else if( command.length() >= 5 && command.substr(0,4) == "type" && command[4] == ' '){
       string arg = command.substr(5);
       if(builtin_commands.find(arg)!= builtin_commands.end()){
          cout << arg << " is a shell builtin\n";
       }else{
          cout << arg<<": not found\n";
       }
    }else{
      cout << command<<": command not found\n";
    }
  }
}
