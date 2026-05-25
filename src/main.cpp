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
#include <readline/history.h>
#include <dirent.h>
#include<bits/stdc++.h>

using namespace std;

struct job{
  int jobid;
  pid_t pid;
  string command;
  string status;

};


string is_exec(string cmd)
{

  string path = getenv("PATH");
  vector<string> directories;
  string tmp;
  string pth = "";

  for (auto c : path)
  {
    if (c == ':')
    {
      directories.push_back(tmp);
      tmp.clear();
    }
    else
    {
      tmp.push_back(c);
    }
  }
  directories.push_back(tmp);

  bool chk = false;

  for (auto &d : directories)
  {
    string cur_pth = d;
    cur_pth.push_back('/');
    cur_pth += cmd;
    if (access(cur_pth.c_str(), X_OK) == 0)
    {
      return cur_pth;
    }
  }

  return pth;
}

vector<string> pipe_tokenizer(string &command)
{
  vector<string> v;
  string tmp;
  for (auto c : command)
  {
    if (c == '|')
    {
      v.push_back(tmp);
      tmp.clear();
    }
    else
    {
      tmp.push_back(c);
    }
  }
  v.push_back(tmp);
  return v;
}

vector<string> tokenize(string &command)
{
  vector<string> v;
  string tmp;

  bool squotes = false;
  bool dquotes = false;

  for (int i = 0; i < command.size(); i++)
  {
    if (command[i] != '\'' && squotes)
    {
      tmp.push_back(command[i]);
    }
    else if (command[i] != '"' && dquotes)
    {
      if (i + 1 < command.size() && command[i] == '\\' && (command[i + 1] == '\\' || command[i + 1] == '"'))
      {
        tmp.push_back(command[i + 1]);
        i++;
        continue;
      }
      tmp.push_back(command[i]);
    }
    else if (command[i] == '\'' && !squotes)
    {
      squotes = true;
    }
    else if (command[i] == '"' && !dquotes)
    {
      dquotes = true;
    }
    else if (command[i] == '\'' && squotes)
    {
      squotes = false;
    }
    else if (command[i] == '"' && dquotes)
    {
      dquotes = false;
    }
    else if (command[i] == ' ')
    {
      if (!tmp.empty())
      {
        v.push_back(tmp);
        tmp.clear();
      }
    }
    else if (command[i] == '\\')
    {
      if (i + 1 < command.size())
      {
        tmp.push_back(command[i + 1]);
        i++;
      }
    }
    else
    {
      tmp.push_back(command[i]);
    }
  }

  if (!tmp.empty())
    v.push_back(tmp);

  return v;
}

string parse_redirection(vector<string> &args)
{

  if (args.size() >= 2 && (args[args.size() - 2] == ">" || args[args.size() - 2] == "1>"))
  {
    string f = args.back();
    args.pop_back();
    args.pop_back();

    return f;
  }

  return "";
}

string parse_append(vector<string> &args)
{

  if (args.size() >= 2 && (args[args.size() - 2] == ">>" || args[args.size() - 2] == "1>>"))
  {
    string f = args.back();
    args.pop_back();
    args.pop_back();

    return f;
  }

  return "";
}

string parse_err_redirection(vector<string> &args)
{

  if (args.size() >= 2 && args[args.size() - 2] == "2>")
  {
    string f = args.back();
    args.pop_back();
    args.pop_back();

    return f;
  }

  return "";
}

string parse_error_append(vector<string> &args)
{

  if (args.size() >= 2 && args[args.size() - 2] == "2>>")
  {
    string f = args.back();
    args.pop_back();
    args.pop_back();

    return f;
  }

  return "";
}

