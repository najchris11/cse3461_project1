# Project 1

Hello everyone!!

This is my first project. You can run it by first running make all in the same directory as the project.
Then, you want to first run server.c by typing

``` bash
./server <portNum>
```

Similarly, you should run

``` bash
./client <samePortNum> 127.0.0.1
```

in another terminal window on the same machine.
The client will prompt you to type in a message. The message will be submitted upon pressing enter.
Then you should get a response in your terminal window, and in the window running server.c, you will also get a log of the exchange.
The server will continue running after furst successful exchange, so to stop the server, you must enter ^C (Control + C) into your command line.
Some limitations include the fact that the buffer only allocates 1 kibibyte of storage, so if you try to send more than 1024 chars, you will likely buffer overflow and break the program.
