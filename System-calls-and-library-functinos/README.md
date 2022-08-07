# Depth-First search, traversing thru directories
Instruction on how to run program "dt" for depth-first search:
	Make sure dt.c and Makefile are in the same folder
	On your terminal execute "make", it will produce file called "dt"
	You can run "dt" by following "./dt"
Since this program is the utility to traverse a specified or default (current) current directory,
it comes with multiple options and it can be executed by following:

		dt [-h] [-I n] [-L -d -g -i -p -s -t -u | -l] [dirname] 

[] stands for optional, it can be added or left.
The options are to be interpreted as follows:
	
	h Print a help message and exit.
	I n Change indentation to n spaces for each level.
	L Follow symbolic links, if any. Default will be to not follow symbolic links.
	t Print information on file type.
	p Print permission bits as rwxrwxrwx.
	i Print the number of links to file in inode table.
	u Print the UID associated with the file.
	g Print the GID associated with the file.
	s Print the size of file.
	d Show the time of last modification
	l This option will be used to print information on the file as if the options [t p i u g s] are all specified

I hope you enjoy this utility!
