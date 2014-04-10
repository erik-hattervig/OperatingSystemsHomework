/******************************************************************************
 * CSC 456 Homework #1
 * Author: Erik Hattervig
 * 
 * Compile instructions: make
 *
 * Usage: dsh
 * 
 *****************************************************************************/
 
// ****** INCLUDES ************************************************************
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <vector>
// ****************************************************************************

#define SHMKEY 1066
#define SEMKEY 1066
#define K 1024

using namespace std;

union semun
{
	int val;
	struct semid_ds *buf;
	unsigned short int *array;
	struct seminfo *__buf;
};


// ****** FUNCTION PROTOTYPES *************************************************
void changeDir( vector<string> &args );
void cmdnm( string id );
void controlLoop();
void fileRedir( vector<string> &args );

// mbox commands
void mboxcopy( vector<string> &args );
void mboxdel();
void mboxinit( vector<string> &args );
void mboxread( vector<string> &args );
void mboxwrite( vector<string> &args );

void parse( string inString , vector<string> &outStrings );
void pipe( vector<string> &args );
void remotePipeClient( vector<string> &args );
void remotePipeServer( vector<string> &args );
void signal( string signal_num , string id );
void systat();
void systemCommand( string line , bool info );
// ****************************************************************************

/******************************************************************************
* Author: Erik Hattervig
* Description: Entry point, calls the control loop function and exits when it
*   returns.
******************************************************************************/
int main()
{
    // enter the control loop
    controlLoop();
    
    return 0;
}

/******************************************************************************
* Author: Erik Hattervig
* Description: Changes the current working directory of the program from the
*   arguments passed in. The arguments are in a vector of strings.
******************************************************************************/
void changeDir( vector<string> &args )
{
    // check if we have the right number of arguments
    if ( args.size() < 2 )
    {
        cout << "Error: No directory specified\n";
    }
    else
    {
        // change directory using the chdir function
        chdir( args[1].c_str() );
    }
    return;
}

/******************************************************************************
* Author: Erik Hattervig
* Description: Grabs the Command name of a process from its PID that is passed
* to the function.
******************************************************************************/
void cmdnm( string id )
{
    ifstream fin;
    string path = "/proc/";    //default path for the proc folder
    string processName;

    //append the id to the path and the comm file
    path = path + id + "/comm";
    
    fin.open( path.c_str() );
    if( ! fin.is_open() )
    {
        cout << "Error: Process PID could not be found\n";
    }
    else
    {
        getline( fin , processName );
        cout << processName << "\n";
    }
    
    fin.close();
    return;
}