void apply_redirection(string ofname, string efname)
{
  if (!ofname.empty())
  {
    int fd = open(ofname.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    dup2(fd, 1);
    close(fd);
  }

  if (!efname.empty())
  {
    int fd = open(efname.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    dup2(fd, 2);
    close(fd);
  }
}

void restore_redirection(int o_saved, int e_saved, int i_saved)
{
  dup2(o_saved, 1);
  dup2(e_saved, 2);
  dup2(i_saved, 0);
  close(o_saved);
  close(e_saved);
  close(i_saved);
}

void restore_op_redirection(int o_saved)
{
  dup2(o_saved, 1);
  close(o_saved);
}

void restore_err_redirection(int e_saved)
{
  dup2(e_saved, 2);
  close(e_saved);
}

void restore_pipe_opr(int o_saved)
{
  dup2(o_saved, 1);
  close(o_saved);
}

void restore_pipe_ipr(int i_saved)
{
  dup2(i_saved, 0);
  close(i_saved);
}

void apply_append_redirection(string oa_name, string ea_name)
{
  if (!oa_name.empty())
  {
    int fd = open(oa_name.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
    dup2(fd, 1);
    close(fd);
  }

  if (!ea_name.empty())
  {
    int fd = open(ea_name.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
    dup2(fd, 2);
    close(fd);
  }
}

void apply_pipe_redirection(int fd1)
{

  dup2(fd1, 1);
  close(fd1);
}

void apply_pipe_input(int fd0)
{
  dup2(fd0, 0);
  close(fd0);
}

unordered_set<string> builtin_list = {
    "echo",
    "type",
    "exit",
    "pwd",
    "cd",
    "history"};

unordered_set<string> executables;

char *command_generator(const char *text, int state)
{
  static int index;
  static vector<string> matches;

  if (state == 0)
  {
    index = 0;
    matches.clear();

    string prefix(text);

    for (auto &cmd : builtin_list)
    {
      if (cmd.substr(0, prefix.size()) == prefix)
      {
        matches.push_back(cmd);
      }
    }

    for (auto &cmd : executables)
    {
      if (cmd.substr(0, prefix.size()) == prefix)
      {
        matches.push_back(cmd);
      }
    }
  }

  if (index < matches.size())
  {
    return strdup(matches[index++].c_str());
  }

  return nullptr;
}

void find_all_files( string pref, vector<string>&matches){


   string filen;
   int idx = -1;
   for (int i = pref.size()-1; i >= 0; i--)
   {
      if(pref[i]=='/'){
           idx = i ;
           break;
        }
      filen.push_back(pref[i]);
   }
   reverse(filen.begin(), filen.end());
   string dir;
   if(idx!=-1){
   
    for (int i = 0; i < idx; i++)
    {
       dir.push_back(pref[i]);
    }
    
   }
   

  struct dirent *entry;
  DIR *dp;

  if(!dir.empty()){
    dp = opendir(dir.c_str());
  }else{
    dp = opendir(".");
  }

  if(dp == NULL){return;}

  while((entry = readdir(dp)) != NULL){

    string file = entry->d_name;
    if(file == "." || file == "..")
        continue;
     
        if(file.rfind(filen,0)== 0){
              if(entry->d_type == DT_DIR){
                  if(!dir.empty()){
                      file = dir + '/' + file + '/';
                  }else{
                    file = file+'/';
                  }
                  
              }else{
                if(!dir.empty()){
              file = dir + '/' + file;
                }
            }
          matches.push_back( file);}
    

  }
  closedir(dp);

}

char *filename_generator(const char *text, int state){

  static int index;
  static vector<string> matches;  

  if(state==0){
    index = 0;
    matches.clear();
    string prefix(text);
    find_all_files(prefix, matches);
    if(matches.size()==1){
          if(matches[0].back() == '/'){
        rl_completion_append_character = '\0';
    }else{
        rl_completion_append_character = ' ';
    }
    }

  }

  if(index < matches.size()){
    return strdup(matches[index++].c_str());
  }
  return nullptr;
}

char **command_completion(const char *text, int start, int end)
{

  rl_attempted_completion_over = 1;
  if(start > 0){
    return rl_completion_matches(text, filename_generator);
  }

  return rl_completion_matches(text, command_generator);
}

void find_all_executables()
{
  string path = getenv("PATH");
  vector<string> dir;
  string tmp;

  for (auto c : path)
  {
    if (c == ':')
    {
      dir.push_back(tmp);
      tmp.clear();
    }
    else
    {
      tmp.push_back(c);
    }
  }
  dir.push_back(tmp);

  for (auto c : dir)
  {
    struct dirent *entry;
    string pth = c;
    DIR *dp = opendir(c.c_str());
    if (dp == NULL)
    {
      continue;
    }

    while ((entry = readdir(dp)) != NULL)
    {
      string cmd = entry->d_name;
      string x = pth + '/' + cmd;
      if (access(x.c_str(), X_OK) == 0)
      {
        executables.insert(cmd);
      }
    }
    closedir(dp);
  }
}




vector<string>hist;
unordered_map<string,int>last_written;

void write_history(string fname){
  int fd = open(fname.c_str(), O_WRONLY| O_CREAT | O_TRUNC, 0644);

  for (int i = 0; i < hist.size(); i++)
  {
     string s = hist[i]+"\n";
     write(fd, s.c_str(), s.size());
  }
  close(fd);

}

void append_history(string fname){
  int fd = open(fname.c_str(), O_WRONLY| O_CREAT | O_APPEND, 0644);

  for (int i = last_written[fname]; i < hist.size(); i++)
  {
     string s = hist[i]+"\n";
     write(fd, s.c_str(), s.size());
  }
  close(fd);

}

void read_history(string fname){
  int fd = open(fname.c_str(), O_RDONLY);
  char c;
  string s;
  while(read(fd, &c, 1) > 0){
    if(c=='\n'){
      hist.push_back(s);
      s.clear();
    }else{
      s.push_back(c);
    }
  }
  if(!s.empty())hist.push_back(s);
  close(fd);
}

void load_history(){
  char* hf = getenv("HISTFILE");
  if(hf==nullptr)return;
  string path(hf);
  int fd = open(path.c_str(),O_RDONLY);

  char c; 
  string s;
  while(read(fd,&c,1) > 0){
    if(c=='\n'){
      hist.push_back(s);
      s.clear();
    }else{
      s.push_back(c);
    }
  }
  if(!s.empty())hist.push_back(s);
  close(fd);
}

void write_history(){
  char* hf = getenv("HISTFILE");
  if(hf==nullptr)return;
  string path(hf);
  int fd = open(path.c_str(),O_WRONLY |O_CREAT | O_APPEND, 0644);

  for(int i  = last_written[path] ; i < hist.size();i++){
     string s = hist[i] + "\n";
     write(fd,s.c_str(), s.size());
  }

  close(fd);
}

map<int,job>job_lists;
priority_queue<int, vector<int>, greater<int>>pq;


void updated_jobs(){

    vector<int>remove_jobs;

    int jbid = job_lists.size();

    for(auto j  : job_lists){

      int status;
      pid_t ret = waitpid(j.second.pid, &status, WNOHANG);

      string s = "Running";
      string cmd= j.second.command;

      if(ret == j.second.pid){
          remove_jobs.push_back(j.first);
          s  = "Done";
          cmd.pop_back();cmd.pop_back();
          cout << "[" << j.first << "]";
          if(jbid==1){
              cout <<"+ ";
          }else if(jbid==2){
              cout <<"- ";
          }else{
              cout <<"  ";
          }
          cout << s<<"                 ";
          cout << cmd<<"\n";
      }
      jbid--;

    }

    for(auto c : remove_jobs){pq.push(c);job_lists.erase(c);}  

}

int job_id = 1;

// vector<job>job_lists;


int main()
{
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  find_all_executables();
  rl_attempted_completion_function = command_completion;
  load_history();
  char* hf = getenv("HISTFILE");
  string hist_path;
  if(hf!=nullptr){
    hist_path = hf;
  }
  last_written[hist_path] = hist.size();


  // TODO: Uncomment the code below to pass the first stage
  string og_command;
  unordered_set<string> builtin_commands = {"echo", "type", "exit", "pwd", "cd", "history", "jobs", "complete"};

  while (true)
  {

    updated_jobs();

    char *input = readline("$ ");
    if(input && *input){
    add_history(input);
}
    og_command = string(input);
    free(input);
    if (og_command.empty())
      continue;
    
    hist.push_back(og_command);  
    vector<string> pipe_tokenzied = pipe_tokenizer(og_command);

    vector<pid_t> pids;
    int prev_rd = -1;
    int o_saved = dup(1);
    int e_saved = dup(2);
    int i_saved = dup(0);

    for (int i = 0; i < pipe_tokenzied.size(); i++)
    {

      string command = pipe_tokenzied[i];
      vector<string> args = tokenize(command);
      int fd[2];
      string ofname = parse_redirection(args);
      string efname = parse_err_redirection(args);
      string oa_name = parse_append(args);
      string ea_name = parse_error_append(args);



      if (i != pipe_tokenzied.size() - 1)
      {
        pipe(fd);
      }

      if (builtin_commands.count(args[0]))
      {
        if (i != 0) apply_pipe_input(prev_rd); 
        if (i != pipe_tokenzied.size() - 1) apply_pipe_redirection(fd[1]);        
        apply_redirection(ofname, efname);
        apply_append_redirection(oa_name, ea_name);

        if (args[0] == "exit")
        {
          write_history();
          
          exit(0);
        }
        else if (args[0] == "echo")
        {
          for (int i = 1; i < args.size(); i++)
          {
            cout << args[i];
            if (i < args.size() - 1) {
                cout << " ";
            }
          }
          cout << "\n";
        }
        else if (args[0] == "type")
        {

          if (builtin_commands.find(args[1]) != builtin_commands.end())
          {
            cout << args[1] << " is a shell builtin\n";
          }
          else
          {
            string pth = is_exec(args[1]);
            if (!pth.empty())
            {
              cout << args[1] << " is " << pth << "\n";
            }
            else
            {
              cout << args[1] << ": not found\n";
            }
          }
        }
        else if (args[0] == "pwd")
        {

          char cwd[1024];
          if (getcwd(cwd, sizeof(cwd)) != NULL)
          {
            cout << cwd << "\n";
          }
        }
        else if (args[0] == "cd")
        {

          if (args[1] == "~")
          {
            string home = getenv("HOME");
            int op = chdir(home.c_str());
            if (op == -1)
            {
              cout << "cd: " << args[1] << ": No such file or directory\n";
            }
          }
          else
          {
            const char *dir = args[1].c_str();
            int op = chdir(dir);
            if (op == -1)
            {
              cout << "cd: " << args[1] << ": No such file or directory\n";
            }
          }
        }else if(args[0] == "history"){

          if(args.size()> 2 && args[1] == "-r"){
              read_history(args[2]);
          }else if(args.size() > 2 && args[1] == "-w"){
              write_history(args[2]);
          }else if(args.size() > 2 && args[1] == "-a"){
              append_history(args[2]);
            last_written[args[2]] = hist.size();

          }else{

          int limit = hist.size();

          if(args.size()==2 &&  args.back()[0] >= '1' && args.back()[0] <= '9' )
              limit = stoi(args.back());

          for (int i = hist.size()-limit; i < hist.size(); i++)
          {
             cout <<  i+1 <<"  "<< hist[i]<<"\n";
          }
        }
          
        }else if( args[0] == "jobs"){

          vector<int>remove_jobs;

          int jbid = job_lists.size();

          for(auto j  : job_lists){

              int status;
              pid_t ret = waitpid(j.second.pid, &status, WNOHANG);

              string s = "Running";
              string cmd= j.second.command;

              if(ret == j.second.pid){
                remove_jobs.push_back(j.first);
                s  = "Done";
                cmd.pop_back();cmd.pop_back();
                
              }

              cout << "[" << j.first << "]";
              if(jbid==1){
                cout <<"+ ";
              }else if(jbid==2){
                cout <<"- ";
              }else{
                cout <<"  ";
              }
              cout << s<<"                 ";
              cout << cmd<<"\n";
              jbid--;

          }

          for(auto c : remove_jobs){pq.push(c);job_lists.erase(c);}  

        }
        dup2(o_saved, 1);
        dup2(e_saved, 2);
        dup2(i_saved, 0);

        if (i != pipe_tokenzied.size() - 1) {
            prev_rd = fd[0];
        }
      }else{


        bool bg_job = args.back() == "&";
        if(bg_job)args.pop_back();
        string pth = is_exec(args[0]);
        vector<char *> c_args;
        for (auto &c : args)
        {
          char *strng = (char *)c.c_str();
          c_args.push_back(strng);
        }
        c_args.push_back(nullptr);

        if (!pth.empty())
        {
          pid_t pid = fork();

          if (pid == 0)
          {
            if (i != 0) apply_pipe_input(prev_rd);
            if (i != pipe_tokenzied.size() - 1) {
                  close(fd[0]);
                  apply_pipe_redirection(fd[1]);
            }
            apply_redirection(ofname, efname);
            apply_append_redirection(oa_name, ea_name);
            execvp(pth.c_str(), &c_args[0]);
            perror("execvp");
            exit(1);
          }
          else
          {
            if(!bg_job)
            pids.push_back(pid);
            else
             {
              int jid;
              if(!pq.empty()){
                jid = pq.top();
                pq.pop();
              }else{
                jid = job_id;
                job_id++;
                
              }
              job j = {jid, pid, og_command,"Running"};
              job_lists[jid] = j; 
              // job_lists.push_back(j);
               cout << "["<< jid <<"] "<< pid<<"\n";
             }
              
            if (i != 0) close(prev_rd); 
            
            if (i != pipe_tokenzied.size() - 1) {
                prev_rd = fd[0]; 
                close(fd[1]); 
            }            
          }
        }
        else
        {
        apply_redirection(ofname, efname);
        apply_append_redirection(oa_name, ea_name);
          cout << args[0] << ": command not found\n";
          restore_err_redirection(e_saved);
          restore_op_redirection(o_saved);
        }

      }
  }

      for (auto &pid : pids)
    {
      waitpid(pid, nullptr, 0);
    }
    restore_redirection(o_saved, e_saved, i_saved);
}

}