//Or Itzhaki 209335058

//paths are relative!! didnt delete absolute check and absolute change
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/resource.h>

int compile_user_program(DIR* main_dir, char *c_file, int errors_fd, int results_fd) {
    int pid = fork();
    if (pid == -1) {
        close(errors_fd);
        close(results_fd);
        //closedir(main_dir);
        if (write(1, "Error in: fork\n", 15) == -1){
            return -1;
        }
        return -1; // move on to other user
    } else if (pid == 0) { //child
        // redirection of errors:
        if (dup2(errors_fd, STDERR_FILENO) == -1) {
            close(errors_fd);
            close(results_fd);
//             closedir(main_dir);
            if (write(1, "Error in: dup2\n", 15) == -1){
                return -1;
            }
            return -1;
        }

        // Execute the gcc command with the given arguments
        char *argv[] = {"gcc", c_file, "-o", "tempuser.out", NULL};
        if (execvp(argv[0], argv) == -1) { // gcc failed so no out file was created, not compilation error
            close(errors_fd);
            close(results_fd);
//             closedir(main_dir);
            if (write(1, "Error in: execvp\n", 17) == -1){
                return -1;
            }
            return -1;
        }
    } else { //parent
        int status;
        if (wait(&status) == -1){
            close(errors_fd);
            close(results_fd);
//             closedir(main_dir);
            if (write(1, "Error in: wait\n", 15) == -1){
                return -1;; // move on to other user
            }
            return -1;; // move to the next user, child process ended with error
        }
        int i = WEXITSTATUS(status);
        return i;
    }
    return -3;
}

int run_user_program(char *input_path, int errors_fd, int results_fd, DIR* main_dir) {
    // Prepare output file
    int output_fd = open("output.txt", O_CREAT | O_WRONLY, 0644);
    if (output_fd == -1) {
        close(errors_fd);
        close(results_fd);
//         closedir(main_dir);
        if (write(1, "Error in: open\n", 15) == -1) {
            return -1;
        }
        return -1; // Move on to the next user
    }

    // Prepare input file
    int input_fd = open(input_path, O_RDONLY);
    if (input_fd == -1) {
        close(errors_fd);
        close(results_fd);
//         closedir(main_dir);
        close(output_fd);
        if (write(1, "Error in: open\n", 15) == -1) {
            return -1;
        }
        return -1;
    }

    struct rusage usage;
    struct timeval start, end;

    // now run the c program using child process
    int pid = fork();
    if (pid == -1) {
        close(errors_fd);
        close(results_fd);
//         closedir(main_dir);
        close(output_fd);
        close(input_fd);
        if (write(1, "Error in: fork\n", 15) == -1) {
            return -1;
        }
        return -1;
    } else if (pid == 0) { //child
        //redirection of inputs:
        if (dup2(input_fd, STDIN_FILENO) == -1) {
            close(errors_fd);
            close(results_fd);
//             closedir(main_dir);
            close(output_fd);
            close(input_fd);
            if (write(1, "Error in: dup2\n", 15) == -1) {
                return -1;
            }
            return -1;
        }

        //redirection of outputs:
        if (dup2(output_fd, STDOUT_FILENO) == -1) {
            close(errors_fd);
            close(results_fd);
//             closedir(main_dir);
            close(output_fd);
            close(input_fd);
            if (write(1, "Error in: dup2\n", 15) == -1) {
                return -1;
            }
            return -1;
        }

        // redirection of errors:
        if (dup2(errors_fd, STDERR_FILENO) == -1) {
            close(errors_fd);
            close(results_fd);
//             closedir(main_dir);
            close(output_fd);
            close(input_fd);
            if (write(1, "Error in: dup2\n", 15) == -1) {
                return -1;
            }
            return -1;
        }

        // Run the c program
        char *argv[] = {"./tempuser.out", NULL};
        if (execvp(argv[0], argv) == -1) { // gcc failed
            close(errors_fd);
            close(results_fd);
//             closedir(main_dir);
            close(output_fd);
            close(input_fd);
            if (write(1, "Error in: execvp\n", 17) == -1) {
                return -1;
            }
            return -1;
        }
    } else { //parent
        int status;

        gettimeofday(&start, NULL); // to check timeout
        while (1) {
            pid_t result = waitpid(pid, &status, WNOHANG);
            if (result == pid) {
                // Child process has exited
                gettimeofday(&end, NULL);
                getrusage(RUSAGE_CHILDREN, &usage);
                //close the input and output fd's and restore stdin and stdout original fd's
                close(output_fd);
                close(input_fd);
                return 1;
            } else if (result == 0) {
                // Child process is still running
                gettimeofday(&end, NULL);
                if (end.tv_sec - start.tv_sec > 5) {
                    // Child process has been running for more than 5
                    close(output_fd);
                    close(input_fd);
                    return -2;
                }
            } else { // check if child process ended with error
                close(errors_fd);
                close(results_fd);
//                 closedir(main_dir);
                close(output_fd);
                close(input_fd);
                if (write(1, "Error in: wait\n", 15) == -1) {
                    return -1;
                }
                return -1;
            }
        }
    }
    return -3;
}

