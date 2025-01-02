# LineProcessor
How to Compile

This program is written in C and requires the GCC compiler with support for `pthread` and the C99 standard.
To compile the program, run the following command in your terminal:

gcc -o line_processor line_processor.c -pthread -std=c99

This will produce an executable named `line_processor`.

How to Run

The program reads input from standard input and writes processed output to standard output.
Type into the terminal an input like this:

./line_processor < input.txt

swapping input.txt with your desired text file.

Run Interactively

You can type the input directly into the terminal. For example:

./line_processor

You can now type the contents of your text file directly into the console and recieve the same result. 
Then type the input text, pressing `Enter` after each line. Finish by typing the `STOP` line:


The processed output will be displayed in the terminal.

 Save Output to a File

You can save the output to a file using output redirection. For example:

./line_processor < input.txt > output.txt

Of course swapping the names of the text filea around if you so choose.

The processed output will be saved in "output.txt".
