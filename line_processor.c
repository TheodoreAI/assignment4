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
#define INPUT_LINES_SIZE 50 // line output are 80 characters long
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
bool flag_stop = false;
bool flag_separator = false;


// strcmp const

const char *stop = "STOP\n";



// Buffer 1, shared resource between input thread and line separator thread
char buffer_1[INPUT_LINES_SIZE][SIZE];
// Number of lines in the buffer
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

char buffer_2[INPUT_LINES_SIZE][SIZE];
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


// prints array of chars
void print_buffer_array(char buffer[]){
  
    printf("[%s]\n", buffer);
    
}

// prints chars
void print_buffer_char(char buffer){
    printf("[%c]", buffer);
}

// print number of lines in a buffer

void print_num_lines(int counts){
    printf("Number of lines in buffer: [%d]\n", counts);
}

/*
    This function is used to pass the string from the fgets() to the buffer_1 from one line till it hits
    the newline character.
*/

void put_buff_1(char buf_1[], int count){
    // lock the mutual exclusion code 
    pthread_mutex_lock(&mutex_1); 
    // printf("[%s]", buf_1);

    // place the string into the buffer at the right index
    strcpy(buffer_1[prod_idx_1], buf_1);
    count_1++;
    
    // increment the prod_idx
    prod_idx_1 = prod_idx_1 + 1;

    // signal to the consumer that the buffer_1 is no longer empty
    pthread_cond_signal(&full_1);

    // unlock the mutual exclusion code
    pthread_mutex_unlock(&mutex_1);
}

/*

    This function is designed to read the input.
    The shell will be redirecting input from a file to stdin and 
    your program should only read from stdin.

*/

void *read_input(void *args){
    int count = 0;
   
    // Get the input to the buffer_1
    while(!flag_stop){

        // get the input from the file
        fgets(buffer_1[count], SIZE, stdin);
        if(strcmp(buffer_1[count], stop) == 0){
            flag_stop = true; // set the stopper when we reach the word STOP
        }else{
             // passing the text to the buffer unless it hits STOP
            put_buff_1(buffer_1[count], count);
            count++;
        }
       
    }
    return NULL;
}


/*
    This function is used to get the buffer_1 line for the line separator thread.
    This code is from the sample file: https://replit.com/@cs344/64prodconsunboundsolc#main.c
*/
char *get_buff_1(){
    char *line_array;
    line_array = malloc(SIZE + 1);
    // Lock the mutual exlusion before checking if the buffer_1 has data
    pthread_mutex_lock(&mutex_1);
    while(count_1 == 0)

        // buffer is empty. wait for the producer to signal that the buffer has data
        pthread_cond_wait(&full_1, &mutex_1);
    
    // print_buffer_array(buffer_1[con_idx_1]);

    strcpy(line_array, buffer_1[con_idx_1]);
    // Increment the index from which the item will be picked up
    con_idx_1 = con_idx_1 + 1;
    // decrement the number of items in the buffer_1
    count_1--; // this will stop the program if there is no more inputs.
    // unlock the mutex
    pthread_mutex_unlock(&mutex_1);

    //return the line
    // print_buffer_array(line_array);

    return line_array;
}

void put_buff_2(char arr[]){

    // Lock the mutex before putting the string into the buffer
    pthread_mutex_lock(&mutex_2);
    // Put the string in the buffer_2
    strcpy(buffer_2[prod_idx_2], arr);
    // Increment the index where the next string will be put.
    prod_idx_2 = prod_idx_2 + 1;
    count_2++; // increase the number of lines in buffer_2 by 1
    //signal to the consumer that buffer_2 is no longer empty
    pthread_cond_signal(&full_2);
    // unlock the mutex
    pthread_mutex_unlock(&mutex_2); 
}

/*

    This function is designed to separate the lines of the input.
    the buffer_1 will be consumed and a new buffer will be produced.

*/

void *line_separator(void *args){
    int count = 0;
    // set the array pointer so that I can free the malloc() from the get_buf_1() to avoid memory leaks.
    char *line_array;
    char arr[SIZE*INPUT_LINES_SIZE];
    print_num_lines(count_1);
    // I need to call the get_buff_1() count_1 times since that's how many lines of string
    // are in buffer_1
    while(count_1 > 0){
        line_array = get_buff_1();
        strncat(arr, line_array, SIZE);
        // Looping through the strings in the arr[] to find
        // places with newline characters and replace them with spaces.
        for (int i = 0; i<SIZE; i++){
            if (arr[i] == '\n'){
                arr[i] = ' ';
                // print_buffer_array(arr);
            }
        }

        // Now that the newlines have been removed, I must add the string to the second buffer_2
        put_buff_2(arr);
    }
    
    free(line_array);
return NULL;
}



// using the input example from the lecture on inputting files to producer/consumer programs.
int main(int argc, char *argv[]){
    // This is from the unbounded three threads example from the lectures on conditional variable.
    pthread_t input_t, line_t, output_t;
    // create the threads
    pthread_create(&input_t, NULL, read_input, NULL);
    pthread_create(&line_t, NULL, line_separator, NULL);
    // pthread_create(&output_t, NULL, write_output, NULL);



    // join the threads
    pthread_join(input_t, NULL);
    pthread_join(line_t, NULL);
    // pthread_join(output_t, NULL);
    return EXIT_SUCCESS;

}