/******************************************************************************
* Author: Erik Hattervig
* Description: This function controls the user input in a loop. It will output
* dsh> and then wait for input. The input will then be parsed and passed onto
* the appropriate function or an error message will be outputted. The loop and
* function will exit when the exit command is provided by the user.
*
*
******************************************************************************/
void controlLoop()
{
    string input;               // raw input from the user
    bool exit = false;          // tells when to exit
    vector<string> arguments;   //the arguments separated


    do
    {
        // print out "dsh > " before the input is gotten
        cout << "dsh > ";
        
        // get input from user
        getline( cin , input );
		
        // parce sting
        if(input.length() != 0 ) 
        {
            parse( input , arguments );
        }
        // check for keywords and errors
        if (arguments.size() == 0 ) // No arguments entered error
        {
			// For Part 2 there will be no output
            // cout << "Error: No arguments entered!\n";
        }
        // --------------------------------------------------------------------
        // check for piping
        else if ( input.find(" | ") != string::npos )
        {
            pipe( arguments );
        }
        // --------------------------------------------------------------------
        // check for remote shell server pipping 
        else if ( input.find(" )) ") != string::npos )
        {
            remotePipeServer( arguments );
        }
        // --------------------------------------------------------------------
        // check for remote shell clent pipping 
        else if ( input.find(" (( ") != string::npos )
        {
            remotePipeClient( arguments );
        }
        // --------------------------------------------------------------------
        else if ( arguments[0] == "exit" ) // Exit command entered
        {
            exit = true;
        }
        // --------------------------------------------------------------------
        else if( arguments[0] == "cmdnm" ) // cmdnm command entered
        {
            if( arguments.size() != 2 ) // make sure has right number of arguments
            {
                cout << "Error: Wrong number of arguments entered for cmdnm!\n";
            }
            // checks to see if input is all digits
            else if ( arguments[1].find_first_not_of("0123456789") == string::npos )
            {
                // arguments for cmdnm look good, call cmdnm function
                cmdnm( arguments[1] );
            }
            else
                cout << "Error: Invalid program id!\n";
        }
        // --------------------------------------------------------------------
        else if ( arguments[0] == "signal" ) // signal command entered
        {
            if ( arguments.size() != 3 )
            {
                cout << "Error: Wrong number of arguments entered for signal!\n";
            }
            // check to see if input arrgs are all digits
            else if ( arguments[1].find_first_not_of("0123456789") == string::npos
                && arguments[2].find_first_not_of("0123456789") == string::npos )
            {
                // argument for signal look good, call signal function
                signal( arguments[1] , arguments[2] );
            }
            else
                cout << "Error: Invalid arguments!\n";

        }
        // --------------------------------------------------------------------
        else if ( arguments[0] == "systat" ) // systat command entered
        {
            // call systat function
            systat();
        }
        // --------------------------------------------------------------------
		else if ( arguments[0] == "mboxinit" ) // the command for mboxinit
		{
			if( arguments.size() > 2 )
			{
				mboxinit( arguments );
			}
		}
		// --------------------------------------------------------------------
		else if ( arguments[0] == "mboxdel" ) // the command for mboxdel
		{
			mboxdel();
		}
		// --------------------------------------------------------------------
		else if ( arguments[0] == "mboxwrite" ) // the command for mboxwrite
		{
			mboxwrite( arguments );
		}
		// --------------------------------------------------------------------
		else if ( arguments[0] == "mboxread" ) // the command for mboxread
		{
			mboxread( arguments );
		}
		// --------------------------------------------------------------------
		else if ( arguments[0] == "mboxcopy" ) // the command for mboxcopy
		{
			mboxcopy( arguments );
		}
        // check for file redirection
        else if ( arguments.size() != 1 )
        {
            if ( arguments[1] == "<" || arguments[1] == ">" )
            {
                fileRedir( arguments );
            }
        }
        // --------------------------------------------------------------------
        else if ( arguments[0] == "cd" ) // the cd command was entered
        {
            // send the input line to the changeDir function
            changeDir( arguments );
        }
		// --------------------------------------------------------------------
        else
        {
            // The command may be a shell process we need to send it to the
            // system command function to be parsed and ran
            systemCommand( input , true );
            
            // Not needed for this project
            // cout << "Error: Command " << arguments[0] << " not found!\n";
        }

        // clear the arguments vector for the next command
        arguments.clear();

    }while( exit == false ); // loop while exit bool variable is false
    
	mboxdel();
	
    return;
}

/*****************************************************************************
* Author: Erik Hattervig
* Description: Takes and redirects a files contents to be used as arguments
* for a Unix command. Does error checking and creates a c-string to be given
* to the execl function.
******************************************************************************/
void fileRedir( vector<string> &args )
{
    int fpt;
    int pid;
    int waitpid;
    int status;
    string line;
    int numArgs = 0;
    
    // check to make sure there are enough arguments
    if ( args.size() < 3 )
    {
        cout << "Error: Not enough arguments for file redirection" << endl;
    }
    else
    {
        pid = fork();
        if ( pid == 0 )
        {
            // child process executes here
            
            if( args[1] == "<" )
            {
                // open for input
                if ( ( fpt = open( args[2].c_str() , O_RDONLY ) ) == -1 )
                {
                    cout << "Unable to open file for reading" << endl;
                    exit(1);
                }
                close( 0 );     // close child standard input
                if ( dup( fpt ) != 0 )    // redirect the child input
                {
                    cout << "Error on input dup" << endl;
                }   
                close( fpt );   // close unnecessary file descriptor
                
                // get the input from the file
                getline( cin , line );
                
                cout << line << endl;
                
                line = args[0] + " " + line;
                
            }
            else
            {
                // open for output
                if ( ( fpt = creat( args[2].c_str() , 0644 ) ) == -1 )
                {
                    cout << "Unable to open file for output" << endl;
                    exit(1);
                }
                close( 1 );     // close child standard output
                if ( dup( fpt ) != 1 )    // redirect the child output
                {
                    cout << "Error on output dup" << endl;
                }
                close( fpt );   // close unnecessary file descriptor
                
                line = args[0];
                
            }
            // execl( args[0].c_str() , cArgs[1] , 0 );
            
            // call command and suppress process information output
            systemCommand( line , false );
            exit(0);
        }
        // Parent process executes here
        waitpid = wait ( &status );
        
    }
    return;
}

