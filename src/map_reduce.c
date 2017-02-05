//**DO NOT** CHANGE THE PROTOTYPES FOR THE FUNCTIONS GIVEN TO YOU. WE TEST EACH
//FUNCTION INDEPENDENTLY WITH OUR OWN MAIN PROGRAM.
#include "map_reduce.h"

//Implement map_reduce.h functions here.
int validateargs(int argc, char** argv) {
	int i, vflag = 0, ana = 0, stats = 0, d = 0;
	DIR* dir;
	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (argv[i][1] == 'h') {
				usage();
				return 0;
			}
			else if (i == 1 && argv[i][1] == 'v') {
				vflag = 1;
			} else {
				usage();
				return -1;
			}
		} else if (strcmp(argv[i], "ana") == 0 && ana == 0 && stats == 0) {
			ana = 1;
		} else if (strcmp(argv[i], "stats") == 0 && ana == 0 && stats == 0) {
			stats = 1;
		} else if (ana + stats == 1 && d == 0) {
			if ((dir = opendir(argv[i]))) {
				d = 1;
				closedir(dir);
			} else {
				usage();
				return -1;
			}
		} else {
			usage();
			return -1;
		}
	}
	if (d == 1) {
		if (ana == 1 && vflag == 0) {
			return 1;
		} else if (stats == 1 && vflag == 0) {
			return 2;
		} else if (ana == 1 && vflag == 1) {
			return 3;
		} else if (stats == 1 && vflag == 1) {
			return 4;
		}
	} 
	usage();
	return -1;
}

int nfiles(char* dir) {
	int files = 0;
	DIR* dirp;
	struct dirent* di;
	dirp = opendir(dir);
	while ((di = readdir(dirp)) != NULL) {
		if ((strcmp(di->d_name, "..")) != 0 && (strcmp(di->d_name, ".")) != 0) {
			files++;
		}
	}
	closedir(dirp);
	return files;
}

int map(char* dir, void* results, size_t size, int (*act)(FILE* f, void* res, char* fn)) {
	int res, sum = 0;
	DIR* dirp;
	struct dirent* di;
	char filepath[4097], *filename;
	FILE* file;
	dirp = opendir(dir);
	memset(results, 0, size);
	while ((di = readdir(dirp)) != NULL) {
		if ((strcmp(di->d_name, "..")) == 0 || (strcmp(di->d_name, ".")) == 0) {
			continue;
		}
		if (strlen(dir) + strlen(di->d_name) > 4095) {
			return -1;
		}
		filepath[0] = 0;
		strcat(filepath, dir);
		strcat(filepath, "/");
		strcat(filepath, di->d_name);
		if ((file = fopen(filepath, "r")) == NULL) {
			return -1;
		}
		filename = strdup(di->d_name);
		if ((res = act(file, results, filename)) == -1) {
			return -1;		
		} 
		fclose(file);
		sum += res;
		results += size;
	}
	closedir(dirp);
	return sum;
}

struct Analysis analysis_reduce(int n, void* results) {
	int i, x, blen = 0;
	struct Analysis total, *result;
	memset(total.ascii, 0, 128*sizeof(int));
	total.lnlen = 0;
	total.lnno = 0;
	for (i = 0; i < n; i++) {
		result = (struct Analysis*)results;
		for (x = 0; x < 128; x++) {
			total.ascii[x] += result->ascii[x];
		}
		if (result->lnlen > blen) {
			blen = result->lnlen;
			total.filename = result->filename;
			total.lnlen = result->lnlen;
			total.lnno = result->lnno;
		}
		results += sizeof(struct Analysis);
	}
	return total;
}

Stats stats_reduce(int n, void* results) {
	int i, x;
	Stats total, *result;
	memset(total.histogram, 0, NVAL*4);
	total.sum = 0;
	total.n = 0;
	total.filename = NULL;
	for (i = 0; i < n; i++) {
		result = (Stats*)results;
		for (x = 0; x < NVAL; x++) {
			total.histogram[x] += result->histogram[x];
		}
		total.sum += result->sum;
		total.n += result->n;
		results += sizeof(Stats);
	}
	return total;
}

void analysis_print(struct Analysis res, int nbytes, int hist) {
	int i, n;
	printf("File: %s\n", res.filename);
	printf("Longest line length: %d\n", res.lnlen);
	printf("Longest line number: %d\n", res.lnno);
	if (hist != 0) {
		printf("Total Bytes in directory: %d\n", nbytes);
		printf("Histogram:\n");
		for (i = 0; i < 128; i++) {
			if (res.ascii[i] > 0) {
				printf("%d:", i);
				for (n = 0; n < res.ascii[i]; n++) {
					printf("-");			
				} 
				printf("\n");		
			}
		}	
	}
	if (hist == 0) {
		printf("\n");
	}
}

