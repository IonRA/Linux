#include <iostream>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>
#include <cstring>

enum Metadata{
    NB_SPEAKERS,
    LAST_PID,
    NB_LINES_OF_DIALOGUE
};

const int THIS_PID = getpid();
const int PAGE_SIZE = sysconf(_SC_PAGE_SIZE);
const int TEXT_AREA_SIZE = 1 * PAGE_SIZE;
const int SESSION_START = PAGE_SIZE;
const int SESSION_END = SESSION_START + TEXT_AREA_SIZE;
bool LEFT_SESSION = false;

void finalizeOnError(
        char* error_message,
        int file_descriptor = -1,
        char* session_file = nullptr,
        int session_file_size = 0
        );
bool isSessionFile(struct stat file_status);
bool initializeSession(bool has_created_session, char* session_file, int session_file_descriptor);
void* listenTalkingSession(void* args);
bool startSpeaking(char* session_file, int session_file_descriptor);
bool speak (
        int session_file_descriptor,
        int offset,
        char* line,
        int size_of_line,
        char* session_file
        );
bool lockAndWrite(
        int session_file_descriptor,
        int offset,
        char* line,
        int size_of_line,
        char* session_file
        );
bool readWhenUnlocked(
        int session_file_descriptor,
        char* session_file,
        int offset,
        int size_of_line,
        char* destination
        );
bool checkIfSessionEnded(
        int session_file_descriptor,
        char* session_file,
        bool* isTheSessionOver
        );

bool leaveSession(
        int session_file_descriptor,
        char* session_file,
        char* session_path
        );
bool closeSession(
        char* session_path
        );
bool updateMetadata(
        int session_file_descriptor,
        char* session_file,
        Metadata metadata,
        int value
        );

