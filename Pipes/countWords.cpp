#include <iostream>
#include <unistd.h>
#include <wait.h>

void finalizeOnError(
        char* error_message,
        int* pipe = nullptr,
        int pipe_end = -1
);

int countWords(int fd);

int main(int argc, char** argv) {

    if (argc < 2) {
        finalizeOnError("There is no program to generate words to output stream!\n");
    }

    char** cmdv = argv;

    int parent_read_pipe[2];

    if (pipe(parent_read_pipe) < 0) {
        finalizeOnError("Failed to create a pipe!\n");
    }

    switch (fork()) {
        case -1:
            finalizeOnError("Could not create child process!\n", parent_read_pipe);
            break;
        case 0: //child process
            if (close(parent_read_pipe[0]) < 0) {
                finalizeOnError("Could not close pipe's read end\n", parent_read_pipe, 1);
            }

            if (dup2(parent_read_pipe[1], STDOUT_FILENO) < 0) {
                finalizeOnError("Could not duplicate pipe's write end\n", parent_read_pipe, 1);
            }

            if (close(parent_read_pipe[1]) < 0) {
                finalizeOnError("Could not close pipe's write end\n");
            }

            if (execvp(cmdv[1], cmdv + 1) < 0) {
                finalizeOnError("Could not run given command in child process!\n");
            }
            break;
        default: //parent process
            if (close(parent_read_pipe[1]) < 0) {
                finalizeOnError("Could not close pipe's write end\n", parent_read_pipe, 0);
            }

            wait(nullptr);
            int word_count = countWords(parent_read_pipe[0]);

            if (word_count < 0) {
                finalizeOnError("Could not read from the pipe\n", parent_read_pipe, 0);
            }

            if (close(parent_read_pipe[0]) < 0) {
                finalizeOnError("Could not close pipe's read end\n");
            }

            printf("The given command printed %d words\n", word_count);
    }
    return 0;
}

void finalizeOnError(
        char* error_message,
        int* pipe,
        int pipe_end
) {
    perror(error_message);

    if (pipe) {
        switch (pipe_end) {
            case 0:
                if (close(pipe[0]) < 0) {
                    perror("Could not close pipe's read end\n");
                }
                break;
            case 1:
                if (close(pipe[1]) < 0) {
                    perror("Could not close pipe's write end\n");
                }
                break;
            default:
                if (close(pipe[0]) < 0) {
                    perror("Could not close pipe's read end\n");
                }
                if (close(pipe[1]) < 0) {
                    perror("Could not close pipe's write end\n");
                }
        }
    }

    exit(1);
}

int countWords(int fd) {
    char current_letter, previous_letter = ' ';
    int word_count = 0;
    int read_file_successfully;

    while ((read_file_successfully = read(fd, &current_letter, 1)) > 0) {
        if (!isspace(current_letter) && isspace(previous_letter)) {
            ++word_count;
        }

        previous_letter = current_letter;
    }

    if (read_file_successfully < 0) {
        return -1;
    }

    return word_count;
}
