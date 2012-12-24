ManorPublic
===========

This is my rendition of the classic arcade game 'Asteroids' written in C and using the code library OpenGL. To compile this program, download all the files and store it in a folder. 

On a linux computer, the program can be compiled using the command:

gcc -o game asteroids.c -I /usr/include/GL/ -L /usr/include/GL/ -lglut -lGL -lGLU -lX11

On a Mac, the program can be compiled using the command
 
gcc -o game asteroids.c -framework GLUT -framework OpenGL -framework Carbon

Note: If your computer does not have the appropriate OpenGL header files in its include directory, then the code will not be able to compile (however, most computers come with these header files pre-installed)

I will post instructions on how to compile this program on windows when I get access to a windows computer for testing. 

Once compiled, this program can be run by typing ./game into your terminal.

Enjoy.
