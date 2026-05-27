# C++ Shell — Unix-Like Shell Built from Scratch

A fully featured Unix-like shell built entirely in **C++**, designed to replicate many of the core behaviors of modern Unix shells such as **Bash** and **Zsh** while providing a deeper understanding of operating system internals and terminal interaction. The project includes a custom interactive REPL, command parsing engine, process management system, job control, programmable completion, persistent history, and support for advanced shell syntax such as pipelines, redirection, quoting, and parameter expansion.

The shell was implemented completely from scratch without relying on existing shell frameworks, focusing heavily on low-level Linux/Unix concepts including process creation, file descriptor management, terminal handling, and command execution.

## Features

* Interactive REPL with a customizable shell prompt
* Execution of external programs using `fork`, `exec`, and `$PATH` resolution
* Built-in command system implemented directly inside the shell
* Persistent command history stored across sessions
* Background process execution and job tracking
* Intelligent tab completion system
* Shell variable declaration and expansion
* Support for pipelines and I/O redirection
* Robust command parsing with quote and escape handling

## Built-in Commands

Implemented several core shell built-ins directly within the interpreter:

* `exit` — terminate the shell session
* `echo` — print text and expanded variables
* `type` — identify whether a command is builtin or external
* `pwd` — display current working directory
* `cd` — navigate directories using relative or absolute paths
* `history` — display and manage command history
* `jobs` — inspect currently running background jobs
* `declare` — create and manage shell variables

## Supported Shell Functionalities

### Command Parsing & Execution

* Tokenization and parsing of user input into executable commands
* Execution of external binaries located through `$PATH`
* Support for command arguments and nested path resolution
* Proper handling of whitespace, quoting, and escaped characters

### Navigation & Environment Handling

* Relative and absolute path navigation with `cd`
* Environment variable and shell variable support
* Parameter expansion using shell-style syntax

### Quoting & Escaping

* Single quote handling for literal strings
* Double quote handling with variable expansion
* Backslash escaping for special characters and spaces

### Redirection & Pipes

* Standard output redirection using `>`
* Append output redirection using `>>`
* Standard error redirection using `2>`
* Append error redirection using `2>>`
* Multi-stage command pipelines using `|`

### Background Jobs & Job Control

* Execute commands asynchronously using `&`
* Background process tracking and monitoring
* Automatic child process reaping to prevent zombies
* Job listing and management through the `jobs` builtin

### History System

* Persistent command history saved between shell sessions
* Interactive history navigation using arrow keys
* Efficient command recall and reuse

## Advanced Features

### Intelligent Command Completion

Implemented a programmable tab-completion engine capable of dynamically generating suggestions for:

* Built-in shell commands
* External executables found in `$PATH`
* Files and directories
* Nested filesystem paths
* Context-aware completion candidates

### Programmable Completion System

* Dynamic completion generation based on command context
* Extensible architecture for adding custom completion logic
* Similar behavior to completion systems found in Bash/Zsh

### Interactive Terminal Handling

* Real-time keyboard input processing
* Arrow-key navigation for command history
* Interactive editing behavior inside the REPL
* Terminal state management for smooth user interaction

### Job Scheduling & Process Management

* Concurrent process execution
* Process lifecycle tracking
* Signal-aware child process handling
* Background job cleanup and synchronization

## Tech Stack

* **Language:** C++
* **Platform:** Unix/Linux
* **Core System APIs:** POSIX system calls (`fork`, `exec`, `pipe`, `dup2`, `waitpid`, etc.)

### Core Concepts Used

* Process creation and management
* Pipes and inter-process communication
* File descriptor manipulation
* Shell parsing and tokenization
* Terminal and TTY handling
* Job control and asynchronous execution
* Environment variable management
* REPL architecture and command interpretation

## What I Learned

Building this shell provided hands-on experience with many low-level operating system and systems programming concepts, including:

* Using `fork`, `exec`, and `waitpid` for process execution
* Implementing Unix pipelines and stream redirection
* Managing file descriptors and terminal I/O
* Designing parsers and tokenizers for shell syntax
* Handling asynchronous background jobs and process cleanup
* Building interactive terminal applications
* Implementing scalable command interpreters and REPL systems
* Understanding how real-world shells internally manage commands, jobs, and user interaction

This project served as a deep dive into Unix shell internals and significantly strengthened my understanding of systems programming, operating systems, and terminal-based application development.
