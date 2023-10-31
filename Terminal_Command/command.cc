
/*
 * CS354: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include "command.h"
#include <regex.h>


SimpleCommand::SimpleCommand()
{
	// Creat available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **)malloc(_numberOfAvailableArguments * sizeof(char *));
}

void SimpleCommand::insertArgument(char *argument)
{
	if (_numberOfAvailableArguments == _numberOfArguments + 1)
	{
		// Double the available space
		_numberOfAvailableArguments *= 2;
		_arguments = (char **)realloc(_arguments,
									  _numberOfAvailableArguments * sizeof(char *));
	}

	_arguments[_numberOfArguments] = argument;

	// Add NULL argument at the end
	_arguments[_numberOfArguments + 1] = NULL;

	_numberOfArguments++;
}

Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc(_numberOfSimpleCommands * sizeof(SimpleCommand *));

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_append=0 ;
	_background = 0;
}

void Command::insertSimpleCommand(SimpleCommand *simpleCommand)
{
	if (_numberOfAvailableSimpleCommands == _numberOfSimpleCommands)
	{
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **)realloc(_simpleCommands,
													_numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));
	}

	_simpleCommands[_numberOfSimpleCommands] = simpleCommand;
	_numberOfSimpleCommands++;
}

void Command::clear()
{
	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
		for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
		{
			free(_simpleCommands[i]->_arguments[j]);
		}

		free(_simpleCommands[i]->_arguments);
		free(_simpleCommands[i]);
	}

	if (_outFile)
	{
		free(_outFile);
	}

	if (_inputFile)
	{
		free(_inputFile);
	}

	if (_errFile)
	{
		free(_errFile);
	}

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_append=0 ;
	_background = 0;
}

void Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");

	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
		printf("  %-3d ", i);
		for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
		{
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[j]);
		}
	}

	printf("\n\n");
	printf("  Output       Input        Error        Background\n");
	printf("  ------------ ------------ ------------ ------------\n");
	printf("  %-12s %-12s %-12s %-12s\n", _outFile ? _outFile : "default",
		   _inputFile ? _inputFile : "default", _errFile ? _errFile : "default",
		   _background ? "YES" : "NO");
	printf("\n\n");
}


void
Command::execute()
{
	
	if ( _numberOfSimpleCommands == 0 ) {
		prompt();
		return;
	}
	
	
	print() ;
	int defaultin = dup(0);
	int defaultout = dup(1);
	int defaulterr = dup(2);
	
	int fdout, fdin, fderr;
    _inputFile ?  fdin = open(_inputFile, O_RDONLY, 0664): fdin = dup(defaultin);
	int pid;
	int i= 0;
	int last_command=_numberOfSimpleCommands - 1;

	while (i<_numberOfSimpleCommands)
	{
	    dup2(fdin,0);
	    dup2(fderr,2);
	    close(fdin);
	    if(i == last_command) 
	      {
		if(_outFile)
		  {
		    if(!_append)
		      fdout = open(_outFile, O_CREAT|O_WRONLY|O_TRUNC, 0664);
		    else
		      fdout = open(_outFile, O_CREAT|O_WRONLY|O_APPEND, 0664);
		    if(fdout < 0 )
		      {
			perror("outFile open error");
			exit(1);
		      }
		  }
		else
		  {
		    fdout = dup(defaultout);
		  }
	      }
	    else 
	      {
		int fdpipe[2];
		pipe(fdpipe);
		fdout=fdpipe[1];
		fdin=fdpipe[0];
	      }
	    dup2(fdout,1);
	    close(fdout);
  
	    pid = fork();
	    if (pid == 0) 
	      {
		execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments);
		perror("execvp");
		_exit(1);
	      }
	    else if (pid < 0) 
	      {
		perror("fork");
		return;
	      }
		  i++;
	  }

	dup2(defaultin, 0);
	dup2(defaultout, 1);
	dup2(defaulterr, 2);
	close(defaultin);
	close(defaultout);
	close(defaulterr);

	if (!_background) 
	  {
	  
	    waitpid(pid, NULL, 0);
	  }


	clear();
	prompt();
	
}


void Command::prompt()
{
	printf("myshell>");
	fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;

int yyparse(void);
void disp( int sig )
{
	fprintf( stderr, "\n    Ctrl-c Disabled\n");
	Command::_currentCommand.prompt();
}
void handler()
{
	signal( SIGINT, disp );	
	fflush( stdout );
}
int main()
{
	handler() ;
	Command::_currentCommand.prompt();
	yyparse();
	return 0;
}
