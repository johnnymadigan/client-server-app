#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "operations.h"



// References:
// *   https://man7.org/training/download/ipc_prog_pshm_slides.pdf
// *   CAB403 Topic 4 lecture notes and practical.

// Compile with flags: -lrt -lpthreads

/**
 * Friendly names for supported worker process operations.
 */
char* op_names[] = {
    "add", "sub", "mul", "div", "quit"
};

/**
 * POST: 

 * AND   (this code is provided for you, don't interfere with it) The shared
 *       semaphore has been initialised to manage access to the shared buffer.
 * AND   Semaphores have been initialised in a waiting state.
 *
 * \param shm The address of a shared memory control structure which will be
 *            populated by this function.
 * \param share_name The unique string used to identify the shared memory object.
 * \returns Returns true if and only if shm->fd >= 0 and shm->data != MAP_FAILED.
 *          Even if false is returned, shm->fd should contain the value returned
 *          by shm_open, and shm->data should contain the value returned by mmap.
 */
bool create_shared_object( shared_memory_t* shm, const char* share_name ) {
    // INSERT SOLUTION HERE
    shm_unlink(share_name);

    // INSERT SOLUTION HERE
    shm->name = share_name;

    // INSERT SOLUTION HERE
    shm->fd = shm_open(shm->name, O_RDWR | O_CREAT, 0666);
    if (shm->fd == -1) {
        perror("shm_open FAILED in create_shared_object()");
        shm->data = NULL;
        return false;
    }

    // INSERT SOLUTION HERE
    if (ftruncate(shm->fd, sizeof(shared_data_t)) != 0) {
        perror("ftruncate FAILED in create_shared_object()");
        shm->data = NULL;
        return false;
    }

    // INSERT SOLUTION HERE
    shm->data = mmap(NULL, sizeof(shared_data_t), PROT_WRITE | PROT_READ, MAP_SHARED, shm->fd, 0);
    if (shm->data == MAP_FAILED) {
        perror("mmap FAILED in create_shared_object()");
        return false;
    }

    // Do not alter the following semaphore initialisation code.
    sem_init( &shm->data->controller_semaphore, 1, 0 );
    sem_init( &shm->data->worker_semaphore, 1, 0 );

    // If we reach this point we should return true.
    return true;
}

void destroy_shared_object( shared_memory_t* shm ) {
    // INSERT SOLUTION HERE
    munmap(shm, sizeof(shared_data_t));
    shm_unlink(shm->name);
    shm->fd = -1;
    shm->data = NULL;
}

double request_work( shared_memory_t* shm, operation_t op, double lhs, double rhs ) {
    // Copy the supplied values of op, lhs and rhs into the corresponding fields 
    // of the shared data object. 
    shm->data->operation = op;
    shm->data->lhs = lhs;
    shm->data->rhs = rhs;

    // Do not alter the following semaphore code. It sends the request to the 
    // worker, and waits for the response in a reliable manner.
    sem_post( &shm->data->controller_semaphore );
    sem_wait( &shm->data->worker_semaphore );

    // Modify the following line to make the function return the result computed 
    // by the worker process. This will be stored in the result field of the 
    // shared data object.
    return shm->data->result;
}

bool get_shared_object( shared_memory_t* shm, const char* share_name ) {

    shm->fd = shm_open(share_name, O_RDWR, 0666);
    if (shm->fd == -1) {
        perror("shm_open FAILED in create_shared_object()");
        shm->data = NULL;
        return false;
    }

    shm->data = mmap(NULL, sizeof(shared_data_t), PROT_WRITE | PROT_READ, MAP_SHARED, shm->fd, 0);
    if (shm->data == MAP_FAILED) {
        perror("mmap FAILED in create_shared_object()");
        return false;
    }

    return true;
}

bool do_work( shared_memory_t* shm ) {
    bool retVal = true;

    // Do not alter the following instruction, which waits for work
    sem_wait( &shm->data->controller_semaphore );

    // Update the value of local variable retVal and/or shm->data->result
    // as required.
    // INSERT IMPLEMENTATION HERE
    if (shm->data->operation == 4) {
        retVal = false;
    } else if (shm->data->operation == 3) {
        shm->data->result = shm->data->lhs / shm->data->rhs;
    } else if (shm->data->operation == 2) {
        shm->data->result = shm->data->lhs * shm->data->rhs;
    } else if (shm->data->operation == 1) {
        shm->data->result = shm->data->lhs - shm->data->rhs;
    } else if (shm->data->operation == 0) {
        shm->data->result = shm->data->lhs + shm->data->rhs;
    }

    // Do not alter the following instruction which send the result back to the
    // controller.
    sem_post( &shm->data->worker_semaphore );

    // If retval is false, the memory needs to be unmapped, but that must be 
    // done _after_ posting the semaphore. Un-map the shared data, and assign
    // values to shm->data and shm-fd as noted above.
    // INSERT IMPLEMENTATION HERE
    if (retVal == false) {
        munmap(shm, sizeof(shared_data_t));
        shm->fd = -1;
        shm->data = NULL;
    }

    // Keep this line to return the result.
    return retVal;
}

double next_rand() {
    return 100.0 * rand() / RAND_MAX;
}

operation_t next_op() {
    return (operation_t)(rand() % op_quit);
}

#define SHARE_NAME "/xyzzy_123"

void controller_main() {
    srand( 42 );
    printf( "Controller starting.\n" );

    shared_memory_t shm;

    if ( create_shared_object( &shm, SHARE_NAME ) ) {
        for ( int i = 0; i < 20; i++ ) {
            operation_t op = next_op();
            double lhs = next_rand();
            double rhs = next_rand();
            double result = request_work( &shm, op, lhs, rhs );
            printf( "%s(%0.2f, %0.2f) = %0.2f\n", op_names[op], lhs, rhs, result );
        }

        request_work( &shm, op_quit, 0, 0 );
        printf( "Controller finished.\n" );

        destroy_shared_object( &shm );
    }
    else {
        printf( "Shared memory creation failed.\n" );
    }
}

void worker_main() {
    printf( "Worker starting.\n" );

    shared_memory_t shm;

    if ( get_shared_object( &shm, SHARE_NAME ) ) {
        while ( do_work( &shm ) ) {}

        printf( "Worker has been told to quit.\n" );
    }
    else {
        printf( "Shared memory connection failed.\n" );
    }
}

int main() {
    pid_t childPid = -1;
    // Invoke the fork function to spawn the worker process, and save the result
    // as childPid.
    childPid = fork();

    if ( childPid < 0 ) { /* error occurred */
        fprintf( stderr, "Fork failed\n" );
        return 1;
    }
    else if ( childPid == 0 ) {
        // Sleep 1 second to give the controller time to create the shared memory 
        // object, then invoke worker_main.
        sleep( 1 );
        worker_main();
    }
    else { /* parent process */
        // Invoke controller_main.
        controller_main();
    }

    return 0;
}



//gcc -Wall -Wno-unused-function -g -D MOCK_MEMORY=1 -lm -lrt -lpthread -o app shm_ipc.c