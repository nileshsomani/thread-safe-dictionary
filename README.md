# Multi-Threaded Dictionary service application
This project contains a dictionary service application that stores words in memory. The service runs and listens on **locahost:8080** socket. The maximum memory footprint of the dictionary data structre is 1M.

To interact with the dictionary service, use the dictionary client command line utility.

# Design
- Using Simple TRIE data structure for storing words in memory.
- Max word length supported is 50.
- The dictionary service is started as a socket program that listens on localhost:8080
- For every new connection, a new worked thread is spawned to perform operations on the dictionary concurrently
- A global mutex is used to synchronize the access to TRIE data structure
- In case of delete action, a soft delete is perfomed(mark node as not end of word). If the node has no children, the a new thread is spawned to asyngronously recursively delete the nodes.
- 1M memory footprint is managed by using a global count of maximum number of TRIE nodes that can be allocated in memory.
- Interrupt handler is created to capture ctrl-C interrupt on always running dictionary service and perform cleanup of memory, distruction of locks.

# Build and Usage
To build the dictionary service and client, execute the following command:

> make

This will generate two binaries:
>- dictionary_service
>- dictionary

#### dictionary_service
This will start the service and wait for clients to interact with it

#### dictionary
This is the command line interface to interact with the dictionary
>dictionary {--insert word | --search word | --delete word}
>- insert: Inserts word in the dictionary
>- search: Search word in the dictionary
>- delete: Delete word from the dictionary

# References
- Computer Systems - A Programmers Perspective(2nd Edition)

# Enhancements
- Currently, dictionary service uses a global lock. This can be optimized more on granular level by using locking at node level.
- TRIE data structure can be further optimized by using converting it to compressed TRIE where long chains of single child edges are converted to single edge.
