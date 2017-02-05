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
int build_cli(int, char*[], int*, char***);
int expand_globs(char**, int, glob_t*);


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

/* Convert config patterns into filesystem globs
 *
 * Input
 * 		patterns: an array of pattern strings to convert
 * 		psize: length of patterns array. helps in for loops
 * 		glob_ptr: pointer to an empty array. Will point to our array of globs we create
 * Output
 * 		count: number of globs created
 */
int expand_globs(char** patterns, int psize, glob_t* globs)
{
	int i = 0;
	int count = 0;
	int result;
//	glob_t* globs = malloc(GLOB_MAX*sizeof(glob_t));

	for(i = 0; i < psize; i++) {
		debug("pattern[%d] = %s", i, patterns[i]);
		result = glob(patterns[i], GLOB_TILDE_CHECK, NULL, &globs[i]);
		if (result == GLOB_NOMATCH) {
			log_err("No matches for %s found. Moving on", patterns[i]);
			continue;
		}
		check(result != GLOB_NOSPACE, "glob() ran out of memory!");
		check(result != GLOB_ABORTED, "glob() experienced a read error!");
		// only bump glob count if all checks are passed
		count++;
	}

	debug("Counted %d globs");
	debug("[2] globs* located at %p", globs);

	return count;

error:
	for(i = 0; i < count; i++) {
		debug("freeing glob %d @ %p", i, &globs[i]);
		globfree(&globs[i]);
	}
	return -1;
}

int main(int argc, char *argv[])
{
	int i = 0;
	int term_count = 0;
	int pattern_count = 0;
	int glob_count = 0;
	int or_flag = 0;
	char* config_path = "/home/thomas/.logfind";
	char** patterns = malloc(GLOB_MAX*sizeof(char*));
	char** terms = malloc(sizeof(char**));
	glob_t* globs = malloc(GLOB_MAX*sizeof(glob_t));
//	const char* config_path = "~/.logfind";

	// meat and potatoes
	term_count = build_cli(argc, argv, &or_flag, &terms);
	check(term_count > 0, "Usage: %s <term1> <term2> ...", argv[0]);

	pattern_count = load_config(config_path, patterns);
	check(pattern_count > 0, "No glob patterns loaded!");

	glob_count = expand_globs(patterns, pattern_count, &globs);
	debug("[1] globs* located at %p", *globs);
	check(glob_count == pattern_count, "Some globs could not be created!");
	debug("[3] globs* located at %p", *globs);

	// summary
	log_info("Found %d patterns in %s", pattern_count, config_path);
	log_info("Found %d terms", term_count);
	log_info("Generated %d globs", glob_count);
	
	// clean up
	for(i = 0; i < pattern_count; i++) 
		free(patterns[i]);
	for(i = 0; i < term_count; i++)
		free(terms[i]);
	for(i = 0; i < glob_count; i++) {
		debug("freeing glob %d @ %p", i, globs[i]);
		globfree(&globs[i]);
	}

	return 0;

error:
	for(i = 0; i < pattern_count; i++)
		free(patterns[i]);
	for(i = 0; i < term_count; i++)
		free(terms[i]);
	for(i = 0; i < glob_count; i++) {
		debug("freeing glob %d @ %p", i, globs[i]);
		globfree(&globs[i]);
	}

	return 1;
}
