/*
 * gencorestub.c - generate stub for core OS modules (DAL, UI, Boot).
 *
 * gencorestub TARGET HEADER OFFSET OBJDIR
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ENTRIES	0x1000

struct entry {
	char		name[256];
	struct entry *	next;
};

static struct entry *entries[MAX_ENTRIES];

int main(int argc, char *argv[])
{
	int offset, i, max_i;
	FILE *fp;

	if (argc != 4)
		return 1;

	fp = fopen(argv[1], "r");
	if (fp == NULL)
		return 2;

	offset = atoi(argv[2]);

	memset(entries, 0, sizeof(entries));

	max_i = 0;
	while (!feof(fp)) {
		struct entry *entry;
		char name[80], line[256];

		fgets(line, 256, fp);
		name[0] = 0;
		sscanf(line, "#define %s %x", name, &i);
		if (strncmp(name, "entryNum", 8) || i < 0 || i >= MAX_ENTRIES)
			continue;

		if (i > max_i)
			max_i = i;

		entry = (struct entry *)malloc(sizeof(*entry));
		strcpy(entry->name, name + 8);
		if (entries[i] == NULL) {
			entry->next = NULL;
			entries[i] = entry;
		} else {
			entry->next = entries[i];
			entries[i] = entry;
		}
	}

	for (i = 0; i < MAX_ENTRIES; i++) {
		struct entry *entry;
		char fname[32];
		FILE *sfp;

		if (entries[i] == NULL)
			continue;

		entry = entries[i];
		if (max_i >= 0x100)
			sprintf(fname, "%s/%s.s", argv[3], entry->name);
		else
			sprintf(fname, "%s/%s.s", argv[3], entry->name);
		sfp = fopen(fname, "w");
		if (sfp == NULL)
			continue;
		fprintf(sfp, "	.text\n");
		do {
			fprintf(sfp, "	.global	%s\n", entry->name);
			fprintf(sfp, "%s:\n", entry->name);
		} while ((entry = entry->next));
		fprintf(sfp, "	ldr	ip, [r9, #%d]\n", offset);
		fprintf(sfp, "	ldr	pc, [ip, #%d]\n", i * 4);
		fclose(sfp);
	}

	return 0;
}
