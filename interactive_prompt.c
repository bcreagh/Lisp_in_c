#include <stdio.h>

/* declare an array for the user input */
static char input[2048];

int main(int argc, char** argv) {

    /* Print version and exit info */
    puts("Lispy Version 0.0.0.1");
    puts("Press Ctrl+c to exit\n");
    
    /* In a never ending loop */
    while(1) {
        
        /* output our prompt */
        fputs("lispy> ", stdout);
        
        /* read a line of user input up to 2048 chars */
        fgets(input, 2048, stdin);
        
        /* echo input back to user */
        printf("No you're a %s", input);
    }
    
    return 0;
}