#include <stdio.h>
#include <stdlib.h>			// getenv
#include <string.h>			// strtok, strncpy
#include <glob.h>			// glob
#include <unistd.h>			// getopt
#include <linux/limits.h>	// PATH_MAX
#include "dbg.h"			// debug, check, log_err

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
int build_cli(int argc, char* argv[], int* or_flag)
{
	if (argc < 2)
		return 1;

	int i = 0;
	char** terms = malloc(SEARCH_TERMS_MAX*sizeof(char**));
	int opt;

	// examine each argument looking for our "OR" flag
	while((opt = getopt(argc, argv, "-o")) != -1) {
		switch(opt) {
			case 'o':
				*or_flag = 1;
				break;
			// treat any non-flag argument as a term to search
			default:
				if (i >= SEARCH_TERMS_MAX) {
					debug("Maximum search terms reached!. Skipping \"%s\" @ argv[%d]", optarg, optind-1);
					break;
				}
				terms[i] = malloc(strlen(optarg)*sizeof(char));
				strncpy(terms[i], optarg, strlen(optarg));
				i++;
				break;
		}
	}

	for (i=0; i<SEARCH_TERMS_MAX; i++)
		debug("term[%d] = %s @ %p", i, terms[i], terms[i]);

	return 0;

error:
	if (terms != NULL) {
		debug("Freeing up terms @ %p", terms);
		for (i=0; i<SEARCH_TERMS_MAX; i++) {
			if (terms[i] != NULL) {
				debug("Freeing memory for terms[%d] = %s @ %p", i, terms[i], terms[i]);
				free(terms[i]);
			}
		}
	}
	return 1;
}

int main(int argc, char *argv[])
{
	int i = 0;
	int error = -1;
	int count = 0;
	int or_flag = 0;
	char** globs = malloc(GLOB_MAX*sizeof(char*));
	char** terms = malloc(sizeof(char**));
	char* config_path = "/home/thomas/.logfind";
//	const char* config_path = "~/.logfind";

	error = build_cli(argc, argv, &or_flag);
	check(error == 0, "Usage: %s <term1> <term2> ...", argv[0]);

	if (or_flag == 1)
		debug("OR flag set!");
	else
		debug("AND flag set!");

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
