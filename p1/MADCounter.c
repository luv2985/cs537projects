#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

void counting(int argc, char * argv[]);

struct charlist {
	int ascii;
    char name;
    int occurrences;
    int initpos;
};

struct wordlist {
    char * name;
    int occurrences;
    int initpos;
    struct wordlist * next;
};

struct linelist {
    char * name;
    int occurrences;
    int initpos;
    struct linelist * next;
};

void clearLineList(struct linelist * head);
void clearWordList(struct wordlist * head);

// MAIN FUNCTION
int main (int argc, char * argv[]) 
{

	const int batch_line_size = 256;
	
	// USAGE ERRORS
	if (argc < 3)
	{
		printf("USAGE:\n\t./MADCounter -f <input file> -o <output file> -c -w -l -Lw -Ll\n\t\tOR\n\t./MADCounter -B <batch file>\n");
		return 1;
	}

	if (strcmp(argv[1], "-B") == 0)
	{
		// enter batchmode to read a txt
		FILE * file = NULL;
		char line[batch_line_size];
		
		file = fopen(argv[2], "r");
		
		if (file == NULL)
		{
			printf("ERROR: Can't open batch file\n");
			return 1;
		}

		long currentPosition = ftell(file);
		fseek(file, 0, SEEK_END);
		long fileSize = ftell(file);
		fseek(file, currentPosition, SEEK_SET);
		if (fileSize == 0)
		{
			printf("ERROR: Batch File Empty\n");
			return 1;
		}
		
		while (fgets(line, batch_line_size, file) != NULL)
		{
			int count = 0;
			char * argv_[10] = {"first_element"};
			char * tok = strtok(line, " ");
			while (tok != NULL)
			{
				count++;
				argv_[count] = tok;
				tok = strtok(NULL, " ");
			}
			argv_[count][strlen(argv_[count]) - 1] = '\0';
			counting(count + 1, argv_);
		}
	}
	else
	{
		// run counting once
		counting(argc, argv);
	}
}

