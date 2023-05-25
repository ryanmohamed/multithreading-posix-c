# Multithreaded Calculation Program

This program is designed to perform multithreaded calculations on a large array of numbers. It uses multiple threads to compute various statistics and store the results in global arrays. The program supports parallel processing to enhance performance.

## Purpose

The purpose of this program is to demonstrate the use of multithreading for concurrent computations on a large dataset. It showcases the creation of threads, their synchronization using join operations, and communication between threads through global variables. The program calculates three statistics for each section of the input array, which are the root of the sum of squares, geometric average, and arithmetic average.

## Features

- Multithreaded computation: The program creates multiple threads to process different sections of the input array concurrently.
- Thread synchronization: The main program thread waits for all worker threads to finish their computations before proceeding.
- Global data storage: The program uses global arrays to store the computed results, which are later accessed by the main thread for further processing.
- File I/O: The program reads input from a specified file and writes the results to an output file.
- Error handling: The program checks for errors during file operations and memory allocation.

## Usage

To compile and run the program, follow these steps:

1. Open a terminal or command prompt.

2. Navigate to the directory containing the program files.

3. Compile the program by executing the following command:

    ```bash
        gcc -o program MultiThreading.c -lm -lpthread
    ```

    The compilation command uses the following flags:
    - `-o program`: Specifies the name of the output file. In this case, the executable will be named "program."
    - `-lm`: Links the math library. This flag is necessary because the program uses math functions, such as square roots.
    - `-lpthread`: Links the pthread library. This flag is necessary for enabling multithreading capabilities in the program.

4. Run the program by executing the following command:

Replace `INPUT_FILE` with the path to your input file and `OUTPUT_FILE` with the desired output file path.
5. The program will read the input file, perform the calculations, and write the results to the output file.
6. After execution, you can examine the output file to view the computed statistics for each thread and the minimum values.

## Input File Format

The input file should contain a list of integers separated by newline characters. The program expects the file to have exactly 14,000 integers. Each thread will process a section of 1,000 integers from the input array.

## Output File Format

The output file will contain the following information:

- For each worker thread:
- Thread number
- Root of the sum of squares for the corresponding section of the input array
- Geometric average for the corresponding section of the input array
- Arithmetic average for the corresponding section of the input array
- Minimum values of the root of the sum of squares, geometric averages, and arithmetic averages among all worker threads.

## License

This program is provided under the [MIT License](https://opensource.org/licenses/MIT). Feel free to modify and distribute it according to the terms of the license.

