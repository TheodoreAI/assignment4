// Mateo Estrada 
// Operating Systems Oregon State University CS344
// Assignment 4
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

// size of the buffer
#define SIZE 1000 // fifty lines of 1000 characters each
#define LINES_SIZE 80 // line output are 80 characters long
// Special marker used to indicate end of the producer data

// special marker used to indicate end of the producer data
#define END_MARKER -1
// I need a bolean data type for flags:
typedef int bool;
enum
{
    false,
    true
};


// Flags for while loops with input
bool flag_stop = true;


// strcmp const

const char *stop = "STOP\n";



// Buffer 1, shared resource between input thread and line separator thread
char buffer_1[SIZE];
int count_1 = 0;
// index where input thread will put the next item
int prod_idx_1 = 0;
// index where the line separator thread will pick up the next item
int con_idx_1 = 0;
// Initialize the mutex for buffer 1
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 1
pthread_cond_t full_1 = PTHREAD_COND_INITIALIZER;





// Buffer 2, shared resource between line separator thread and the output thread (adding the plus sign thread will start HERE)

char buffer_2[SIZE];
// Number of items in the buffer
int count_2 = 0;
// Index where the line separator thread will put the next item
int prod_idx_2 = 0;
// Index where the output thread will put the next item (insert the next thread for the plus sign HERE)
int con_idx_2 = 0;
//Initialize the mutex for buffer 2
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 2
pthread_cond_t full_2 = PTHREAD_COND_INITIALIZER;


/*
    This function is used to pass the string from the fgets() to the buffer_1 from one line till it hits
    the newline character.
*/

void put_buff_1(char buf_1[], int count){
    // lock the mutual exclusion code 
    pthread_mutex_lock(&mutex_1); 

    // place the string into the buffer at the right index
    strcpy(buffer_1[prod_idx_1], buf_1);

    // increment the prod_idx
    prod_idx_1 = prod_idx_1 + 1;

    // signal to the consumer that the buffer_1 is no longer empty
    pthread_cond_signal(&full_1);

    // unlock the mutual exclusion code
    pthread_mutex_unlock(&mutex_1);
}


void print_buffer(char buffer[]){
    printf("[%s]", buffer);
}

/*

    This function is designed to read the input.
    The shell will be redirecting input from a file to stdin and 
    your program should only read from stdin.

*/

void *read_input(void *args){
    int count = 0;
    bool find_stop = false;
    const char arr_space[2] = " ";
   

    
    // Get the input to the buffer_1
    while(!find_stop){
        fgets(buffer_1, SIZE, stdin);
        
        // passing the text to the buffer
        put_buff_1(buffer_1, count);

        if(strcmp(buffer_1, "STOP\n") == 0){
            find_stop = true; // set the stopper when we reach the word STOP
        }
        count++;
    }

    // Checking to see if the buffer does have the string saved
    print_buffer(buffer_1);
    return NULL;
}


/*

    This function is designed to separate the lines of the input.
    the buffer_1 will be consumed and a new buffer will be produced.

*/


void line_separator(void *args){

    // Gotta lock the mutex_1 before checking if buffer_1 has data
    pthread_mutex_lock(&mutex_1);

    // check if buffer_1 is empty, if it is then wait for the producer to 
    while(strlen(buffer_1) == 0){
        printf("This is empty: %s\n", buffer_1);


    }



}



// using the input example from the lecture on inputting files to producer/consumer programs.

int main(int argc, char *argv[]){

    // initialize the buffers to have 0 
    memset(buffer_1, 0, SIZE);
    memset(buffer_2, 0, SIZE);
    // memset(buffer_3, 0, SIZE);
    // This is from the unbounded three threads example from the lectures on conditional variable.
    pthread_t input_t, line_t, output_t;
    // create the threads
    pthread_create(&input_t, NULL, read_input, NULL);
    // pthread_create(&line_t, NULL, line_separator, NULL);
    // pthread_create(&output_t, NULL, write_output, NULL);



    // join the threads
    pthread_join(input_t, NULL);
    // pthread_join(line_t, NULL);
    // pthread_join(output_t, NULL);
    return EXIT_SUCCESS;

}