// SINGLE CALL
void counting (int argc, char* argv[])
{
	int o_flag = 0; // 0 is no, 1 is yes
	char * input_filename = NULL;
	char * output_filename = NULL;
	// CHECK FLAGS
	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-f") == 0)
		{
			input_filename = argv[i+1];
		}
		else if (strcmp(argv[i], "-o") == 0)
		{
			o_flag = 1;
			output_filename = argv[i+1];
		}
	}
	if (input_filename == NULL || input_filename[0] == '-')
	{
		printf("ERROR: No Input File Provided\n");
		exit(1);
	}

	if (o_flag == 1 && (output_filename == NULL || output_filename[0] == '-'))
	{
		printf("ERROR: No Output File Provided\n");
		exit(1);
	}

	// TRACKERS
	// track curr line, size of line
	int line_alloc = 256;
	char * curr_ln = (char *)malloc(line_alloc * sizeof(char));
	curr_ln[0] = '\0';
	// track curr word, size of word
	int word_alloc = 256;
	char * curr_wd = (char *)malloc(word_alloc * sizeof(char));
	curr_wd[0] = '\0';
	// list of unique words
	// list of unique characters
	// list of all words
	// list of all characters

	// CHARACTER TRACKING
	int total_chars = 0;
	int unique_chars = 0;
	struct charlist all_char[128];
	
	for (int i = 0; i < 128; ++i) {
		all_char[i].ascii = i;
		all_char[i].name = i;
	    all_char[i].occurrences = 0;
	    all_char[i].initpos = INT_MAX;
	}
    
	
	// LINE TRACKING
	int total_lines = 0;
	int unique_lines = 0;
	struct linelist * startline = NULL;
	int lmax = 0;

	
	// WORD TRACKING
	int total_words = 0;
	int unique_words = 0;
	struct wordlist * startword = NULL;
	int wmax = 0;


	// OPEN FILE
	FILE *file = NULL;
	char thisChar;

	file = fopen(input_filename, "r");

	if (file == NULL)
	{
		printf("ERROR: Can't open input file\n");
		exit(1);
	}

	long currentPosition = ftell(file);
	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	fseek(file, currentPosition, SEEK_SET);
	if (fileSize == 0)
	{
		printf("ERROR: Input File Empty\n");
		exit(1);
	}

	int curr_pos = 0;
	// LOOP THROUGH THE FILE
	while ((thisChar = fgetc(file)) != EOF)
	{
		// CHECK CHAR occurrences and initial position
		int thisASCII = thisChar;
		all_char[thisASCII].occurrences++;
		if (curr_pos < all_char[thisASCII].initpos)
		{
			all_char[thisASCII].initpos = curr_pos;
		}
		
		total_chars++;
		if (all_char[thisASCII].occurrences == 1)
		{
			unique_chars++;
		}
	
		// RESIZE TRACKERS
		int ll = strlen(curr_ln);
		
		int wl = strlen(curr_wd);
		
		// LINE
		if (thisChar == '\n')
		{
			// ADD LINE TO LINELIST
			char * ln = strdup(curr_ln);
			int new = 1; // 1 is true
			struct linelist * iter_line = startline;
			struct linelist * prev_line = startline;
			while(iter_line != NULL)
			{
				if (strcmp(iter_line -> name, ln) == 0)
				{
					iter_line -> occurrences++;
					new = 0;
					total_lines++;
				}
				prev_line = iter_line;
				iter_line = iter_line -> next;
			}
			// if the line is new
			if (new == 1)
			{
				total_lines++;
				unique_lines++;
				struct linelist * newline = (struct linelist*)malloc(sizeof(struct linelist));
				newline -> name = ln;
				newline -> occurrences = 1;
				newline -> initpos = total_lines - 1;
				newline -> next = NULL;
				if (prev_line == NULL)
				{
					startline = newline;// the first element
				}
				else
				{
					struct linelist * iline = startline;
					struct linelist * pline = startline;
					int done_line_add = 0;
					while(iline != NULL && done_line_add == 0)
					{
						if (strcmp(iline -> name, newline -> name) < 0)
						{
							pline = iline;
							iline = iline -> next;
						}
						else if (iline == startline)
						{
							// reassigning startword
							newline -> next = startline;
							startline = newline;
							done_line_add = 1;
						}
						else	
						{
							pline -> next = newline;
							newline -> next = iline;
							done_line_add = 1;
						}
					}
					if (done_line_add == 0)
						pline->next = newline;
				}
			}
			// FIND LONGEST LINE CNT
			if (ll > lmax)
				lmax = ll;
			// RESET CURR LINE
			curr_ln[0] = '\0';
		}
		else // updates current line tracking
		{
			curr_ln[ll] = thisChar;
			curr_ln[ll + 1] = '\0';
		}

		// WORD
		if (thisChar == ' ' || thisChar == '\n')
		{
			// ADD WORD TO WORDLIST
			char * wd = strdup(curr_wd);
			int new = 1; // 1 is true
			struct wordlist * iter_word = startword;
			struct wordlist * prev_word = startword;
			while(iter_word != NULL)
			{
				if (strcmp(iter_word -> name, wd) == 0)
				{
					iter_word -> occurrences++;
					new = 0;
					total_words++;
				}
				prev_word = iter_word;
				iter_word = iter_word -> next;
			}
			// if the word is new
			if (new == 1 && wl > 0)
			{
				total_words++;
				unique_words++;
				struct wordlist * newword = (struct wordlist*)malloc(sizeof(struct wordlist));
				newword -> name = wd;
				newword -> occurrences = 1;
				newword -> initpos = total_words - 1;
				newword -> next = NULL;
				if (prev_word == NULL)
				{
					startword = newword;// the first element
				}
				else
				{
					struct wordlist * iword = startword;
					struct wordlist * pword = startword;
					int done_word_add = 0;
					while(iword != NULL && done_word_add == 0)
					{
						if (strcmp(iword -> name, newword -> name) < 0)
						{
							pword = iword;
							iword = iword -> next;
						}
						else if (iword == startword)
						{
							// reassigning startword
							newword -> next = startword;
							startword = newword;
							done_word_add = 1;
						}
						else	
						{
							pword -> next = newword;
							newword -> next = iword;
							done_word_add = 1;
						}
					}
					if (done_word_add == 0)
						pword->next = newword;
				}
			}
			
			if (wl > wmax)
				wmax = wl;
			curr_wd[0] = '\0';
		}
		else // updates current word tracking
		{
			curr_wd[wl] = thisChar;
			curr_wd[wl + 1] = '\0';
		}
		curr_pos++;
	}
	
	int spaces = 0;
	for (int i = 1; i < argc; i++)
	{
		char * flag = argv[i];
		if (strcmp(flag, "-f") == 0 ||
			strcmp(flag, "-o") == 0)
		{
			i++;
		}
		else if (strcmp(flag, "-c") == 0 ||
				strcmp(flag, "-w") == 0 ||
				strcmp(flag, "-l") == 0 ||
				strcmp(flag, "-Lw") == 0 ||
				strcmp(flag, "-Ll") == 0)
		{
			spaces++;
		}
		else // ERROR
		{
			printf("ERROR: Invalid Flag Types\n");
			exit(1);
		}
	}

	
	// loop through flags, to either print or store
	FILE * output;
	if (o_flag == 1)
	{
		output = fopen(output_filename, "w");
	}

	int skip = 1;
	for (int i = 1; i < argc; i++)
	{
		char * flag = argv[i];

		if (strcmp(flag, "-f") == 0 || strcmp(flag, "-o") == 0)
		{
			skip = 0;
			i++;
		}
		else if (strcmp(flag, "-c") == 0 && o_flag == 0) // CHARACTER
		{
			skip = 1;
			printf("Total Number of Chars = %i\nTotal Unique Chars = %i\n\n", total_chars, unique_chars);
			for (int j = 0; j < 128; j++)
			{
				if (all_char[j].occurrences != 0)
				{
					printf("Ascii Value: %i, Char: %c, Count: %i, Initial Position: %i\n", 
							all_char[j].ascii, 
							all_char[j].name,
							all_char[j].occurrences, 
							all_char[j].initpos);
				}
			}
		}
		else if (strcmp(flag, "-c") == 0 && o_flag == 1)
		{
			skip = 1;
			fprintf(output, "Total Number of Chars = %i\nTotal Unique Chars = %i\n\n", total_chars, unique_chars);
			for (int j = 0; j < 128; j++)
			{
				if (all_char[j].occurrences != 0)
				{
					fprintf(output, "Ascii Value: %i, Char: %c, Count: %i, Initial Position: %i\n", 
							all_char[j].ascii, 
							all_char[j].name,
							all_char[j].occurrences, 
							all_char[j].initpos);
				}
			}
		}
		else if (strcmp(flag, "-w") == 0 && o_flag == 0) // WORD
		{
			skip = 1;
			printf("Total Number of Words: %i\nTotal Unique Words: %i\n\n", total_words, unique_words);
			struct wordlist * iterword = startword;
			while (iterword != NULL)
			{
				printf("Word: %s, Freq: %i, Initial Position: %i\n", 
						iterword -> name,
						iterword -> occurrences, 
						iterword -> initpos);
				iterword = iterword -> next;
			}
		}
		else if (strcmp(flag, "-w") == 0 && o_flag == 1)
		{
			skip = 1;
			fprintf(output, "Total Number of Words: %i\nTotal Unique Words: %i\n\n", total_words, unique_words);
			struct wordlist * iterword = startword;
			while (iterword != NULL)
			{
				fprintf(output, "Word: %s, Freq: %i, Initial Position: %i\n", 
						iterword -> name,
						iterword -> occurrences, 
						iterword -> initpos);
				iterword = iterword -> next;
			}
		}
		else if (strcmp(flag, "-l") == 0 && o_flag == 0) // LINE
		{
			skip = 1;
			printf("Total Number of Lines: %i\nTotal Unique Lines: %i\n\n", total_lines, unique_lines);
			struct linelist * iterline = startline;
			while (iterline != NULL)
			{
				printf("Line: %s, Freq: %i, Initial Position: %i\n", 
						iterline -> name,
						iterline -> occurrences, 
						iterline -> initpos);
				iterline = iterline -> next;
			}
		}
		else if (strcmp(flag, "-l") == 0 && o_flag == 1)
		{
			skip = 1;
			fprintf(output, "Total Number of Lines: %i\nTotal Unique Lines: %i\n\n", total_lines, unique_lines);
			struct linelist * iterline = startline;
			while (iterline != NULL)
			{
				fprintf(output, "Line: %s, Freq: %i, Initial Position: %i\n", 
						iterline -> name,
						iterline -> occurrences, 
						iterline -> initpos);
				iterline = iterline -> next;
			}
		}
		else if (strcmp(flag, "-Lw") == 0 && o_flag == 0) // LONGEST WORD
		{
			skip = 1;
			printf("Longest Word is %i characters long:\n", wmax);
			struct wordlist * iterword = startword;
			while (iterword != NULL)
			{
				if (strlen(iterword -> name) == wmax)
				{
					printf("\t%s\n", iterword -> name);
				}
				iterword = iterword -> next;
			}
		}
		else if (strcmp(flag, "-Lw") == 0 && o_flag == 1)
		{
			skip = 1;
			fprintf(output, "Longest Word is %i characters long:\n", wmax);
			struct wordlist * iterword = startword;
			while (iterword != NULL)
			{
				if (strlen(iterword -> name) == wmax)
				{
					fprintf(output, "\t%s\n", iterword -> name);
				}
				iterword = iterword -> next;
			}
		}
		else if (strcmp(flag, "-Ll") == 0 && o_flag == 0) // LONGEST LINE
		{
			skip = 1;
			printf("Longest Line is %i characters long:\n", lmax);
			struct linelist * iterline = startline;
			while (iterline != NULL)
			{
				if (strlen(iterline -> name) == lmax)
				{
					printf("\t%s\n", iterline -> name);
				}
				iterline = iterline -> next;
			}
		}
		else if (strcmp(flag, "-Ll") == 0 && o_flag == 1)
		{
			skip = 1;
			fprintf(output, "Longest Line is %i characters long:\n", lmax);
			struct linelist * iterline = startline;
			while (iterline != NULL)
			{
				if (strlen(iterline -> name) == lmax)
				{
					fprintf(output, "\t%s\n", iterline -> name);
				}
				iterline = iterline -> next;
			}
		}

		if (skip == 1 && i + 1 != argc && o_flag == 0 && spaces > 1)
		{
			printf("\n");
			spaces--;
		}
		else if (skip == 1 && i + 1 != argc && o_flag == 1 && spaces > 1)
		{
			fprintf(output, "\n");
			spaces--;
		}
	}
	
	fclose(file);
	clearLineList(startline);
	clearWordList(startword);
	if (curr_ln != NULL) {
	    free(curr_ln);
	}
	if (curr_wd != NULL) {
		free(curr_wd);
	}
}

void clearLineList(struct linelist * head)
{
    struct linelist * current = head;
    struct linelist * next;

    while (current != NULL)
    {
        next = current->next;
        free(current);
        current = next;
    }
}

void clearWordList(struct wordlist * head)
{
    struct wordlist * current = head;
    struct wordlist * next;

    while (current != NULL)
    {
        next = current->next;
        free(current);
        current = next;
    }
}
