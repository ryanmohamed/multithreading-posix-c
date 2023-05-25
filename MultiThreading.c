#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <unistd.h> // for read sys call
#include <string.h>
#include <pthread.h>
#include <math.h>

#define NUM_THREADS 14
#define NUM_UTILITY_THREADS 3

/* GLOBAL THREAD DATA */
int InpArray[14000];
long double Roots_Of_Sums[NUM_THREADS];   /* threads will write to global arrays, main thread will then read */
long double Geometric_Averages[NUM_THREADS];
long double Arithmetic_Averages[NUM_THREADS];

// global file descriptor incase threads should mutate the output file
int outputFile;

// each of the 14 thread children of the main thread will run this function
void * child_runner(void * param); 

// each of the 14 thread children, will themselves have 3 threads who run these functions
void * root_of_square_sums_runner(void * param);
void * geometric_average_runner(void * param);
void * arithmetic_averages_runner(void * param);

// utility functions 
long double min(long double arr[], int length);
void write_to_standard_out(char * str);
void write_to_file(int fd, char * str);
void write_long_double_to_file(int fd, long double num);
void write_int_to_file(int fd, int num);

int main(int argc, char *argv[]){

/* BEGIN: ARGUMENT VALIDATION --------------------------------------------------------------------------- */
    /* check if input matches EXE INPUT_FILE OUTPUT_FILE */
    if(argc != 3) {
        printf("CMD argument syntax is: ./MYLASTNAME_MYSTUDENTID.exe INPUT_FILE OUTPUT_FILE\n");
        return EXIT_FAILURE;
    }

    char * infile = argv[1];
    char * outfile = argv[2];
    printf("Input file: %s\n", infile);
    printf("Output file: %s\n", outfile);
/* END: ARGUMENT VALIDATION --------------------------------------------------------------------------- */



/* BEGIN: READ INPUT FILE --------------------------------------------------------------------------- */

    /* main program thread reads input file 
    into 14k element int arr */
    int fd;

    //an important number since we want to read ONE integer at a time
    //if we use sizeof int, we'll read 4 bytes, which may include
    //more data than necessary when a line contains 3 bytes (2 numbers and 1 newline)
    //read 1 byte at a time
    int BUFFER_SZ = 1;
    int readBuffer[BUFFER_SZ]; //read buffer contains 1 byte of information 
    int n; // data read from file
    
    // is the file valid to read from
    if((fd = open(infile, O_RDONLY, 0444)) == -1){
        printf("Failed to open input file!\n");
        return 2;
    }

    int num = 0; //track current number
    
    //digit string is initally empty
    char * digit = malloc((size_t) 0);
    int digit_len = 0;  

    while((n = read(fd, readBuffer, BUFFER_SZ)) > 0){
        
        // if not a newline add to concatenated digit
        if(readBuffer[0] != '\n'){

            digit_len = (size_t) ((int) digit_len + 1); //track the extra byte
            digit = realloc(digit, digit_len * sizeof(char)); //reallocate the extra room
            digit[digit_len-1] = readBuffer[0]; //assign latest digit

        }

        // if a newline, convert the digit and add to array
        else {
            
            int converted_digit = atoi((const char *) digit);
            InpArray[num] = converted_digit;
            num += 1; //track numbers read
            
            digit_len = 0;  //clear the concatenated string for next data
            digit = realloc(digit, (size_t) 0);
        }
    }

    //now that we're done reading from file, free memory from malloc
    free(digit);

/* END: READ INPUT FILE --------------------------------------------------------------------------- */



    // //debug info
    // for(int i = 0; i < 14000; i++){
    //     printf("#%d:\t", i+1);
    //     printf("%d\n", InpArray[i]);
    // }



/* BEGIN: CREATE & OPEN OUTPUT FILE --------------------------------------------------------------------------- */
    if((outputFile = creat(outfile, 0666)) == -1){ //create a file for writing only, check if it fails
        printf("\nMain program thread: Could not create output file!\n");
        return EXIT_FAILURE;
    } // *recall create behaves as if we called open with the create flag
/* END: CREATE OUTPUT FILE --------------------------------------------------------------------------- */



/* BEGIN: CREATE THREADS --------------------------------------------------------------------------- */

    pthread_t workers[NUM_THREADS];
    pthread_attr_t worker_attrs[NUM_THREADS];

    for(int i = 0; i < NUM_THREADS; i++){
        //initialize default attributes of current thread
        pthread_attr_init(&worker_attrs[i]);

        //its easy to assume that i gets passed to the runner function
        //each time we create a thread
        //however this would be passing the SAME memory to each thread
        //which is something we don't want
        //instead we need to allocate new memory to pass over

        int * arg = malloc(sizeof(*arg));
        if(arg == NULL){
            printf("Could not allocate memory to pass to thread!\n");
            return EXIT_FAILURE;
        }

        *arg = i; //dereference and assign value of i

        //create thread, pass allocated memory to runner of the thread
        pthread_create(&workers[i], &worker_attrs[i], child_runner, arg);
    }

    //wait for all threads to join before the main thread continues execution
    //we don't include this call in the above loop
    //because we'd be waiting for a thread to terminate before
    //making a new one
    //we want to harness the power of multithread but letting
    //the threads run concurrently
    for(int i = 0; i < NUM_THREADS; i++){
        pthread_join(workers[i], NULL);
    }
    
/* END: CREATE THREADS --------------------------------------------------------------------------- */

/* BEGIN: COMPUTE MINIMUMS --------------------------------------------------------------------------- */

    long double min_root_sq_sum = min(Roots_Of_Sums, NUM_THREADS);
    long double min_geo_avg = min(Geometric_Averages, NUM_THREADS);
    long double min_ari_avg = min(Arithmetic_Averages, NUM_THREADS);

/* END: COMPUTE MINIMUMS --------------------------------------------------------------------------- */
  

/* BEGIN: FILE OUTPUT --------------------------------------------------------------------------- */

    write_to_file(outputFile, "From Main Program Thread:\n");

    for(int i = 0; i < NUM_THREADS; i++){
        write_to_file(outputFile, "Worker Child Pthread Number = ");
        write_int_to_file(outputFile, i);

        write_to_file(outputFile, " : \t Root of the Sum of Squares = ");
        write_long_double_to_file(outputFile, Roots_Of_Sums[i]);

        write_to_file(outputFile, " : \t Geometric Average = ");
        write_long_double_to_file(outputFile, Geometric_Averages[i]);

        write_to_file(outputFile, " : \t Arithmetic Average = ");
        write_long_double_to_file(outputFile, Arithmetic_Averages[i]);

        write_to_file(outputFile, "\n");
    }

    // min root of square sums
    write_to_file(outputFile, "\n");
    write_to_file(outputFile, "Main program thread:\tMin of the Root of the Sum of Squares =\t");
    write_long_double_to_file(outputFile, min_root_sq_sum);
    
    // min root of geometric averages
    write_to_file(outputFile, "\n");
    write_to_file(outputFile, "Main program thread:\tMin of the Geometric Averages =\t");
    write_long_double_to_file(outputFile, min_geo_avg);

    // min root of arithmetic averages
    write_to_file(outputFile, "\n");
    write_to_file(outputFile, "Main program thread:\tMin of the Arithmetic Averages =\t");
    write_long_double_to_file(outputFile, min_ari_avg);

/* END: FILE OUTPUT --------------------------------------------------------------------------- */

    //finally close file
    write_to_file(outputFile, "\nMain program thread: Terminating. \n");
    close(outputFile);
    return EXIT_SUCCESS;

}