void stats_print(Stats res, int hist) {
	int i, n, min, max, maxocc = 0, q1, q2, q3, x1, x2, next;
	double np25, np50, np75;	
	if (hist != 0) {
		printf("Histogram:\n");
		for (i = 0; i < NVAL; i++) {
			if (res.histogram[i] > 0) {
				printf("%d :", i);
				for (n = 0; n < res.histogram[i]; n++) {
					printf("-");			
				} 
				printf("\n");				
			}		
		}
		printf("\n");
	}
	if (hist == 0) {
		printf("File: %s\n", res.filename);
	}
	for (i = 0; i < NVAL; i++) {
		if (res.histogram[i] > 0) {
			min = i;
			break;
		}	
	}
	for (i = NVAL-1; i >= 0; i--) {
		if (res.histogram[i] > 0) {
			max = i;
			break;
		}	
	}
	for (i = 0; i < NVAL; i++) {
		if (res.histogram[i] > maxocc) {
			maxocc = res.histogram[i];
		}	
	}
	np25 = .25 * res.n;
	np50 = .5 * res.n;
	np75 = .75 * res.n;
	q1 = (int)np25;
	if ((double)q1 != np25) {
		q1++;
	} 
	q2 = (int)np50;
	if ((double)q2 != np50) {
		q2++;
	}
	q3 = (int)np75;
	if ((double)q3 != np75) {
		q3++;
	}
	next = 0;
	for (i = 0; i < NVAL; i++) {
		if (res.histogram[i] > 0) {
			if (next == 1) {
				x2 = i;
				np25 = (double)((x1+x2)/2);
				break;
			}
			q1 -= res.histogram[i];
			if (q1 < 0) {
				np25 = (double)i;
				break;
			} else if (q1 == 0) {
				x1 = i;		
				next = 1;
			}
		}
	}
	next = 0;
	for (i = 0; i < NVAL; i++) {
		if (res.histogram[i] > 0) {
			if (next == 1) {
				x2 = i;
				np50 = (double)((x1+x2)/2);
				break;
			}
			q2 -= res.histogram[i];
			if (q2 < 0) {
				np50 = (double)i;
				break;
			} else if (q2 == 0) {
				x1 = i;		
				next = 1;
			}
		}
	}
	next = 0;
	for (i = 0; i < NVAL; i++) {
		if (res.histogram[i] > 0) {
			if (next == 1) {
				x2 = i;
				np75 = (double)((x1+x2)/2);
				break;
			}
			q3 -= res.histogram[i];
			if (q3 < 0) {
				np75 = (double)i;
				break;
			} else if (q3 == 0) {
				x1 = i;		
				next = 1;
			}
		}
	}
	printf("Count: %d\n", res.n);
	printf("Mean: %f\n", (double)res.sum/res.n);
	printf("Mode:");
	for (i = 0; i < NVAL; i++) {
		if (res.histogram[i] == maxocc) {
			printf(" %d", i);	
		}	
	}
	printf("\n");
	printf("Median: %f\n", np50);
	printf("Q1: %f\n", np25);
	printf("Q3: %f\n", np75);
	printf("Min: %d\n", min);
	printf("Max: %d\n", max);
	if (hist == 0) {
		printf("\n");
	}
	
}

int analysis(FILE* f, void* res, char* filename) {
	struct Analysis *a;
	int c, lnlen = 0, lnno = 1, bytes = 0;
	a = (struct Analysis*)res;
	a->filename = filename;
	while (1) {
		c = fgetc(f);
		if (c >= 0 && c <= 127) {
			a->ascii[c]++;
		}
		if (c == '\n') {
			if (lnlen > a->lnlen) {
				a->lnlen = lnlen;
				a->lnno = lnno;	
			}
			lnlen = 0;
			lnno++;	
		} else {
			lnlen++;		
		}
		if (c == EOF) {
			break;
		}
		bytes++;
	}
	return bytes;
}

int stats(FILE* f, void* res, char* filename) {
	Stats *s;
	int c;
	s = (Stats*)res;
	s->filename = filename;
	while (fscanf(f, "%d ", &c) > 0) {
		if (c >= NVAL) {
			return -1;
		}
		s->n++;
		s->sum += c;
		s->histogram[c]++;
	}
	return 0;
}

//Prints the usage statement
void usage() {
	printf("Usage:\t./mapreduce [h|v] FUNC DIR\n\tFUNC\tWhich operation you would like to run on the data:\n\t\tana - Analysis of various text files in a directory.\n\t\tstats - Calculates stats on files which contain only numbers.\n\tDIR\tThe directory in which the files are located.\n\n\tOptions:\n\t-h\tPrints this help menu.\n\t-v\tPrints the map function's results, stating the file it's from.\n");
}
