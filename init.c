// init: The initial user-level program

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

char *argv[] = { "sh", 0 };

int main(void) {
    int pid, wpid;

    if(open("console", O_RDWR) < 0){
        mknod("console", 1, 1);
        open("console", O_RDWR);
    }
    dup(0);  // stdout
    dup(0);  // stderr

    // Login authentication
    int attempts = 0;
    int logged_in = 0;
    char username[100];
    char password[100];

    while (attempts < 3 && !logged_in) {
        printf(1, "Enter Username: ");
        memset(username, 0, sizeof(username));
        gets(username, sizeof(username));
        int user_len = strlen(username);
        if (user_len > 0 && username[user_len-1] == '\n') 
            username[user_len-1] = 0;

        if (strcmp(username, USERNAME) != 0) {
            attempts++;
            printf(1, "Invalid username. Attempts left: %d\n", 3 - attempts);
            continue;
        }

        printf(1, "Enter Password: ");
        memset(password, 0, sizeof(password));
        gets(password, sizeof(password));
        int pass_len = strlen(password);
        if (pass_len > 0 && password[pass_len-1] == '\n') 
            password[pass_len-1] = 0;

        if (strcmp(password, PASSWORD) != 0) {
            attempts++;
            printf(1, "Invalid password. Attempts left: %d\n", 3 - attempts);
        } else {
            logged_in = 1;
            printf(1,"Login successful\n");
        }
    }

    if (!logged_in) {
        printf(1, "Maximum login attempts reached. Login disabled.\n");
        while(1) {
            sleep(100);  // Avoid busy-waiting
        }
        // exit();
    }

    // Proceed to start the shell
    for(;;) {
        printf(1, "init: starting sh\n");
        pid = fork();
        

        if (pid < 0) {
            printf(1, "init: fork failed\n");
            exit();
        }
        if (pid == 0) {
            exec("sh", argv);
            printf(1, "init: exec sh failed\n");
            exit();
        }
        while((wpid=wait()) >= 0 && wpid != pid)
            printf(1, "zombie!\n");
    }
}