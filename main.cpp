#define WIN32_LEAN_AND_MEAN			1
#define VC_EXTRALEAN				1
#define _CRT_SECURE_NO_DEPRECATE	1
#include <windows.h>
#include <cstdio>
#include <iostream>
using namespace std;

const char *LineEnds = "\r\n";

size_t GetFileLength(FILE *fp)
{
	rewind(fp);
	fseek(fp, 0, SEEK_END);
	size_t ret = ftell(fp);
	rewind(fp);
	return ret;
}

size_t LoadFile(const char *filename, char **out)
{
	FILE *fp = fopen(filename, "r");
	if(!fp) return 0;
	size_t len = GetFileLength(fp);
	*out = new char[len+1];
	if(!*out) return 0;
	size_t ret = fread(*out, 1, len, fp);
	out[0][len] = 0;
	fclose(fp);
	return ret;
}

size_t WriteFile(const char *filename, char *data)
{
	FILE *fp = fopen(filename, "w");
	if(!fp) return 0;
	size_t ret = fwrite(data, 1, strlen(data), fp);
	fclose(fp);
	return ret;
}

char *GetFirstLine(char *str)
{
	size_t ret = strspn(str, LineEnds);
	return str+ret;
}

char *GetNextLine(char *str)
{
	char *end = strpbrk(str, LineEnds);
	if(!end) return 0;
	return GetFirstLine(end);
}

void add(const char *in, char *out, long long millis)
{
	long long hours   = 0;
	long long minutes = 0;
	long long seconds = 0;
	char *next  = 0;
	
	hours   = strtoll(in,     &next, 10);
	minutes = strtoll(next+1, &next, 10);
	seconds = strtoll(next+1, &next, 10);
	millis += strtoll(next+1, &next, 10);
	
	/*Positive Rationalization*/
	while(millis>=1000)
	{
		seconds++;
		millis-=1000;
	}
	
	while(seconds>=60)
	{
		minutes++;
		seconds-=60;
	}
	
	while(minutes>=60)
	{
		hours++;
		minutes-=60;
	}
	/**************************/
	
	/*Negitive Rationalization*/
	while(millis<0)
	{
		seconds--;
		millis+=1000;
	}
	
	while(seconds<0)
	{
		minutes--;
		seconds+=60;
	}
	
	while(minutes<0)
	{
		hours--;
		minutes+=60;
	}
	
	if(hours<0)
	{
		hours++;
		if(minutes) minutes = 60-minutes;
		if(seconds) seconds = 60-seconds;
		if(millis) millis  = 1000-millis;
	}
	/**************************/
	
	snprintf(out, 13, "%02lli:%02lli:%02lli,%03lli", hours, minutes, seconds, millis);
}

bool IsTimeLine(const char *line)
{
	if(line[2]!=':') return false;	
	if(strncmp(line+12, " --> ", 5)) return false;
	return true;
}

void Usage(const char *name)
{
	cout << "Usage: " << name << " input.srt output.srt offset\n";
}

void dump(const char *data, int len)
{
	for(int i = 0; i<len; i++)
	{
		printf("[%i]'%c', %i\n", i, data[i], (int)data[i]);
	}
}

int main(int argc, char *argv[])
{
	setbuf(stdout, 0);
	long long offset;
	if(argc!=4)
	{
		Usage(argv[0]);
		return 10;
	}
	
	offset = strtoll(argv[3], 0, 10);
	if(!offset)
	{
		Usage(argv[0]);
		return 10;
	}

	char *data = 0;
	char *line = 0;
	
	LoadFile(argv[1], &data);
	
	if(!data)
	{
		cout << "Cannot open " << argv[1] << " for reading\n";
		return 4;
	}
	
	cout << "Processing File...";
	char end;
	line = GetFirstLine(data);
	while(line && *line)
	{
		if(IsTimeLine(line))
		{
			end = line[29];
			add(line, line, offset);
			add(line+17, line+17, offset);
			line[12] = ' ';
			line[29] = end;
		}
		line = GetNextLine(line);
	}
	cout << "Complete!\n";
	
	if(!WriteFile(argv[2], data))
	{
		cout << "Cannot open " << argv[2] << " for writing\n";
		return 16;
	}
	
	return 0;
}
