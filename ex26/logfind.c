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
int build_cli(int, char*[], int*, char***);


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
//	debug("Config file found at \"%s\"", resolved_path);
	logfind = fopen(resolved_path, "r");
	check(logfind != NULL, "Couldn't open .logfind");

	// begin reading
	while (fgets(buffer, LINE_LENGTH - 1, logfind) != NULL) {
		check(count < GLOB_MAX, "Maximum number of patterns exceeded. %d >= %d", count, GLOB_MAX);
		// remove \n characters from string
		tokenized = strtok(buffer, "\n");
		globs[count] = malloc(strlen(tokenized)*sizeof(char));
//		debug("copying %s into globs[%d]", tokenized, count);
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

/* Parse command line arguments for search terms
 * Takes any sequence of words and applies "and" to them
 * Allow the option to "or" words with a -o flag
 *
 * Input
 * 		argc: same as in main
 * 		argv: same as in main
 *		or_flag: address to store OR flag value in (1 for OR, 0 for AND)
 *		terms_addr: address to store terms string array in
 *	Output
 *		error: any errors returned. 0 means the function ran successfully
 */
int build_cli(int argc, char* argv[], int* or_flag, char*** terms_addr)
{
	if (argc < 2)
		return -1;

	int count = 0;
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
				if (count >= SEARCH_TERMS_MAX) {
//					debug("Maximum search terms reached!. Skipping \"%s\" @ argv[%d]", optarg, optind-1);
					break;
				}
				terms[count] = malloc(strlen(optarg)*sizeof(char));
				strncpy(terms[count], optarg, strlen(optarg));
				count++;
				break;
		}
	}

	*terms_addr = terms;
	return count;
}

int main(int argc, char *argv[])
{
	int i = 0;
	int term_count = 0;
	int glob_count = 0;
	int or_flag = 0;
	char** globs = malloc(GLOB_MAX*sizeof(char*));
	char** terms = malloc(sizeof(char**));
	char* config_path = "/home/thomas/.logfind";
//	const char* config_path = "~/.logfind";

	term_count = build_cli(argc, argv, &or_flag, &terms);
	check(term_count > 0, "Usage: %s <term1> <term2> ...", argv[0]);

	if (or_flag == 1)
		debug("OR flag set!");
	else
		debug("AND flag set!");

	glob_count = load_config(config_path, globs);
	check(glob_count > 0, "No glob patterns loaded!");

	for(i = 0; i < term_count; i++) 
		debug("term[%d] = %s", i, terms[i]);
	for(i = 0; i < glob_count; i++) 
		debug("glob[%d] = %s", i, globs[i]);

	// summary
	log_info("Found %d globs in %s", glob_count, config_path);
	log_info("Found %d terms", term_count);
	
	// clean up
	for(i = 0; i < glob_count; i++) 
		free(globs[i]);
	for(i = 0; i < term_count; i++)
		free(terms[i]);

	return 0;

error:
	for(i = 0; i < glob_count; i++)
		free(globs[i]);

	for(i = 0; i < term_count; i++)
		free(terms[i]);

	return 1;
}