/******************************************************************************
* Author: Erik Hattervig
*
* copies the contents of on box into another one
******************************************************************************/
void mboxcopy( vector<string> &args )
{
	ifstream fin;
	int shmid;
	int numBoxes;
	int size;
	int box1 = atoi( args[1].c_str() );
	int box2 = atoi( args[2].c_str() );
	char *addr;
	char *addrRead;
	char *addrWrite;
	int i;
	int semid;
	struct sembuf lock;
	
	
	// get shared memory stats 
	fin.open("sharedMemoryID.txt");
	if( ! fin.is_open() )
	{
		cout << "Error: No shared memory" << endl;
		return;
	}
	fin >> shmid;
	fin >> numBoxes;
	fin >> size;
	fin >> semid;
	fin.close();
	
	if( box1 > numBoxes || box2 > numBoxes )
	{
		cout << "Error: There are only " << numBoxes << " boxes." << endl;
		return;
	}
	
	// loop while semaphore is locked
	while( semctl( semid, 0, GETVAL, 0) == 0 )
	{
		// the semaphore is locked wait until it is not
	}
	
	// lock the semaphore
	lock.sem_num = 0;  // semaphore index
	lock.sem_op = -1;  // the operation
	lock.sem_flg =  IPC_NOWAIT;  // operation flags
	semop( semid, &lock, 1);     // perform the requested operation
	
	
	// attach the shared memory to process
	addrRead = (char*)shmat( shmid, 0, 0 );
	addrWrite = (char*)shmat( shmid, 0, 0 );
	
	// copy data to the other box
	for( i = 0 ; i < size ; i++ )
	{
		*(addrWrite + i + (size*box1) ) = *( addrRead + i + (size*box2) );
	}
	
	// unlock the semaphore
	lock.sem_num = 0;  // semaphore index
	lock.sem_op = 1;   // the operation
	lock.sem_flg =  IPC_NOWAIT;  // operation flags
	semop( semid, &lock, 1);     // perform the requested operation
	
	return;
}

/******************************************************************************
* Author: Erik Hattervig
* 
* Deletes all of the shared memory on the system
******************************************************************************/
void mboxdel()
{
	ifstream fin;
	int size;
	int numBoxes;
	int shmid;
	int semid;
	
	// open the file containing the shared memory information
	fin.open("sharedMemoryID.txt");
	if( ! fin.is_open() )
	{
		return;
	}
	fin >> shmid;
	fin >> numBoxes;
	fin >> size;
	fin >> semid;
	fin.close();
	
	system( "rm sharedMemoryID.txt" );
	
	// release shared memory
	shmctl( shmid, IPC_RMID, 0 );
	
	// remove the semaphore
	semctl( semid, 0, IPC_RMID, 0);
	
	return;
}

/******************************************************************************
* Author: Erik Hattervig
* 
* Creates the mailboxes and save a file to find the id and size of the first
* one.
*
* arg[1] - number of mailboxes
* arg[2] - size of mailbox in kbytes
******************************************************************************/
void mboxinit( vector<string> &args )
{
	int i;
	int shmid;
	//char *addr;
	//int *pint;
	ofstream mboxfile;
	int size = atoi(args[2].c_str());
	int semid;
	union semun options;
	
	// Using ftok( "sharedMemoryID.txt", 'a' ), create one block of shared memory region with access permissions
	// 0666, this is the first mailbox will hold addresses to the other boxes
	shmid = shmget( SHMKEY, size * K * ( atoi( args[1].c_str() ) + 1 ) , IPC_CREAT | IPC_EXCL | 0666 );
	if( shmid < 0 )
	{
		printf( "Error: shmid is %d\n", shmid );
		return;
	}
	
	// Create Semaphore
	semid = semget( SEMKEY, 1, IPC_CREAT | IPC_EXCL | 0666 );
	
	// Initialize the semaphore at index 0
	options.val = 1;
	semctl(semid , 0, SETVAL, options);
	
	// Test that the semaphore was created correctly:
	if (semctl ( semid, 0, GETVAL, 0 ) == 0 )
	{
		printf( "Error: Can not lock semaphore.\n");
	}
	
	
	// write the memory id out to a file for other processes to find
	mboxfile.open( "sharedMemoryID.txt" );
	mboxfile << shmid << endl << args[1] << endl << size*K << endl << semid;
	mboxfile.close();
	
	return;
}