int check_user_output(int errors_fd, int results_fd, char *correct_output_path, DIR* main_dir) {
    int pid = fork();
    if (pid == -1) {
        close(errors_fd);
        close(results_fd);
//         closedir(main_dir);
        if (write(1, "Error in: fork\n", 15) == -1) {
            return -1;
        }
        return -1;
    } else if (pid == 0) { //child
        // redirection of errors:
        if (dup2(errors_fd, STDERR_FILENO) == -1) {
            close(errors_fd);
            close(results_fd);
//             closedir(main_dir);
            if (write(1, "Error in: dup2\n", 15) == -1){
                return -1;
            }
            return -1;
        }

        //Run comp.out to compare the output
        char *argv[] = {"./comp.out", "output.txt", correct_output_path, NULL};
        if (execvp(argv[0], argv) == -1) {
            close(errors_fd);
            close(results_fd);
//             closedir(main_dir);
            if (write(1, "Error in: execvp\n", 17) == -1) {
                return -1;
            }
            return -1;
        }
    } else { //parent
        int status;
        if ((wait(&status)) == -1) {
            close(errors_fd);
            close(results_fd);
//             closedir(main_dir);
            if (write(1, "Error in: wait\n", 15) == -1) {
                return -1; // move on to other user
            }
            return -1;
        }
        int i = WEXITSTATUS(status);
        return i;
    }
    return -3;
}

int is_absolute_path(const char *path) {
    if (path[0] == '/') {
        return 1; // Path is absolute
    } else {
        return 0; // Path is relative
    }
}

//make absolute paths
void get_full_path(char destination[200], char path[200]) {
    int len = strlen(destination), lastPlace = len, i;
    for (i = 0; i < len; i++) {
        if (destination[i] == '/')
            lastPlace = i;
    }
    destination[lastPlace] = '\0'; //now is the path to destination, just need to add relative path:
    strcat(destination, "/");
    strcat(destination, path);
}

