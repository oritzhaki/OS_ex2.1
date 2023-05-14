// Or Itzhaki 209335058
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>

//helper function that compares the two files and return 1 if files are the same, 3 if they are similar and 2 otherwise
int compare_files(int fd1, int fd2){
    int ch1 = 0;
    int ch2 = 0;
    int same = 1; // flag
    int r1, r2;
    //check if each byte in the file is the same:
    while (((r1 = read(fd1, &ch1, 1)) == 1) && ((r2 = read(fd2, &ch2, 1)) == 1)) {
        if (ch1 != ch2) {
            same = 0;
            // checking similarity by ignoring spaces, end-of-lines and upper/lowercase:
            while (isspace(ch1) || ch1 == '\r' || ch1 == '\n') {
                read(fd1, &ch1, 1);
            }
            while (isspace(ch2) || ch2 == '\r' || ch2 == '\n') {
                read(fd2, &ch2, 1);
            }

            if (tolower(ch1) != tolower(ch2)) {
                return 2; // must be different
            }
        }
    }
    //check that both files have ended:
    if (r1 != r2) {
        if (r1 == 0){
            if (read(fd2, &ch2, 1) != 0) {
                same = 0;
                int similar_flag = 1;
                while (isspace(ch2) || ch2 == '\r' || ch2 == '\n') {
                    if (read(fd2, &ch2, 1) < 1){
                        similar_flag = 0;
                        break;
                    }
                }
                if (similar_flag) {
                    return 2; // must be different
                }
            }
        } else {
            same = 0;
            int similar_flag = 1;
            while (isspace(ch1) || ch1 == '\r' || ch1 == '\n') {
                if (read(fd1, &ch1, 1) < 1){
                    similar_flag = 0;
                    break;
                }
            }
            if (similar_flag) {
                return 2; // must be different
            }
        }
    }
    if (same){
        return 1;
    } else {
        return 3; //similar
    }
}


// the program should get 2 arguments, which are paths to files
int main(int argc, char *argv[]) {
    int fd1, fd2;
    char *file1, *file2;
    // check if there are enough arguments
    if (argc != 3) {
        return -1;
    }
    //get the file paths:
    file1 = argv[1];
    file2 = argv[2];
    // open the files:
    fd1 = open(file1, O_RDONLY);
    if (fd1 == -1) {
        return -1;
    }
    fd2 = open(file2, O_RDONLY);
    if (fd2 == -1) {
        close(fd1);
        return -1;
    }
    int result = compare_files(fd1, fd2);
    close(fd1);
    close(fd2);
    return result;
}
