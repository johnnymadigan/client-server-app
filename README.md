# Client-Server app using POSIX Shared Memory
My implementation of a simple client-server app using POSIX shared memory to perform inter-process communication.

Given structs for data/memory objects, along with a test driver that creates 2 processes, the challenge is to implement the source code which creates the shared memory object, gets it, and destroys it. Futhermore, being able to request and complete 'work', which is a series of arithmetic equations. 

# Run
- Run using a LINUX VM for safety
- Ensure source.c and header.h files are in the same folder
- Compile using: `gcc -Wall -D MOCK_MEMORY=1 -lm -lrt -lpthread -o app shm_ipc.c`
- Shorter compile line: `gcc -Wall -lm -lrt -lpthread -o app shm_ipc.c`
- Run using: `./app`

# Expected output from the test suit
```
Controller starting.
Worker starting.
mul(33.00, 69.06) = 2278.85
sub(20.63, 25.01) = -4.39
sub(86.36, 30.17) = 56.20
div(36.50, 76.54) = 0.48
sub(13.57, 10.67) = 2.90
mul(8.17, 55.10) = 449.98
mul(74.33, 98.18) = 7297.13
add(45.44, 51.87) = 97.30
div(55.52, 72.85) = 0.76
add(79.61, 58.37) = 137.98
sub(82.96, 91.37) = -8.41
mul(25.21, 11.99) = 302.36
add(88.86, 98.36) = 187.22
add(91.36, 34.86) = 126.21
div(23.14, 48.43) = 0.48
div(99.21, 56.60) = 1.75
add(55.68, 30.92) = 86.60
div(77.56, 76.36) = 1.02
add(34.94, 31.89) = 66.83
sub(97.83, 11.50) = 86.33
Worker has been told to quit.
Controller finished.
```