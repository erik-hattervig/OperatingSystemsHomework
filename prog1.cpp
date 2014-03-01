/******************************************************************************
 * CSC 456 Homework #1
 * Author: Erik Hattervig
 * 
 * Compile instructions: make
 *
 * Usage: dsh
 * 
 *****************************************************************************/
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sys/types.h>
#include <signal.h>
#include <sstream>

using namespace std;

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
            cout << "Error: No arguments entered!\n";
        }
        // --------------------------------------------------------------------------
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
        else
        {
            cout << "Error: Command " << arguments[0] << " not found!\n";
        }

        // clear the arguments vector for the next command
        arguments.clear();

    }while( exit == false ); // loop while exit bool variable is false
    
    return;
}


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