/******************************************************************************
* Author: Erik Hattervig
*
* Reads what is in the memory block until a null terminator of the end of the
* memory.
******************************************************************************/
void mboxread( vector<string> &args )
{
	ifstream fin;
	int shmid;
	int numBoxes;
	int size;
	int boxid = atoi( args[1].c_str() );
	char *addr;
	int i;
	int semid;
	struct sembuf lock;
	
	// get shared memory stats 
	fin.open("sharedMemoryID.txt");
	if( ! fin.is_open() )
	{
		cout << "Error: No shared memory" << endl;
		return;
	}
	fin >> shmid;
	fin >> numBoxes;
	fin >> size;
	fin >> semid;
	fin.close();
	
	if( boxid > numBoxes )
	{
		cout << "Error: No box at that number" << endl;
		return;
	}
	
	// loop while semaphore is locked
	while( semctl( semid, 0, GETVAL, 0) == 0 )
	{
		// the semaphore is locked wait until it is not
	}
	
	// lock the semaphore
	lock.sem_num = 0;  // semaphore index
	lock.sem_op = -1;  // the operation
	lock.sem_flg =  IPC_NOWAIT;  // operation flags
	semop( semid, &lock, 1);     // perform the requested operation
	
	// attach the shared memory to process
	addr = (char*)shmat( shmid, 0, 0 );
	
	// print out the data in the stored memory
	// prints out till end of string or end of box
	for( i = 0 ; i < size && *( addr + i + (size * boxid) ) != '\0' ; i++ )
	{
		cout << *( addr + i + (size * boxid) );
	}
	
	cout << endl;
	
	// unlock the semaphore
	lock.sem_num = 0;  // semaphore index
	lock.sem_op = 1;   // the operation
	lock.sem_flg =  IPC_NOWAIT;  // operation flags
	semop( semid, &lock, 1);     // perform the requested operation
	
	return;
}

/******************************************************************************
* Author: Erik Hattervig
*
* Writes what the user types to the memory block, until ctrl-D is given.
******************************************************************************/
void mboxwrite( vector<string> &args )
{
	ifstream fin;
	int shmid;
	int numBoxes;
	int size;
	int boxid = atoi( args[1].c_str() );
	char *addr;
	// int *pint;
	int buffer;
	int totalBytes = 0;
	int i = 0;
	int semid;
	struct sembuf lock;
	
	// get shared memory stats 
	fin.open("sharedMemoryID.txt");
	if( ! fin.is_open() )
	{
		cout << "Error: No shared memory" << endl;
		return;
	}
	fin >> shmid;
	fin >> numBoxes;
	fin >> size;
	fin >> semid;
	fin.close();
	
	if( boxid > numBoxes )
	{
		cout << "Error: No box at that number" << endl;
		return;
	}
	
	// loop while semaphore is locked
	while( semctl( semid, 0, GETVAL, 0) == 0 )
	{
		// the semaphore is locked wait until it is not
	}
	
	// lock the semaphore
	lock.sem_num = 0;  // semaphore index
	lock.sem_op = -1;  // the operation
	lock.sem_flg =  IPC_NOWAIT;  // operation flags
	semop( semid, &lock, 1);     // perform the requested operation
	
	// attach the shared memory to process
	addr = (char*)shmat( shmid, 0, 0 );
	//pint = (int *) addr;
	
	// read in the user's string till ctrl-D
	buffer = getchar();
	while( buffer != EOF && totalBytes <= size )
	{
		
		// put buffer into box
		*(addr + i + (size * boxid) ) = (char)buffer;
		
		totalBytes++;
		buffer = getchar();
		i++;
	}
	cout << endl;
	
	// unlock the semaphore
	lock.sem_num = 0;  // semaphore index
	lock.sem_op = 1;   // the operation
	lock.sem_flg =  IPC_NOWAIT;  // operation flags
	semop( semid, &lock, 1);     // perform the requested operation
	
	return;
}

/******************************************************************************
* Author: Erik Hattervig
* Description: Parses a string given by spaces and puts the separated element
* into a vector
******************************************************************************/
void parse( string inString , vector<string> &outStrings )
{
    inString = inString + ' ';
    do
    {
        outStrings.push_back( inString.substr( 
            0 , inString.find( ' ' , 0 ) ) );

        inString.erase( 0 , inString.find( ' ' , 0 ) + 1 );

    } while (inString.length() > 0 );

    return;
}

