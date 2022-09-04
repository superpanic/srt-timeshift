// september 2022, fredrik josefsson, superpanic.com

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#define ARG_COUNT 3
#define EQUAL 0
#define COLON ':'
#define COMMA ','
#define TIME_SEPARATOR " --> "
#define MAX_LINES 20000

void die(const char *message);
bool isValidFile(char *path);
bool stringIsNumber(char *str);
int lineContainsChar(char *line, char c);
void timeShift(long *h_ptr, long *m_ptr, long *s_ptr, long *ms_ptr, long h_shift, long m_shift, long s_shift, long ms_shift);

int main(int argc, char *argv[]) {

	// help?
	if(strcmp(argv[1], "help") == EQUAL) {
		printf("\nUse this tool to shift all time stamps in an .srt subtitle file.\nProvide two arguments: \n 1. PATH to .srt file \n 2. TIME in format 00:00:00,000 in hours:minutes:seconds,milliseconds\n\n");
		die("");
	}
	
	// check number of arguments
	if(argc != ARG_COUNT) 
		die("Arguments missing.");

	// is first argument a valid path?
	char *path = argv[1];
	if(!isValidFile(path)) 
		die("First argument is not a valid path.");

	// is filetype of path .srt ?
	int path_length = strlen(path);
	if(path_length < 4) 
		die("Path too short, not valid.");
	const char *file_type = &path[path_length-4];
	if(strcmp(file_type, ".srt") != EQUAL) 
		die("File type  .srt  not valid.");

	const char *ts_fail_msg = "Could not read time shift argument.";

	char *time_shift = argv[2];
	char *head;
	char *end;
	
	head = time_shift;

	long shift_hours;
	shift_hours = strtol(head, &end, 10);
	if(head==end) die(ts_fail_msg);
	if(end[0]==COLON) end++;
	else die(ts_fail_msg);

	long shift_minutes;
	head = end;
	shift_minutes = strtol(head, &end, 10);
	if(head==end) die(ts_fail_msg);
	if(end[0]==COLON) end++;
	else die(ts_fail_msg);

	long shift_seconds;
	head = end;
	shift_seconds = strtol(head, &end, 10);
	if(head==end) die(ts_fail_msg);
	if(end[0]==COMMA) end++;
	else die(ts_fail_msg);

	long shift_milliseconds;
	head = end;
	shift_milliseconds = strtol(head, &end, 10);
	if(head==end) die(ts_fail_msg);

	printf("Time shifting subtitles %ld hours, %ld minutes, %ld seconds, %ld milliseconds \n", shift_hours, shift_minutes, shift_seconds, shift_milliseconds);

	// open file
	FILE *subtitles_source_file; 
	FILE *subtitles_destination_file;
	char *line = NULL;
	size_t buffer_size = 0;
	ssize_t chars_read;

	subtitles_source_file = fopen(path, "r");
	if(subtitles_source_file == NULL)
		die("Failed to read file.");

	subtitles_destination_file = fopen("subtitles_out.srt", "w");
	if(subtitles_destination_file == NULL)
		die("Failed to write file.");

	int line_counter = 0;
	char new_line[32];
	while((chars_read = getline(&line, &buffer_size, subtitles_source_file)) != EOF) {

		bool is_time_shifted = false;

		// read hours in
		long hours_in;
		head = end = line;
		hours_in = strtol(head, &end, 10);
		if(head == end) goto write_line;

		if(end[0]==COLON) {
			end++;
		} else {
			goto write_line;
		}

		// read minutes in
		long minutes_in;
		head = end;
		minutes_in = strtol(head, &end, 10);
		if(head==end) goto write_line;

		if(end[0]==COLON) {
			end++;
		} else {
			goto write_line;
		}

		// read seconds in
		long seconds_in;
		head = end;
		seconds_in = strtol(head, &end, 10);
		if(head==end) goto write_line;
		if(end[0]==COMMA) {
			end++;
		} else {
			goto write_line;
		}

		// read milli-seconds in
		long milliseconds_in;
		head = end;
		milliseconds_in = strtol(head, &end, 10);
		if(head==end) goto write_line;

		// read time separator
		char time_separator[5+1];
		memcpy(time_separator, end, strlen(TIME_SEPARATOR)+1);
		if(strcmp(time_separator, TIME_SEPARATOR)) {
			end += strlen(TIME_SEPARATOR)+1;
		} else {
			goto write_line;
		}	

		// read hours out
		long hours_out;
		head = end;
		hours_out = strtol(head, &end, 10);
		if(head==end) goto write_line;
		if(end[0]==COLON) {
			end++;
		} else {
			goto write_line;
		}

		// read minutes out
		long minutes_out;
		head = end;
		minutes_out = strtol(head, &end, 10);
		if(head==end) goto write_line;
		if(end[0]==COLON) {
			end++;
		} else {
			goto write_line;
		}
		
		// read seconds out
		long seconds_out;
		head = end;
		seconds_out = strtol(head, &end, 10);
		if(head==end) goto write_line;
		if(end[0]==COMMA) {
			end++;
		} else {
			goto write_line;
		}

		// read milliseconds out
		long milliseconds_out;
		head = end;
		milliseconds_out = strtol(head, &end, 10);
		if(head==end) goto write_line;
		
		timeShift(&hours_in, &minutes_in, &seconds_in, &milliseconds_in, shift_hours, shift_minutes, shift_seconds, shift_milliseconds);
		timeShift(&hours_out, &minutes_out, &seconds_out, &milliseconds_out, shift_hours, shift_minutes, shift_seconds, shift_milliseconds);

		// format: "00:02:18,312 --> 00:02:22,838"
		sprintf(new_line, "%02ld:%02ld:%02ld,%02ld --> %02ld:%02ld:%02ld,%02ld\n", hours_in, minutes_in, seconds_in, milliseconds_in, hours_out, minutes_out, seconds_out, milliseconds_out);
		
		is_time_shifted = true;

write_line:
		if(is_time_shifted) {
			fprintf(subtitles_destination_file, "%s", new_line);
		} else {
			fprintf(subtitles_destination_file, "%s", line);
		}
		if(line_counter++ > MAX_LINES) break;
	}

	fclose(subtitles_source_file);
	fclose(subtitles_destination_file);

	printf("Finished, closing.");
	return 0;
}

