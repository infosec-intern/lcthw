#include <stdio.h>
#include <stdlib.h>
#include <string.h>			// strtok, strncpy
#include <glob.h>			// glob
#include <unistd.h>			// getopt
#include "dbg.h"			// debug, check, log_err

// going off of FAT filesystem paths -- modern systems can handle longer paths
#define PATH_MAX 256
// an upper limit on glob patterns makes things easier for me
#define GLOB_MAX 10
// lines can only be LINE_LENGTH characters long before getting truncated
#define LINE_LENGTH 100
// an upper limit on the number of terms that can be searched
#define SEARCH_TERMS_MAX 5

int load_config(char*, char**);
int gen_config(void);
int build_cli(int, char*[], int*);


/* Load a configuration file from ~/.logfind
 * The file should contain globs of files to search
 * (e.g. *.c, Makefile, lib.h)
 *
 * Input
 *		config: A file path to read from
 *		globs: Pointer to a list of globs
 * Output
 * 		count: Number of lines found in config file
 */
int load_config(char* config_path, char** globs)
{
	int count = 0;
	FILE* logfind = NULL;
	char* resolved_path	= malloc(PATH_MAX*sizeof(char));
	char* buffer = malloc(LINE_LENGTH*sizeof(char));
	char* tokenized = malloc(LINE_LENGTH*sizeof(char));

	// resolve an absolute path to our config and open it for reading	
	realpath(config_path, resolved_path);
	debug("Config file found at \"%s\"", resolved_path);
	logfind = fopen(resolved_path, "r");
	check(logfind != NULL, "Couldn't open .logfind");

	// begin reading
	while (fgets(buffer, LINE_LENGTH - 1, logfind) != NULL) {
		check(count < GLOB_MAX, "Maximum number of patterns exceeded. %d >= %d", count, GLOB_MAX);
		// remove \n characters from string
		tokenized = strtok(buffer, "\n");
		globs[count] = malloc(strlen(tokenized)*sizeof(char));
		debug("copying %s into globs[%d]", tokenized, count);
		strncpy(globs[count], tokenized, strlen(tokenized));
		count++;
	}

	// clean up
	free(buffer);
	free(resolved_path);
	fclose(logfind);

	// return the number of globs found in the config file
	return count;

error:
	free(buffer);
	free(resolved_path);
	if (logfind != NULL) {
		fclose(logfind);
	}
	return -1;
}

/* Generate a basic configuration file at ~/.logfind
 * Only include some basic glob patterns
 *
 * Output
 * 		error: 0 means the function was successful. Anything else is an error
 */
int gen_config(void)
{
	FILE* logfind = NULL;
	char* c_glob = "*.c";
	char* h_glob = "*.h";
	char* makefile = "Makefile";

	// open config file for writing
	logfind = fopen("/home/thomas/.logfind", "w+b");
	check(logfind != NULL, "Couldn't open .logfind");

	// write some basic glob patterns
	fprintf(logfind, "%s\n", c_glob);
	fprintf(logfind, "%s\n", h_glob);
	fprintf(logfind, "%s\n", makefile);

	// clean up
	fclose(logfind);
	return 0;

error:
	c_glob = NULL;
	h_glob = NULL;
	makefile = NULL;
	if (logfind != NULL) {
		fclose(logfind);
	}
	return -1;
}

/* Parse command line arguments for search terms
 * Takes any sequence of words and applies "and" to them
 * Allow the option to "or" words with a -o flag
 */
int build_cli(int argc, char* argv[])
{
	if (argc < 2)
		check(error == 0, "Usage: %s <term1> <term2> ...", argv[0]);

	int i = 0;
	char* terms = malloc(SEARCH_TERMS_MAX*sizeof(char*));
	int opt;
	int length;

	// examine each argument looking for our "OR" flag
	while((opt = getopt(argc, argv, "-o")) != -1) {
		debug("Option: %s", optarg);
		switch(opt) {
			case 'o':
				debug("Found -o flag! %c in argv[%d]", (char)opt, optind-1);
				*or_flag = 1;
				break;
			// treat any non-flag argument as a term to search
			default:
				length = strlen(optarg)+1;
				terms[i] = malloc(length*sizeof(char));
				strncpy(terms[i], optarg, length);
				i++;
				break;
		}
	}

	return 0;

error:
	for (i=0; i<SEARCH_TERMS_MAX; i++) {
		debug("Freeing memory for terms[%d] = %s", i, terms[i]);
		if (terms[i] != NULL)
			free(terms[i]);
	}
	return 1;
}

int main(int argc, char *argv[])
{
	int i = 0;
	int error = -1;
	int count = 0;
	int or_flag;
	char** globs = malloc(GLOB_MAX*sizeof(char*));
	char** terms = malloc(sizeof(char**));
	char* config_path = "/home/thomas/.logfind";

	error = build_cli(argc, argv, &or_flag, terms);

//	error = gen_config();
	count = load_config(config_path, globs);
	check(count > 0, "No glob patterns loaded!");

	for(i = 0; i < count; i++) 
		debug("glob[%d] = %s", i, globs[i]);
	log_info("Found %d globs in %s", count, config_path);
	
	// clean up
	for(i = 0; i < count; i++) 
		free(globs[i]);
	return 0;

error:
	for(i = 0; i < count; i++)
		free(globs[i]);
	return 1;
}
