# FileManagementServer-On-VirtualFileSystem

Introduction
The purpose of this lab is to make your server built in lab 10 multi-user friendly. 
In lab 10 you have developed a server which provides remote access to file system you developed in lab 8. In this lab we will protect the files from reader writer problem.
The description of reader-writer problem is described in section 6.6 of your text book.
You may use monitors, locks or semaphore libraries provided by the language APIs to implement the tasks
Objectives
By the end of this lab you will learn practical uses of synchronization and implement it on reader-writer problem.
Tasks
Your first task is to implement mutual exclusion of file access. 
Multiple threads can attempt to access your files. The first task is to make sure that at one time only one user 
is able to edit a file (create or write)

Your second task is to implement a queue for readers. Similar to task one, multiple threads can attempt to read a file and some can request a write. Your second task is to make sure that while the file is being read not write is allowed.

Your third task is to implement a limit on the user name to access a file.  Multiple users can access your system. Though it is not required that you implement any type of security, but any user may not access more than 5 files. If more than 5 requests are placed then the requesting thread must wait. 
Your fourth task will be to comment on the possibility of deadlock in the system if all the tasks are executed.

Requirements
                1. You must provide two programs, a server and a client.
                2. The client must allow the user to specify the ip address of the server
                3. The client will allow user to first specify the user name (which will be used in lab 10).
                4. The client must provide an interface to apply the operations developed in lab 6.
                5. The client must give errors when the server is not available.
                6. The client must display the response of the actions performed.
                7. The server must respond to multiple requests at the same time (this will require threads)
                9. The server and clients can be run on different machines.