int main(int argc, char** argv) {

    if (errno) {
        finalizeOnError("Couldn't get the page size of the system.\n");
    }

    //you need to pass the session file address as arg from cmd line in order to join in the talking session
//    if (argc != 2) {
//        finalizeOnError(
//                "You can't join the talking session!\nErr: Checkout the path of the file session!\n"
//                );
//    }

    char* session_path =  "/home/ion/CLionProjects/talking_session/assets/session.txt";//argv[1];

    int session_file_descriptor = open(session_path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

    if (session_file_descriptor == -1) {
        finalizeOnError("Couldn't open file.\n");
    }

    struct stat session_status;

    if (fstat(session_file_descriptor, &session_status) == -1) {
        finalizeOnError(
                "Couldn't get file size.\n",
                session_file_descriptor
                );
    }

    bool has_created_session = false;

    if (!isSessionFile(session_status)) {
        if (ftruncate(session_file_descriptor, SESSION_END) == -1) {
            finalizeOnError(
                    "Couldn't extend the file.\n",
                    session_file_descriptor
            );
        }

        if (fstat(session_file_descriptor, &session_status) == -1) {
            finalizeOnError(
                    "Couldn't get file size.\n",
                    session_file_descriptor
            );
        }

        has_created_session = true;
    }

    char* session_file = (char*)mmap(
            NULL,
            session_status.st_size,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            session_file_descriptor,
            0);

    if (session_file == MAP_FAILED) {
        finalizeOnError(
                "Couldn't map session file.\n",
                session_file_descriptor
                );
    }

    //initialize the file for the talking session
    initializeSession(has_created_session, session_file, session_file_descriptor);

    pthread_t listening_thread_id;
    void* listen_args[2];
    listen_args[0] = session_file;
    listen_args[1] = &session_file_descriptor;

    if (pthread_create(&listening_thread_id, NULL, listenTalkingSession, listen_args)) {
        finalizeOnError(
                "Couldn't create session listening thread!\n",
                session_file_descriptor,
                session_file,
                session_status.st_size
                );
    }

    bool session_ended_well = startSpeaking(session_file, session_file_descriptor) &&
                              leaveSession(session_file_descriptor, session_file, session_path);

    LEFT_SESSION = true;

    if (pthread_join(listening_thread_id, nullptr)) {
        finalizeOnError(
                "Couldn't create session listening thread!\n",
                session_file_descriptor,
                session_file,
                session_status.st_size
        );
    }

    if (!session_ended_well) {
        finalizeOnError(
                "An error occurred in talking session!\n",
                session_file_descriptor,
                session_file,
                session_status.st_size
        );
    }

    if (munmap(session_file, session_status.st_size) == -1) {
        perror("Couldn't unmap the file.\n");
    }

    if (close(session_file_descriptor) == -1) {
        perror("Couldn't close the file.\n");
    }

    if (errno) {
        return 1;
    }

    return 0;
}

void finalizeOnError(
        char* error_message,
        int file_descriptor,
        char* session_file,
        int session_file_size
) {
    perror(error_message);

    if (session_file) {
        if (munmap(session_file, session_file_size) == -1) {
            perror("Couldn't unmap the file.\n");
        }
    }

    if (file_descriptor != -1) {
        if (close(file_descriptor) == -1) {
            perror("Couldn't close the file.\n");
        }
    }

    exit(1);
}

bool isSessionFile(struct stat file_status) {
    if (file_status.st_size < SESSION_END) {
        return false;
    }
    return true;
}

bool initializeSession(bool has_created_session, char* session_file, int session_file_descriptor) {
    int nb_speakers;

    if (has_created_session) {
        nb_speakers = 1;

        //initialize the metadata of the session
        int byte_increment = 0;
        for (
                char* str_nb_speakers = (char*)&nb_speakers;
                byte_increment < sizeof(int);
                ++byte_increment
                ) {
            session_file[byte_increment] = str_nb_speakers[byte_increment];
            session_file[sizeof(int) + byte_increment] &= 0;
            session_file[2 * sizeof(int) + byte_increment] &= 0;
        }
    } else if (!updateMetadata( //update the metadata of the session
            session_file_descriptor,
            session_file,
            NB_SPEAKERS,
            1
            )) {
        return false;
    }

    return true;
}

void* listenTalkingSession(void* listen_args) {
    char* session_file = ((char**)listen_args)[0];
    int session_file_descriptor = *((int**)listen_args)[1];
    int last_process_PID;
    int tracked_lines_of_dialogue = 0;
    int nb_lines_of_dialogue;
    char* new_line_of_dialogue = session_file + SESSION_START;
    struct flock other_file_lock;

    do {
        memset (&other_file_lock, 0, sizeof(struct flock));

        if (fcntl(session_file_descriptor, F_GETLK, &other_file_lock) == -1) {
            break;
        }

        if (other_file_lock.l_type == F_UNLCK) {
            last_process_PID = *((int*)(session_file + sizeof(int)));
            nb_lines_of_dialogue = *((int*)(session_file + 2 * sizeof(int)));
            if (tracked_lines_of_dialogue < nb_lines_of_dialogue) {
                tracked_lines_of_dialogue = nb_lines_of_dialogue;

                if (last_process_PID != THIS_PID) {
                    printf("%s", new_line_of_dialogue);
                }
            }
        }

        sleep(2);
    } while (!LEFT_SESSION);

    return nullptr;
}

bool startSpeaking(char* session_file, int session_file_descriptor) {
    char* line_of_dialogue = nullptr;
    ssize_t size_of_line;
    size_t alloc_mem = 0;

    while (
            (size_of_line = getline(&line_of_dialogue, &alloc_mem, stdin)) != -1 && //check if input call is successful
            size_of_line > 1 //check if it's been entered anything
            ) {

        if (!speak(
                session_file_descriptor,
                SESSION_START,
                line_of_dialogue,
                size_of_line + 1,
                session_file
                )) {
            free(line_of_dialogue);
            return false;
        }

        free(line_of_dialogue);
        line_of_dialogue = nullptr;
        alloc_mem = 0;
    }

    free(line_of_dialogue);

    return size_of_line != -1;
}

bool speak (
        int session_file_descriptor,
        int offset,
        char* line,
        int size_of_line,
        char* session_file
        ) {
    if (!lockAndWrite(
            session_file_descriptor,
            offset,
            line,
            size_of_line,
            session_file
        )) {
        return false;
    }

    if (!updateMetadata(session_file_descriptor, session_file, NB_LINES_OF_DIALOGUE, 1)) {
        return false;
    }

    return updateMetadata(session_file_descriptor, session_file, LAST_PID, THIS_PID);
}

bool lockAndWrite(
        int session_file_descriptor,
        int offset,
        char* line,
        int size_of_line,
        char* session_file
        ) {
    struct flock this_file_lock, other_file_lock;

    memset (&this_file_lock, 0, sizeof(struct flock));

    this_file_lock.l_whence = SEEK_SET; /* beginning of file */
    this_file_lock.l_start  = 0;        /* offset from l_whence */
    this_file_lock.l_len    = 0;        /* length, 0 = to EOF */
    this_file_lock.l_pid    = THIS_PID; /* PID */
    this_file_lock.l_type   = F_WRLCK;  //prepare the lock for write locking

    do {
        memset (&other_file_lock, 0, sizeof(struct flock));

        if (fcntl(session_file_descriptor, F_GETLK, &other_file_lock) == -1) {
            return false;
        }
    } while (other_file_lock.l_type != F_UNLCK);

    //set write lock on file
    if (fcntl(session_file_descriptor, F_SETLKW, &this_file_lock) == -1) {
        return false;
    }

    //write message in file
    for (int byte_increment = 0; byte_increment < size_of_line; ++byte_increment) {
        session_file[offset + byte_increment] = line[byte_increment];
    }

    //unlock the file
    this_file_lock.l_type = F_UNLCK;
    if (fcntl(session_file_descriptor, F_SETLK, &this_file_lock) == -1) {
        return false;
    }

    return true;
}

bool readWhenUnlocked(
        int session_file_descriptor,
        char* session_file,
        int offset,
        int size_of_line,
        char* destination_buffer
        ) {
    struct flock other_file_lock;

    //wait until file is unlocked
    do {
        memset (&other_file_lock, 0, sizeof(struct flock));

        if (fcntl(session_file_descriptor, F_GETLK, &other_file_lock) == -1) {
            return false;
        }
    } while (other_file_lock.l_type != F_UNLCK);

    //read from mapped file into buffer
    for (int byte_increment = 0; byte_increment < size_of_line; ++byte_increment) {
        destination_buffer[byte_increment] = session_file[offset + byte_increment];
    }

    return true;
}

bool checkIfSessionEnded(
        int session_file_descriptor,
        char* session_file,
        bool* isTheSessionOver
        ) {
    int nb_speakers;

    if (!readWhenUnlocked(
            session_file_descriptor,
            session_file,
            0,
            sizeof(int),
            (char*)&nb_speakers
    )) {
        return false;
    }

    if (nb_speakers == 1) {
        *isTheSessionOver = true;
    } else {
        *isTheSessionOver = false;
    }

    return true;
}

bool updateMetadata(
        int session_file_descriptor,
        char* session_file,
        Metadata metadata,
        int value
        ) {
    switch (metadata) {
        case NB_SPEAKERS:
            int nb_speakers;

            if (!readWhenUnlocked(
                    session_file_descriptor,
                    session_file,
                    0,
                    sizeof(int),
                    (char*)&nb_speakers
            )) {
                return false;
            }

            nb_speakers += value;

            if (!lockAndWrite(
                    session_file_descriptor,
                    0,
                    (char*)&nb_speakers,
                    sizeof(int),
                    session_file)) {
                return false;
            }
            break;
        case LAST_PID:
            if (!lockAndWrite(
                    session_file_descriptor,
                    sizeof(int),
                    (char*)&value,
                    sizeof(int),
                    session_file)) {
                return false;
            }
            break;
        case NB_LINES_OF_DIALOGUE:
            int nb_lines_of_dialogue;

            if (!readWhenUnlocked(
                    session_file_descriptor,
                    session_file,
                    2 * sizeof(int),
                    sizeof(int),
                    (char*)&nb_lines_of_dialogue
            )) {
                return false;
            }

            nb_lines_of_dialogue += value;

            if (!lockAndWrite(
                    session_file_descriptor,
                    2 * sizeof(int),
                    (char*)&nb_lines_of_dialogue,
                    sizeof(int),
                    session_file)) {
                return false;
            }
            break;
    }
    return true;
}

bool leaveSession(
        int session_file_descriptor,
        char* session_file,
        char* session_path
        ) {
    bool isTheSessionOver;

    //store in isTheSessionOver true if session ended and false if it is not ended
    if (!checkIfSessionEnded(
            session_file_descriptor,
            session_file,
            &isTheSessionOver
    )) {
        return false;
    }

    if (isTheSessionOver) {
        return closeSession(session_path);
    } else {
        //setup message to notify when leaving the session
        char leave_message[60];
        sprintf(
                leave_message,
                "The process with PID %d has left the session!\n",
                THIS_PID
        );
        int size_of_line = strlen(leave_message);

        if (!speak(
                session_file_descriptor,
                SESSION_START,
                leave_message,
                size_of_line + 1,
                session_file
        )) {
            return false;
        }

        //return call to updateMetadata since it is the last action to check if it is successful
        return updateMetadata(
                session_file_descriptor,
                session_file,
                NB_SPEAKERS,
                -1
        );
    }
}

bool closeSession(
        char* session_path
        ) {
    if (unlink(session_path) == -1) {
        return false;
    }
    return true;
}