/******************************************************************************
* Author: Erik Hattervig
* Description: Pipes two arguments together.
******************************************************************************/
void pipe( vector<string> &args )
{
    int fd_pipe[2];
    int pid1;
    int pid2;
    int status;
    int wpid;
    int separator = 0;
    string line;
    int i;
    
    // find where the | is in the arguments
    while ( args[separator] != "|" )
    {
        separator = separator + 1;
    }
    
    pid1 = fork();
    if ( pid1 == 0 )
    {
        // child process executes here for input side of pipe
        
        pipe( fd_pipe );    // create pipe
        
        pid2 = fork();
        if ( pid2 == 0 )
        {
            // grandchild process executes here for output of pipe
            close(1);               // close standard output
            dup( fd_pipe[1] );      // redirect the output
            close( fd_pipe[0] );    // close unnecessary file descriptor
            close( fd_pipe[1] );    // close unnecessary file descriptor
            
            // put command together for systemCommand function
            for( i = 0 ; i < separator ; i++ )
            {
                line += args[i];
                line += " ";
            }
            //execute command
            systemCommand( line , false );
            exit(0);
        }
        
        // back to process for input side of pipe
        close( 0 );
        dup( fd_pipe[0] );
        close( fd_pipe[0] );
        close( fd_pipe[1] );
        
        wpid = wait( &status );
        // put command together for systemCommand function
        for( i = separator + 1 ; i < args.size() ; i++ )
        {
            line += args[i];
            line += " ";
        }
        // execute command
        systemCommand( line , false );
        exit(0);
    }
    // parent process executes here
    wpid = wait( &status );
    
    return;
}