int main(int argc, char *argv[]) {
    // Check number of arguments
    if (argc < 2) {
        if (write(1, "Not valid argv\n", 15) == -1){ //error message????????????????
            return -1;
        }
        return -1;
    }

    // Open configuration file (can assume that the file exists and is not a folder)
    int conf_fd = open(argv[1], O_RDONLY);
    if (conf_fd == -1) {
        if (write(1, "Error in: open\n", 15) == -1){
            return -1;
        }
        return -1;
    }

    // Read paths from configuration file
    char folder_path[200], input_path[200], correct_output_path[200];
    int row = 0;
    ssize_t nread;
    char buffer[256];
    char* buf_ptr = buffer;
    while (row < 3 && (nread = read(conf_fd, buf_ptr, 1)) > 0) {
        if (nread == -1) {
            if (write(1, "Error in: read\n", 15) == -1){
                close(conf_fd);
                return -1;
            }
            close(conf_fd);
            return -1;
        }
        if (*buf_ptr == '\n') {
            *buf_ptr = '\0';  // Replace newline character with null terminator
            switch (row) {
                case 0:
                    strncpy(folder_path, buffer, 200);
                    break;
                case 1:
                    strncpy(input_path, buffer, 200);
                    break;
                case 2:
                    strncpy(correct_output_path, buffer, 200);
                    break;
                default:
                    break;
            }
            buf_ptr = buffer;
            row++;
        } else {
            buf_ptr++;
        }
    }
    close(conf_fd);

    // Check and fix paths
   char temp_path[200];
   if (!is_absolute_path(folder_path)){
       strcpy(temp_path, argv[1]);
       get_full_path(temp_path, folder_path);
       strcpy(folder_path, temp_path);
   }
   if (!is_absolute_path(input_path)){
       strcpy(temp_path, argv[1]);
       get_full_path(temp_path, input_path);
       strcpy(input_path, temp_path);
   }
   if (!is_absolute_path(correct_output_path)){
       strcpy(temp_path, argv[1]);
       get_full_path(temp_path, correct_output_path);
       strcpy(correct_output_path, temp_path);
   }

    // Check if folder exists for first path
    struct stat folder_stat;
    if (stat(folder_path, &folder_stat) == -1) {
        if (write(1, "Error in: stat\n", 15) == -1){
            return -1;
        }
        return -1;
    }

    //Check if path is folder
    if (!S_ISDIR(folder_stat.st_mode)) {
        if (write(1, "Not a valid directory\n", 22) == -1){
            return -1;
        }
        return -1;
    }

    // Check that output and input files exists
    if (access(correct_output_path, F_OK) == -1) {
        if (write(1, "Output file not exist\n", 22) == -1){
            return -1;
        }
        return -1;
    }
    if (access(input_path, F_OK) == -1) {
        if (write(1, "Input file not exist\n", 21) == -1){
            return -1;
        }
        return -1;
    }

    //Prepare error file
    int errors_fd = open("errors.txt", O_CREAT | O_WRONLY, 0644);
    if (errors_fd == -1) {
        if (write(1, "Error in: open\n", 15) == -1){
            return -1;
        }
        return -1;
    }

    // Open results file
    int results_fd = open("results.csv", O_CREAT | O_WRONLY, 0644);
    if (results_fd == -1) {
        if (write(1, "Error in: open\n", 15) == -1){
            close(errors_fd);
            return -1;
        }
        close(errors_fd);
        return -1;
    }

    // Go through every entry in the main folder and explore folders only
    DIR *main_dir = opendir(folder_path);
    if (!main_dir) {
        if (write(1, "Error in: opendir\n", 18) == -1){
            close(errors_fd);
            close(results_fd);
            return -1;
        }
        close(results_fd);
        close(errors_fd);
        return -1;
    }

    // Go over all user folders in this folder
    struct dirent *entry;
    while ((entry = readdir(main_dir)) != NULL) { //until end of main directory
        
        // Ignore hidden files and folders, current and parent dirs
        if (entry->d_name[0] == '.' || !strcmp(entry->d_name, "..")) {
            continue;
        }
        
        write(1, entry->d_name, strlen(entry->d_name));
        write(1, "22222222222222\n", 15);

        // Generate full path for folder
        char full_entry_path[200];
        memset(full_entry_path, 0, sizeof(full_entry_path));
        strcpy(full_entry_path, folder_path);
        strcat(full_entry_path, "/");
        strcat(full_entry_path, entry->d_name);

        // Check if the entry is a directory
        // note: if the path doesnt exist in the system, S_ISDIR may still confirm as a folder, therefore the use of stat here.
        if (stat(full_entry_path, &folder_stat) == -1) {
            close(results_fd);
            close(errors_fd);
//             closedir(main_dir);
            if (write(1, "Error in: stat\n", 15) == -1){
                return -1;
            }
            /// return or exit or continue?
        }
        if(!S_ISDIR(folder_stat.st_mode)){
            continue;
        }

        // Open the user folder
        DIR *user_dir = opendir(full_entry_path);
        if (user_dir == NULL) {
            close(results_fd);
            close(errors_fd);
//             closedir(main_dir);
            if (write(1, "Error in: opendir\n", 18) == -1){
                return -1;
            }
            /// return or exit or continue?
        }

        // Check if the directory found contains a C file
        int c_flag = 1;
        char c_file[300];
        struct dirent *user_entry;
        while ((user_entry = readdir(user_dir)) != NULL) {
            write(1, "in loop\n", 9);
            write(1, user_entry->d_name, strlen(user_entry->d_name));
            write(1, "\n", 1);
            if (user_entry->d_type == DT_REG) { // check if the entry is a regular file
                const char* file_name = user_entry->d_name;
                const size_t name_len = strlen(file_name);
                if (name_len >= 2 && strcmp(file_name + name_len - 2, ".c") == 0) {
                    memset(c_file, 0, sizeof(c_file));
                    strcpy(c_file, full_entry_path);
                    strcat(c_file, "/");
                    strcat(c_file,file_name);
                    c_flag = 0;
                    break; // c file found
                }
            }
        }
        if (c_flag){//no c file, move on to other user
            write(1, "in if\n", 6);
            char full_result[150];
            memset(full_result, 0, sizeof(full_result));
            strcpy(full_result, entry->d_name);
            strcat(full_result, ",0,NO_C_FILE\n");
            if (write(results_fd, full_result, strlen(full_result)) == -1) {
                close(results_fd);
                close(errors_fd);
//                 closedir(main_dir);
                closedir(user_dir);
                return -1;
            }
            continue;
        }

        //close user folder
        closedir(user_dir);

        // Compile the user program
        int outcome = compile_user_program(main_dir, c_file, errors_fd, results_fd);
        if (outcome) {
            char full_result[150];
            memset(full_result, 0, sizeof(full_result));
            strcpy(full_result, entry->d_name);
            strcat(full_result, ",10,COMPILATION_ERROR\n");
            if (write(results_fd, full_result, strlen(full_result)) == -1) {
                close(results_fd);
                close(errors_fd);
//                 closedir(main_dir);
                return -1;
            }
            continue;
        }

        // Run user program
        outcome = run_user_program(input_path, errors_fd, results_fd, main_dir);
        if (outcome == -2) {
            char full_result[150];
            memset(full_result, 0, sizeof(full_result));
            strcpy(full_result, entry->d_name);
            strcat(full_result, ",20,TIMEOUT\n");
            if (write(results_fd, full_result, strlen(full_result)) == -1) {
                close(results_fd);
                close(errors_fd);
//                 closedir(main_dir);
                return -1;
            }
            continue;
        }

        // Run the compare program to check the users output
        outcome = check_user_output(errors_fd, results_fd, correct_output_path, main_dir);

        //add to result the correct score
        char full_result[150];
        memset(full_result, 0, sizeof(full_result));
        strcpy(full_result, entry->d_name);
        switch (outcome) {
            case 1:
                strcat(full_result, ",100,EXCELLENT\n");
                if (write(results_fd, full_result, strlen(full_result)) == -1) {
                    close(results_fd);
                    close(errors_fd);
//                     closedir(main_dir);
                    return -1;
                }
                write(1, "in 100!\n", 8);
                break;
            case 2:
                strcat(full_result, ",50,WRONG\n");
                if (write(results_fd, full_result, strlen(full_result)) == -1) {
                    close(results_fd);
                    close(errors_fd);
//                     closedir(main_dir);
                    return -1;
                }
                break;
            case 3:
                strcat(full_result, ",75,SIMILAR\n");
                if (write(results_fd, full_result, strlen(full_result)) == -1) {
                    close(results_fd);
                    close(errors_fd);
//                     closedir(main_dir);
                    return -1;
                }
                break;
            default:
//                close(results_fd);
//                close(errors_fd);
//                closedir(main_dir);
                break;
        }

        write(1, "after\n", 6);
        
        // delete output file
        if (remove("output.txt") == -1) {
            close(results_fd);
            close(errors_fd);
//             closedir(main_dir);
            if ((write(2, "Error in: remove\n", 17)) == -1)
                return -1;
            return -1;
        }
        write(1, "end\n", 4);
    }
    //delete out file
    if (remove("tempuser.out") == -1) {
        if ((write(2, "Error in: remove\n", 17)) == -1)
            return -1;
        return -1;
    }
    close(errors_fd);
    close(results_fd);
}
