/* 
 * NAME: 	Zhenghao Li
 * EMAIL: 	lizhenghao99@g.ucla.edu
 * ID: 		704971934
 */

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<getopt.h>
#include<fcntl.h>
#include<errno.h>
#include<signal.h>

int copy(int input_fd, int output_fd)
{
	char buffer;
	while (read(input_fd, &buffer, sizeof(char)) == 1)
		write(output_fd, &buffer, sizeof(char));

	return 0;
}

void seg_handler()
{
	fprintf(stderr, "seg_handler(): segmentation fault caught!\n");
	exit(4);
}

int seg_cause()
{
	int *ptr = NULL;
	int n = *ptr;
	return n;
}

int main(int argc, char* argv[])
{	
	int option_index;

	int flag_in = 0;
	int flag_out = 0;
	
	struct option long_options[] = 
	{
		{"input", 	required_argument, 	0, 	'i'},
		{"output",	required_argument,	0,	'o'},
		{"segfault",no_argument,		0,	's'},
		{"catch",	no_argument,		0,	'c'},
		{"dump-core",no_argument,		0,	'd'},
		{0,			0,					0,	0}
	};

	int option = -1;
	char *inputpath = NULL;
	char *outputpath = NULL;


	while ((option = 
			getopt_long(argc, argv, "i:o:scd", long_options, &option_index)) != -1)
	{
		switch (option)
		{
			case 'i':
				inputpath = optarg;
				flag_in = 1;
				break;
			case 'o':
				outputpath = optarg;
				flag_out = 1;
				break;
			case 's':
				seg_cause();
				break;
			case 'c':
				signal(SIGSEGV, seg_handler);
				break;
			case 'd':
				signal(SIGSEGV, SIG_DFL);
				break;
			default:
				fprintf(stderr, "Usage: --input=filename --output=filename --segfault --catch --dump-core\n");
				exit(1);
		}
	}

	if (flag_in)
	{
		int fd = open(inputpath, O_RDONLY);
		if (fd != -1)
		{
			dup2(fd, 0);
			close(fd);
		}
		else
		{
			fprintf(stderr, "--input: unable to open file: %s\nerror: %s\n", inputpath, strerror(errno));
			exit(2);
		}
	}
	
	if (flag_out)
	{
		int fd = open(outputpath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (fd != -1)
		{
			dup2(fd, 1);
			close(fd);
		}
		else
		{
			fprintf(stderr, "--output: unable to write file: %s\nerror: %s\n", outputpath, strerror(errno));
			exit(3);
		}
	}

	copy(0,1);	

	exit(0);
}
