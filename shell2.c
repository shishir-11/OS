#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_COMMAND_LENGTH 100
#define MAX_HISTORY_SIZE 100
#define MAX_VARIABLES 100

typedef struct {
    char* name;
    char* value;
} Variable;

void printHistory(char* history[], int count) {
    printf("Command History:\n");
    for (int i = 0; i < count; i++) {
        printf("%d. %s\n", i + 1, history[i]);
    }
}

void executeSeparatedCommand(char* command, char separator) {
    char *pipedCommands[2];
    char* token = strtok(command, &separator);
    int i = 0;
    while (token != NULL) {
        pipedCommands[i] = token;
        i++;
        token = strtok(NULL, &separator);
    }
    pipedCommands[i] = NULL;

    char* args1[MAX_COMMAND_LENGTH];
    int argCount1 = 0;
    token = strtok(pipedCommands[0], " ");
    while (token != NULL) {
        args1[argCount1] = token;
        argCount1++;
        token = strtok(NULL, " ");
    }
    args1[argCount1] = NULL;

    char* args2[MAX_COMMAND_LENGTH];
    int argCount2 = 0;
    token = strtok(pipedCommands[1], " ");
    while (token != NULL) {
        args2[argCount2] = token;
        argCount2++;
        token = strtok(NULL, " ");
    }
    args2[argCount2] = NULL;

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid1 = fork();

    if (pid1 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid1 == 0) {
        // Child process 1
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        execvp(args1[0], args1);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        pid_t pid2 = fork();

        if (pid2 == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid2 == 0) {
            // Child process 2
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            execvp(args2[0], args2);
            perror("execvp");
            exit(EXIT_FAILURE);
        } else {
            // Parent process
            close(pipefd[0]);
            close(pipefd[1]);
            int status;
            waitpid(pid1, &status, 0);
            waitpid(pid2, &status, 0);
        }
    }
}

void executeCommand(char* command) {
    // Tokenize the command
    char* args[MAX_COMMAND_LENGTH];
    int argCount = 0;
    char* token = strtok(command, " ");
    while (token != NULL) {
        args[argCount] = token;
        argCount++;
        token = strtok(NULL, " ");
    }
    args[argCount] = NULL;

    // Fork a child process
    pid_t pid = fork();

    if (pid == -1) {
        // Error occurred while forking
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process
        execvp(args[0], args);
        // If execvp returns, an error occurred
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
    }
}

void handleInterrupt(int sig) {
    printf("Interrupt signal received\n");
}

int main() {
    char* history[MAX_HISTORY_SIZE];
    int historyCount = 0;
    int variableCount = 0;
    Variable variables[MAX_VARIABLES];
    signal(SIGINT, handleInterrupt);

    while (1) {
        char command[MAX_COMMAND_LENGTH];
        printf(">> ");
        fgets(command, MAX_COMMAND_LENGTH, stdin);
        fflush(stdin);

        // Remove newline character from the command
        command[strcspn(command, "\n")] = '\0';

        // Add command to history
        history[historyCount] = strdup(command);
        historyCount++;

        if (strstr(command, "=") != NULL && variableCount < MAX_VARIABLES) {
            char* token = strtok(command, "=");
            char* name = token;
            token = strtok(NULL, "=");
            char* value = token;
            variables[variableCount].name = strdup(name);
            variables[variableCount].value = strdup(value);
            variableCount++;
            continue;
        }

        if(strcmp(command, "vars") == 0) {
            for (int i = 0; i < variableCount; i++) {
                printf("%s=%s\n", variables[i].name, variables[i].value);
            }
            continue;
        }

        // Replace variables in the command
        if (strstr(command, "$") != NULL) {
            char* token = strtok(command, " ");
            while (token != NULL) {
                if (token[0] == '$') {
                    char* name = token + 1;
                    for (int i = 0; i < variableCount; i++) {
                        if (strcmp(variables[i].name, name) == 0) {
                            strcpy(token, variables[i].value);
                            break;
                        }
                    }
                }
                token = strtok(NULL, " ");
            }
        }
        // Implement interrupt signal
        if (strcmp(command, "^C") == 0) {
            raise(SIGINT);
        }

        // If history size exceeds MAX_HISTORY_SIZE, remove the oldest command
        if (historyCount > MAX_HISTORY_SIZE) {
            free(history[0]);
            for (int i = 0; i < historyCount - 1; i++) {
                history[i] = history[i + 1];
            }
            historyCount--;
        }

        // Handle history command
        if (strcmp(command, "history") == 0) {
            printHistory(history, historyCount);
        }
        // Handle cd command
        else if (strstr(command, "cd") != NULL) {
            char* token = strtok(command, " ");
            token = strtok(NULL, " ");
            if (token == NULL) {
                printf("cd: missing operand\n");
            } else {
                if(chdir(token) != 0) {
                    perror("chdir");
                }
                else {
                    printf("Directory changed to %s\n", token);
                }
            }
        }
        // Handle !n command
        else if (command[0] == '!') {
            int index = atoi(command + 1);
            if (index > 0 && index <= historyCount) {
                strcpy(command, history[index - 1]);
                printf("%s\n", command);
            } else {
                printf("Command not found\n");
            }
        }
        // Handle exit command
        else if (strcmp(command, "exit") == 0) {
            break;
        }
        // Handle other shell commands
        else {
            if (strstr(command, "|") != NULL) {
                executeSeparatedCommand(command, '|');
            }
            else 
                executeCommand(command);
        }

        // Exit the shell
        if (strcmp(command, "exit") == 0) {
            break;
        }
    }

    // Free memory allocated for history commands
    for (int i = 0; i < historyCount; i++) {
        free(history[i]);
    }

    return 0;
}
