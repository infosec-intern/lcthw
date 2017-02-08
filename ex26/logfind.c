#include <stdio.h>
#include <stdlib.h>			// getenv
#include <string.h>			// strtok, strncpy, strstr
#include <glob.h>			// glob
#include <unistd.h>			// getopt
#include <linux/limits.h>	// PATH_MAX
#include "dbg.h"			// debug, check, log_err

// an upper limit on glob patterns makes things easier for me
#define GLOB_MAX 10
// lines can only be LINE_LENGTH characters long before getting truncated
#define LINE_LENGTH 512
// an upper limit on the number of terms that can be searched
#define SEARCH_TERMS_MAX 5

int load_config(char*, char**);
int build_cli(int, char*[], int*, char***);
void search_files(char**, int, char**, int, int);


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
	char* tokenized;

	// resolve an absolute path to our config and open it for reading	
	realpath(config_path, resolved_path);
	logfind = fopen(resolved_path, "r");
	check(logfind != NULL, "Couldn't open .logfind");

	// begin reading
	while (fgets(buffer, LINE_LENGTH - 1, logfind) != NULL) {
		check(count < GLOB_MAX, "Maximum number of patterns exceeded. %d >= %d", count, GLOB_MAX);
		// remove \n characters from string
		tokenized = strtok(buffer, "\n");
		if (tokenized == NULL || tokenized[0] == '\0')
			continue;
		globs[count] = malloc(strlen(tokenized)*sizeof(char));
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
	if (logfind != NULL)
		fclose(logfind);

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
				if (count >= SEARCH_TERMS_MAX)
					break;
				terms[count] = malloc(strlen(optarg)*sizeof(char));
				strncpy(terms[count], optarg, strlen(optarg));
				count++;
				break;
		}
	}

	*terms_addr = terms;
	return count;
}

/* Search all files matching glob patterns for search term(s)
 * Number of terms is variable, and we need to search for each one
 *
 * Input
 * 		patterns: strings that match file pattern globs
 * 		pattern_count: length of patterns array
 * 		terms: array of search terms
 * 		term_count: length of terms array
 * 		or_flag: determines how to analyze search results. 1 == OR. 0 == AND
 */
void search_files(char** patterns, int pattern_count, char** terms, int term_count, int or_flag)
{
	int i, j, k;
	// describes if a file matches all (or one of) our pattern(s). 1 = true. 0 = false
	int match = -1;
	// current line number we are searching
	int line_no = 0;
	// result of glob()
	int result;
	FILE* fp = NULL;
	char* current_file = malloc(PATH_MAX*sizeof(char));
	char* current_pattern = malloc(PATH_MAX*sizeof(char));
	char* buffer = malloc(LINE_LENGTH*sizeof(char));
	glob_t current_glob;

	// work on each glob pattern
	for(i = 0; i < pattern_count; i++) {
		current_pattern = patterns[i];
		result = glob(current_pattern, GLOB_TILDE_CHECK | GLOB_ERR, NULL, &current_glob);
		if (result == GLOB_NOMATCH) {
			perror(current_pattern);
			continue;
		}
		check(result != GLOB_NOSPACE, "glob() ran out of memory!");
		check(result != GLOB_ABORTED, "glob() experienced a read error!");
		// work on each file now
		for (j = 0; j < current_glob.gl_pathc; j++) {
			current_file = current_glob.gl_pathv[j];
			fp = fopen(current_file, "r");
			if (fp == NULL) {
				perror(current_file);
				continue;
			}
			// begin searching file
			for (k = 0; k < term_count; k++) {
				while (fgets(buffer, LINE_LENGTH - 1, fp) != NULL) {
					line_no++;
					debug("strstr(buffer, \"%s\") != NULL", terms[k]);
					if (strstr(buffer, terms[k]) != NULL) {
						// with the break, the max number of matches 
						// on a file should be equal to term_count
						if (match == -1)
							match = 1;
						else
							match++;
						// break out because we now know the term is
						// somewhere in the file - no need to keep processing
						break;
					}
				}
				if (match == -1)
					match = 0;
			}

			// OR flag requires just one term to match
			// AND flag requires all terms to match
			if ((or_flag == 1 && match > 0) || (or_flag != 1 && match == term_count))
				printf("%s matches!\n", current_file);
			else
				printf("%s does not match!\n", current_file);

			// reset for the next file
			match = 0;
			k = 0;
			fclose(fp);
		}
		globfree(&current_glob);
	}

	free(current_file);
	free(current_pattern);
	free(buffer);
	return;

error:
	free(current_file);
	free(current_pattern);
	free(buffer);
	// there's no jmp into the 'error:' label between fopen() and fclose()
	// so there's no need to check if the file pointer is open
	return;
}

int main(int argc, char *argv[])
{
	int i = 0;
	int term_count = 0;
	int pattern_count = 0;
	int or_flag = 0;
	char* config_path = "/home/thomas/.logfind";
//	const char* config_path = "~/.logfind";
	char** patterns = malloc(GLOB_MAX*sizeof(char*));
	char** terms = malloc(sizeof(char**));

	term_count = build_cli(argc, argv, &or_flag, &terms);
	check(term_count > 0, "Usage: %s <term1> <term2> ...", argv[0]);

	pattern_count = load_config(config_path, patterns);
	check(pattern_count > 0, "No glob patterns loaded!");

	// summary
	debug("%s flag set", (or_flag == 1) ? "OR" : "AND");		// ternary, bitches
	debug("Found %d patterns in %s", pattern_count, config_path);
	debug("Found %d terms", term_count);

	// perform search
	search_files(patterns, pattern_count, terms, term_count, or_flag);

	for (i = 0; i < pattern_count; i++)
		debug("patterns[%d] @ %p = %s", i, patterns[i], patterns[i]);		
	for (i = 0; i < term_count; i++)
		debug("terms[%d] @ %p = %s", i, terms[i], terms[i]);

	// clean up
	for (i = 0; i < pattern_count; i++)
		free(patterns[i]);
	for (i = 0; i < term_count; i++)
		free(terms[i]);

	return 0;

error:
	for (i = 0; i < pattern_count; i++)
		free(patterns[i]);
	for (i = 0; i < term_count; i++)
		free(terms[i]);

	return 1;
}
