#include "map_reduce.h"

//Space to store the results for analysis map
struct Analysis analysis_space[NFILES];
//Space to store the results for stats map
Stats stats_space[NFILES];

//Sample Map function action: Print file contents to stdout and returns the number bytes in the file.
int cat(FILE* f, void* res, char* filename) {
    char c;
    int n = 0;
    printf("%s\n", filename);
    while((c = fgetc(f)) != EOF) {
        printf("%c", c);
        n++;
    }
    printf("\n");
    return n;
}

int main(int argc, char** argv) {
	int i, n, m, files;
	struct Analysis ana;
	Stats stat;	
	if ((i = validateargs(argc, argv)) < 0) {
		return EXIT_FAILURE;
	} else if (i == 0) {
		return EXIT_SUCCESS;
	} else {
		files = nfiles(argv[argc-1]);
		if (files == 0) {
			printf("No files present in the directory\n");
			return EXIT_SUCCESS;		
		}
		if (i == 1 || i == 3) {
			if ((n = map(argv[argc-1], analysis_space, sizeof(struct Analysis), analysis)) == -1) 	{
				return EXIT_FAILURE;
			}
			ana = analysis_reduce(files, analysis_space);
			if (i == 3) {
				for (m = 0; m < files; m++) {
					analysis_print(analysis_space[m], n, 0);			
				}
			}		
			analysis_print(ana, n, 1);
			return EXIT_SUCCESS;
		} else if (i == 2 || i == 4) {
			if ((n = map(argv[argc-1], stats_space, sizeof(Stats), stats)) == -1) 	{
				return EXIT_FAILURE;
			}	
			stat = stats_reduce(files, stats_space);
			if (i == 4) {
				for (m = 0; m < files; m++) {
					stats_print(stats_space[m], 0);
				}
			}
			stats_print(stat, 1);
			return EXIT_SUCCESS;
		}
	}
}