/* END: CHILD WORKER THREAD RUNNER --------------------------------------------------------------------------- */
// for our purposes the parameter passed to the child thread
// will be the pointer to the index of the child thread in relation to the
// other threads
void * child_runner(void * param){

    int index = *((int *) param); //cast to integer pointer and dereference to get integer value
    int array_offset = index * 1000; //each thread is responsible for a 1000 integer section of the array

    // each thread must create 3 threads of their own
    pthread_t grandchildren[NUM_UTILITY_THREADS];
    pthread_attr_t grandchildren_attrs[NUM_UTILITY_THREADS];

    // pointers to addresses where we will store 
    // grandchild thread return values 
    void * root_sq_ret;
    void * geo_avg_ret;
    void * ari_avg_ret;

    // create root of sq sum, geo avg, arith avg threads
    for(int i = 0; i < NUM_UTILITY_THREADS; i++){
        //initialize default attributes of current thread
        pthread_attr_init(&grandchildren_attrs[i]);
        if(i == 0)
            pthread_create(&grandchildren[i], &grandchildren_attrs[i], root_of_square_sums_runner, param);
        else if(i == 1)
            pthread_create(&grandchildren[i], &grandchildren_attrs[i], geometric_average_runner, param);
        else if(i == 2)
            pthread_create(&grandchildren[i], &grandchildren_attrs[i], arithmetic_averages_runner, param);
    }

    // let each concurrently running threads children, also run concurrently
    for(int i = 0; i < NUM_UTILITY_THREADS; i++){

        if(i == 0) 
            // recall that when a non NULL argument is used as the value_ptr in the join call
            // the value passed to pthread_exit is made
            // available in the address provided in value_ptr
            // return value is stored in the address pointed
            // to by root_square_sum
            pthread_join(grandchildren[i], &root_sq_ret);
        else if (i == 1)
            pthread_join(grandchildren[i], &geo_avg_ret);
        else if (i == 2)
            pthread_join(grandchildren[i], &ari_avg_ret);
    }

    // cast void pointer ret value to long double pointer
    // dereference to get long double VALUE
    long double root_sq_sum = *((long double *) root_sq_ret);
    long double geo_avg = *((long double *) geo_avg_ret);
    long double ari_avg = *((long double *) ari_avg_ret);

    // after the worker child thread recieves the
    // computations from ITS child, the worker child thread
    // can update the global data found in the main thread
    // known as the side effects of the thread
    Roots_Of_Sums[index] = root_sq_sum;
    Geometric_Averages[index] = geo_avg;
    Arithmetic_Averages[index] = ari_avg;

    // /* BEGIN CHILD THREAD OUTPUT --------------------- */
    // wasn't sure if this was required from the worker threads to show concurrency
    write_to_file(outputFile, "From Child Worker Thread:\n");
    write_int_to_file(outputFile, index);
    write_to_file(outputFile, " - Thread Number\n");

    write_long_double_to_file(outputFile, root_sq_sum);
    write_to_file(outputFile, " - Root of Sum of Squares\n");

    write_long_double_to_file(outputFile, geo_avg);
    write_to_file(outputFile, " - geometric average (that is, the nth root of the product of the n numbers)\n");

    write_long_double_to_file(outputFile, ari_avg);
    write_to_file(outputFile, " - arithmetic average (that is, the sum of n numbers divided by n)\n\n");
    // /* END CHILD THREAD OUTPUT --------------------- */

    pthread_exit(0); // parent process is waiting for child worker threads to join
}
/* END: CHILD WORKER THREAD RUNNER --------------------------------------------------------------------------- */




