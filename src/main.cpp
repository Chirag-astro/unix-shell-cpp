#include <iostream>
#include <string>
using namespace std;

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  // TODO: Uncomment the code below to pass the first stage
  string command;

  while(true){
    std::cout << "$ ";
    
    getline(cin, command);
    if(command == "exit"){
      break;
    }else if( command.length() >= 5 && command.substr(0,4) == "echo" && command[4] == ' '){
      cout << command.substr(5) <<"\n";
    }else{
      cout << command<<": command not found\n";
    }
  }
}