/******************************************************************************
* Author: Erik Hattervig
* Description: Creates remote shell client pipe
* Credit to Alex Wulff for helping me understand these commands.
******************************************************************************/
void remotePipeClient( vector<string> &args )
{
    int sockfd, n;
    int BUFFERSIZE = 100000;
    char buffer[BUFFERSIZE];
    struct sockaddr_in serv_addr;
    struct hostent *server;
    int separator = 0;
    string cmd;
    int i;
    
    // find the ip address and port
    while( args[separator] != "((" )
    {
        separator += 1;
    }
    for( i = 0 ; i < separator ; i++ )
    {
        cmd += args[i] + " ";
    }
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    {
        printf("Error opening socket");
    }
    
    server = gethostbyname( args[separator + 1].c_str() );
    if (server == NULL) 
    {
        fprintf( stderr, "Error, no such host\n" );
        return;
    }
    
    bzero( (char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy( (char *) server->h_addr, 
        (char *) &serv_addr.sin_addr.s_addr,
        server->h_length);
    serv_addr.sin_port = htons( atoi( args[separator + 2].c_str() ) );
    
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    {
        printf("Error connecting");
    }
    
    // empty the buffer
    bzero( buffer , BUFFERSIZE );

    n = write( sockfd, cmd.c_str(), cmd.length());
    
     if (n < 0)
    { 
        printf("Error writing to socket");
    }

    if (cmd.compare("exit") == 0)
    {
        return;
    }

    bzero( buffer, BUFFERSIZE );
    n = read( sockfd, buffer, BUFFERSIZE - 1 );

    if (n < 0) 
    {
        printf( "Error reading from socket" );
    }

    printf( "%s\n", buffer );
    close( sockfd );
    return;
}

/******************************************************************************
* Author: Erik Hattervig
* Description: Creates remote shell server pipe
* Modified from Robert Ingalls' code example
* http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html
******************************************************************************/
void remotePipeServer( vector<string> &args )
{
    int sockfd, newsockfd, portno, n;
    int BUFFERSIZE = 100000;
    char buffer[BUFFERSIZE];
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;
    int separator = 0;
    
    // find the ip address and port
    while( args[separator] != "))" )
    {
        separator += 1;
    }
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if( sockfd < 0)
    {
        printf("ERROR opening socket\n");
    }
    bzero( (char*) &serv_addr, sizeof(serv_addr) );
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons( atoi( args[separator + 1].c_str() ) );
    
    if ( bind( sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr) )
        < 0 )
    {
        printf( "Error on binding" );
    }
    
    while (1)
    {
        listen( sockfd, 5 );
        clilen = sizeof(cli_addr);
        newsockfd = accept( sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if ( newsockfd < 0 )
        {
            printf("Error on accept");
        }
        bzero( buffer, BUFFERSIZE );
        n = read( newsockfd, buffer, BUFFERSIZE - 1 );
        if ( n < 0 )
        {
            printf( "Error reading from socket" );
        }
        
        // take command in and make a string
        string reply ( buffer );
        //terminate the loop
        if ( reply.compare( "exit" ) == 0 )
        {
            return;
        }
        //execute
        systemCommand( reply , false );
        
        // reply with what was sent
        reply = reply + " executed.";
        n = write( newsockfd, reply.c_str(), reply.length() );
        if( n < 0 )
        {
            printf( "Error writing to socket" );
        }
    }
    return;
}

/******************************************************************************
* Author: Erik Hattervig
* Description: Send a signal to a prosses using the kill command
******************************************************************************/
void signal( string signal_num , string id )
{
    int pid;
    int sig;

    istringstream ( id ) >> pid;
    istringstream ( signal_num ) >> sig;

    //perform kill command and check if signal was sent
    if ( kill( pid , sig ) != 0 )
    {
        cout << "Error: Signal could not be sent";
    }
}

/******************************************************************************
* Author: Erik Hattervig
* Description: This function prints out information about the system. It prints
* out the Linux version, system uptime, memory information, and cpu
* information to the console using the files in the proc directory.
******************************************************************************/
void systat()
{
    ifstream fin;
    int i;
    string buffer;
    
    //open version file in the proc folder and print out the info
    fin.open( "/proc/version" );
    cout << "Linux Version:\n";
    getline( fin , buffer );
    cout << buffer << "\n";
    fin.close();
    
    //open uptime file in the proc folder and print out the info
    fin.open( "/proc/uptime" );
    cout << "System Uptime:\n";
    getline( fin , buffer );
    cout << buffer << "\n";
    fin.close();

    //open the meminfo in the proc folder and print out the info
    fin.open( "/proc/meminfo" );
    cout << "Memory Information:\n";
    getline( fin , buffer );
    cout << buffer << "\n";
    getline( fin , buffer );
    cout << buffer << "\n";
    fin.close();

    //open the cpuinfo file in the proc folder and print out the info
    // about the processer
    fin.open( "/proc/cpuinfo" );
    getline( fin , buffer );
    cout << "CPU Information:\n";
    for ( i = 0 ; i < 7 ; i++ )
    {
        getline( fin , buffer );
        cout << buffer << "\n";
    }
    
    fin.close();

    return;
}

/******************************************************************************
* Author: Erik Hattervig
* Description: Forks the process and execute the command given to it and then
* outputs the stdout and joins back to the main process.
******************************************************************************/
void systemCommand( string line , bool info )
{
    int childpid;       // the pid of the child process
    int waitpid;
    int status;         // the exit status of a child function when it exits
    char *cArgs[100];   // a c-string array of the arguments
    int numArgs = 0;    // the number of arguments
    rusage *usage;      // a stuct pointer used to get information about a 
                        //    process
    int i;
    
    // convert my sting into a c-string array
    cArgs[ numArgs ] = strtok( (char*)line.c_str() , " " );
    while ( cArgs[ numArgs ] != NULL )
    {
        numArgs++;
        cArgs[ numArgs ] = strtok( NULL, " " );
    }
    numArgs--;
    
    // fork the process
    childpid = fork();
    // parent will print out the child pid
    if ( childpid != 0 && info == true )
    {
        printf( "The child pid is %d\n" , childpid );
    }
    
    // The child process will enter this if block
    if ( childpid == 0 )  
    {
        // execute the command
        execvp( cArgs[0] , cArgs );
        // if command did not exit then there was a problem, exit with code 5
        perror( "Exec failed: ");
        exit(5);
    }
    
    // wait for the child to finish
    waitpid = wait ( &status );
    
    if( info == true )
    {
        // print out the child exit information
        printf( "Shell process %d exited with status %d\n", waitpid, 
            ( status >> 8 ) );
            
        // get the process usage for the child process
        getrusage( waitpid , usage );
        
        // print out information about process
        cout << "  Information for process: " << waitpid << "\n";
        cout << "    Time for user: " << usage->ru_utime.tv_usec << " microseconds.\n";
        cout << "    Time for system: " << usage->ru_stime.tv_usec << " microseconds.\n";
        cout << "    Soft page faults: " << usage->ru_minflt << "\n";
        cout << "    Hard page faults: " << usage->ru_majflt << "\n";
        cout << "    Swaps: " << usage->ru_nswap << endl;
    }
    return;
}