/* BEGIN: UTILITY FUNCTIONS --------------------------------------------------------------------------- */
long double min(long double arr[], int length){
    long double min = INFINITY; //start with min as infinity, will never find in array
    for(int i = 0; i < length; i++){
        if(arr[i] < min)
            min = arr[i];
    }
    return min;
}

void write_to_standard_out(char * str){
    write(STDOUT_FILENO, str, strlen(str));
}

// write to file using string and file descriptor
void write_to_file(int fd, char * str){
    if( write(fd, str, strlen(str)) != strlen(str) ){
        printf("Could not write to file!\n");
        close(fd);
    }
}

void write_long_double_to_file(int fd, long double num){
    char str[40];
    sprintf(str, "%.3LF", num); // create a string consisting of the formated long double

    if( write(fd, str, strlen(str)) != strlen(str) ){
        printf("Could not write to file!\n");
        close(fd);
    }
}

void write_int_to_file(int fd, int num){
    char str[40];
    sprintf(str, "%d", num);    // create a string consisting of the formated long double

    if( write(fd, str, strlen(str)) != strlen(str) ){
        printf("Could not write to file!\n");
        close(fd);
    }
}
/* END: UTILITY FUNCTIONS --------------------------------------------------------------------------- */





/* BEGIN: GRANDCHILDREN WORKER THREAD RUNNERS --------------------------------------------------------------------------- */
void * root_of_square_sums_runner(void * param){
    int index = *((int *) param);
    int array_offset = index * 1000; //each thread is responsible for a 1000 integer section of the array

    long double sum = 0;

    // iterate through the 1000 numbers
    for(int i = array_offset; i < array_offset + 1000; i++){
        // sum curr value^2
        long double square = pow((long double) InpArray[i], (long double) 2); //pow returns long double
        sum += square;
    }

    long double root = sqrt(sum);

    // return address of computed value directly to child thread
    // when a thread terminates, it's access to its local variables is lost
    // thus the reference &root shouldn't be used as a value for value_ptr
    long double * ret;

    //if allocating memory for the return value fails
    if((ret = (long double *) malloc(sizeof(root))) == NULL){
        printf("Could not allocate memory in thread %d!\n", index);
        pthread_exit(NULL);
    }

    // deref return pointer to assign value from root
    *ret = root;
    pthread_exit(ret); 
}


void * geometric_average_runner(void * param){
    int index = *((int *) param);
    int array_offset = index * 1000; //each thread is responsible for a 1000 integer section of the array

    // product of n numbers
    long double product = 1;

    // get the nth root of the current integer
    // multiply it by the product to avoid 
    // using pow on a large number as mentioned in class
    for(int i = array_offset; i < array_offset+1000; i++){
        long double nth_root = pow(InpArray[i], (long double) 1.0/1000);
        product *= nth_root;
    }

    // return pointer
    long double * ret;

    //if allocating memory for the return value fails
    if((ret = (long double *) malloc(sizeof(product))) == NULL){
        printf("Could not allocate memory in thread %d!\n", index);
        pthread_exit(NULL);
    }

    // deref return pointer to assign value from root
    *ret = product;
    pthread_exit(ret); 
}


void * arithmetic_averages_runner(void * param){
    int index = *((int *) param);
    int array_offset = index * 1000; //each thread is responsible for a 1000 integer section of the array

    //sum numbers
    long double sum = 0;
    for(int i = array_offset; i < array_offset+1000; i++){
        sum += InpArray[i];
    }

    long double quotient = sum / 1000;
    
    // return pointer
    long double * ret;

    //if allocating memory for the return value fails
    if((ret = (long double *) malloc(sizeof(quotient))) == NULL){
        printf("Could not allocate memory in thread %d!\n", index);
        pthread_exit(NULL);
    }

    // deref return pointer to assign value from root
    *ret = quotient;
    pthread_exit(ret); 
}
/* END: GRANDCHILDREN WORKER THREAD RUNNERS --------------------------------------------------------------------------- */