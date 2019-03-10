# Multi-Threaded Dictionary service application
This project contains a dictionary service application that stores words in memory. The service runs and listens on **locahost:8080** socket. The maximum memory footprint of the dictionary data structre is 1M.

To interact with the dictionary service, use the dictionary client command line utility.

#Design

#Build and Usage
To build the dictionary service and client, execute the following command:

> make

This will generate two binaries:
>- dictionary_service
- dictionary

#### dictionary_service
This will start the service and wait for clients to interact with it

#### dictionary
This is the command line interface to interact with the dictionary
>dictionary {--insert word | --search word | --delete word}
- insert: Inserts word in the dictionary
- search: Search word in the dictionary
- delete: Delete word from the dictionary

#References
- Computer Systems - A Programmers Perspective(2nd Edition)

#Enhancements
- Currently, dictionary service uses a global lock. This can be optimized more on granular level by using locking at node level.
- TRIE data structure can be further optimized by using converting it to compressed TRIE where long chains of single child edges are converted to single edge.
