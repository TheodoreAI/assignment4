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
char buffer_1[SIZE];
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





// Buffer 2, shared resource between line separator thread and plus sign thread 
char buffer_2[SIZE];
// Number of items in the buffer
int count_2 = 0;
// Index where the line separator thread will put the next item
int prod_idx_2 = 0;
// Index where the plus sign thread will put the next item 
int con_idx_2 = 0;
//Initialize the mutex for buffer 2
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 2
pthread_cond_t full_2 = PTHREAD_COND_INITIALIZER;

// buffer 3, shared resource between line plus sign thread and output thread
char buffer_3[SIZE];
// Number of lines in the buffer
int count_3 = 0;
// index where the plus sign thread will put the next item
int prod_idx_3 = 0;
// index where the output thread will put the next item.
int con_idx_3 = 0;
 // Initialize the mutex for buffer_3
 pthread_mutex_t mutex_3 = PTHREAD_MUTEX_INITIALIZER;
 // Initialize the condition variable for buffer 3;
 pthread_cond_t full_3 = PTHREAD_COND_INITIALIZER;

// prints array of chars
void print_buffer_array(char buffer[]){
    // for(int i = 0; i < 4; i++){
    //     printf("[%s]\n", buffer);
    // }
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

void put_buff_1(char buf_1[]){
    // lock the mutual exclusion code 
    pthread_mutex_lock(&mutex_1); 

    // concatctenate the string into the buffer_2
    strncat(buffer_1, buf_1, SIZE);
    count_1++;
    
    // increment the prod_idx
    // prod_idx_1 = prod_idx_1 + 1;

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
    // for some strange reason the count_1 can't be used to read the number of times the
    // newlines are reached in the input text. I am using count to keep track of
    // how many times I iterate before hitting a "STOP\n".
    // There shouldn't be a risk of losing count because it gets restarted every time read_input() gets called
    // and it only gets called once.
    int count = 0; 
    char input[SIZE];

    // Get the input to the buffer_1
    while(!flag_stop){
        
        // get the input from the file

        fgets(input, SIZE, stdin);
        
        if(strcmp(input, stop) == 0){
            flag_stop = true; // set the stopper when we reach the word STOP
        }else{
             // passing the text to the buffer unless it hits STOP
            put_buff_1(input);
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

    strncat(line_array, buffer_1, SIZE);
    // Increment the index from which the item will be picked up
    // con_idx_1 = con_idx_1 + 1;
    // decrement the number of items in the buffer_1
    count_1--; // this will stop the program if there is no more inputs.
    // unlock the mutex
    pthread_mutex_unlock(&mutex_1);

    //return the line
    // print_buffer_array(line_array);

    return line_array;
}

void put_buff_2(char arr[]){
    // print_buffer_array(arr); 
    // Lock the mutex before putting the string into the buffer
    pthread_mutex_lock(&mutex_2);
    // Put the string in the buffer_2
    strncat(buffer_2, arr, SIZE);
    // Increment the index where the next string will be put.
    // prod_idx_2 = prod_idx_2 + 1;
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
    // set the array pointer so that I can free the malloc() from the get_buf_1() to avoid memory leaks.
    char *line_array;
   
    // I need to call the get_buff_1() count_1 times since that's how many lines of string
    // are in buffer_1
    
        line_array = get_buff_1();
        // print_buffer_array(line_array);
      
        // Looping through the strings in the arr[] to find
        // places with newline characters and replace them with spaces.
        for (int i = 0; i<SIZE; i++){
            if (line_array[i] == '\n'){
                line_array[i] = ' ';
                
            }
        }
        // Now that the newlines have been removed, I must add the string to the second buffer_2
        // print_buffer_array(line_array);
        put_buff_2(line_array);
    
    free(line_array);
    return NULL;
}


char *get_buff_2(){
    char *plus_arr;
    plus_arr = malloc(SIZE+1);
    // Lock the mutual exclusion before checking if the buffer_2 has data.
    pthread_mutex_lock(&mutex_2);
    while(count_2 == 0)
        // buffer_2 is empty. wait for the producer to signal that the buffer has data
        pthread_cond_wait(&full_2, &mutex_2);
    
    strcpy(plus_arr, buffer_2);
    // increment the index from which the item will be picked up
    // con_idx_2 = con_idx_2 + 1;
    // decrement the number of items in the buffer_2
    count_2--;
    // unlock the mutex
    pthread_mutex_unlock(&mutex_2);
    // return the line
    return plus_arr;
}

void put_buff_3(char input3[]){

    // Lock the mutex before putting the string into the buffer
    pthread_mutex_lock(&mutex_3);
    strncat(buffer_3, input3, SIZE);

    // increase the number of lines in buffer_3
    count_3++;
    // signal condition that buffer_3 is full
    pthread_cond_signal(&full_3);
    // unlock the mutex_3
    pthread_mutex_unlock(&mutex_3);

}


void *plus_sign(void *args){
    // set the array pointer so that I can free the malloc() from the get_buf_2() to avoid memory leaks.
    char *plus_arr;
    char input3[SIZE];

    
        plus_arr = get_buff_2();
        
        
        // Looping through the string in the plus_array to find 
        // places with the ++ and repace them with ^ (have to be next to each other).
        int i, j;
        for (i = 0; i<= SIZE; i++, j++){
            if(plus_arr[i] == '+' && plus_arr[i+1] == '+'){
                input3[j] = '^';
                i++;
            }else{
                input3[j] = plus_arr[i];
            }
        }
        // print_buffer_array(input3);
        put_buff_3(input3);

    free(plus_arr);
    return NULL;
}


char *get_buff_3(){
    char *output_arr;
    output_arr = malloc(SIZE+1);
    // Lock the mutual exclusion before checking if the buffer_3 has data.
    pthread_mutex_lock(&mutex_3);
    while(count_3 == 0)
        // buffer_3 is empty, wait for the produced to signal that the buffer_3 has data.
        pthread_cond_wait(&full_3, &mutex_3);
    strcpy(output_arr, buffer_3);
    // decrememnt the number of items in the buffer_3
    count_3--;
    // unlock the mutex
    pthread_mutex_unlock(&mutex_3);
    // print_buffer_array(output_arr);
    return output_arr;
}


/*
    Function that the output thread will run.
    Consume the string from the buffer shared with the plus sign thread
    stdout string.
*/


void *write_output(void *args){
    // I have to take care of freeing malloc() so I have to delcare the output_arr again here and free() at end.
    char *output_arr;
    char arr_eighty[80];
    char input4[SIZE];
    int idx = 0;

    // I might need to make another put_buff_3() 
   
    output_arr = get_buff_3(); // get the buffer_3
    // for (int i = 0; i < SIZE; i++){
    //     fprintf(stdout, output_arr[i]);
    // }
    // strncpy(input4, output_arr, SIZE);
    // loop the length of the output_arr
    for(int i = 0; i <= strlen(output_arr); i++, idx++){
        // save the characters on the input4 array and then once we hit 79 (gotta allow for the \0)
        // output it
        input4[idx] = output_arr[i];
        if (idx == 79){
            //  fprintf(stdout, "%.80s %d\n", input4, strlen(output_arr));
             fprintf(stdout, "%.80s\n", input4);
             idx = -1;
             // there is a bug for output3.txt file writting
        }
    }
    
    free(output_arr);
    return NULL;

}
// using the input example from the lecture on inputting files to producer/consumer programs.
int main(int argc, char *argv[]){
    // This is from the unbounded three threads example from the lectures on conditional variable.
    pthread_t input_t, line_t, plus_sign_t, output_t;
    // create the threads
    pthread_create(&input_t, NULL, read_input, NULL);
    pthread_create(&line_t, NULL, line_separator, NULL);
    pthread_create(&plus_sign_t, NULL, plus_sign, NULL);
    pthread_create(&output_t, NULL, write_output, NULL);


    // join the threads
    pthread_join(input_t, NULL);
    pthread_join(line_t, NULL);
    pthread_join(plus_sign_t, NULL);
    pthread_join(output_t, NULL);
    return EXIT_SUCCESS;
}


