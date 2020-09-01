# Extremely minimal static thread pool in C
* Uses POSIX threads.
* Uses semaphores.
* No lazy creation. Every worker start at the pool creation.
* No immediate shutdown. Workers will wait until tasks are completed. 