void timeShift(long *h_ptr, long *m_ptr, long *s_ptr, long *ms_ptr, long h_shift, long m_shift, long s_shift, long ms_shift) {
	long h = *h_ptr, m = *m_ptr, s = *s_ptr, ms = *ms_ptr;

	long ms_total = (h * 3600 * 1000) + (m * 60 * 1000)+(s * 1000) + ms;
	long ms_total_shift = (h_shift*3600*1000) + (m_shift*60*1000) + (s_shift*1000) + ms_shift;
	ms_total = ms_total + ms_total_shift;

	h = ms_total/(3600*1000);
	ms_total = ms_total-(h * 3600*1000);
	m = ms_total/(60*1000);
	ms_total = ms_total-(m * 60 * 1000);
	s = ms_total/1000;
	ms_total = ms_total-(s*1000);
	ms = ms_total;

	*h_ptr=h;
	*m_ptr=m;
	*s_ptr=s;
	*ms_ptr=ms;
}

int lineContainsChar(char *line, char c) {
	for(int i=0; i<strlen(line); i++) {
		if(line[i] == c) return i;
	}
	return -1;
}

bool stringIsNumber(char *str) {
	for(int i=0;i<strlen(str);i++){
		if(!isdigit((char)str[i])) {
			return false;
		}
	}
	return true;
}

bool isValidFile(char *path) {
	FILE *f = fopen(path, "r");
	if(f) {
		fclose(f);
		return true;
	} else {
		return false;
	}
}

void die(const char *message) {
	if (errno) perror("");
	printf("PROGRAM END: %s\n", message);
	exit(errno);
}