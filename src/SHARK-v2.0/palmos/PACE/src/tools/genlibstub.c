/*
 * genlibstub.c - generate ARM-native library stub for use in fully native applications.
 *
 * genlibstub TARGET HEADER APPREFNUM APPLIBLINKOFFSET TYPE CREATOR REVISION ENTRIES OBJDIR
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cygwin/in.h>

#define MAX_ENTRIES	0x1000

struct entry {
	char		name[256];
	struct entry *	next;
};

static struct entry *entries[MAX_ENTRIES];

int main(int argc, char *argv[])
{
	int apprefnum, appliblinkoffset, revision, nentries, i;
	char *target, filename[32];
	unsigned int type, creator;
	FILE *fp, *sfp;
	char *p;

	if (argc != 10)
		return 1;

	target = argv[1];

	fp = fopen(argv[2], "r");
	if (fp == NULL)
		return 2;

	apprefnum = strtol(argv[3], &p, 16);
	appliblinkoffset = strtol(argv[4], &p, 16);

	if (strlen(argv[5]) != 4 || strlen(argv[6]) != 4)
		return 3;

	type = ntohl(*(unsigned int *)argv[5]);
	creator = ntohl(*(unsigned int *)argv[6]);

	revision = strtol(argv[7], &p, 16);
	nentries = strtol(argv[8], &p, 16);

	memset(entries, 0, sizeof(entries));

	while (!feof(fp)) {
		struct entry *entry;
		char name[80], line[256];

		fgets(line, 256, fp);
		name[0] = 0;
		sscanf(line, "#define %s %x", name, &i);
		if (strncmp(name, "entryNum", 8) || i < 0 || i >= MAX_ENTRIES)
			continue;

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

		if (entries[i] == NULL)
			continue;

		sprintf(filename, "%s/%s_0x%2.2x.s", argv[9], target, i);
		sfp = fopen(filename, "w");
		if (sfp == NULL)
			continue;
		entry = entries[i];
		fprintf(sfp, "	.text\n");
		do {
			fprintf(sfp, "	.global	%s\n", entry->name);
			fprintf(sfp, "%s:\n", entry->name);
		} while ((entry = entry->next));
		fprintf(sfp, "	ldr	ip, [r9]\n");
		fprintf(sfp, "	ldr	ip, [ip, #%d]\n", apprefnum);
		fprintf(sfp, "	ldr	ip, [ip, #%d]\n", appliblinkoffset);
		fprintf(sfp, "	cmp	ip, #0\n");
		fprintf(sfp, "	addne	pc, ip, #%d\n", i * 4);
		fprintf(sfp, "	stmdb	sp!, {pc}\n");
		fprintf(sfp, "	b	%s_LinkerStub\n", target);
		fclose(sfp);
	}

	sprintf(filename, "%s/%s_LinkerStub.s", argv[9], target);
	sfp = fopen(filename, "w");
	fprintf(sfp, "	.text\n");
	fprintf(sfp, "	.global	%s_LinkerStub\n", target);
	fprintf(sfp, "%s_LinkerStub:\n", target);
	fprintf(sfp, "	adr	ip, 0f\n");
	fprintf(sfp, "	stmdb	sp!, {ip}\n");
	fprintf(sfp, "	b	LibLinkerStub\n");
	fprintf(sfp, "0:	.long	0x%x\n", type);
	fprintf(sfp, "	.long	0x%x\n", creator);
	fprintf(sfp, "	.long	0x%x\n", revision);
	fprintf(sfp, "	.long	0x%x\n", nentries);
	fprintf(sfp, "	.long	0x%x\n", 'amdd');
	fprintf(sfp, "	.long	0x%x\n", 'amdi');
	fprintf(sfp, "	.long	0x%x\n", 'amdc');
	fprintf(sfp, "	.long	0\n");
	fprintf(sfp, "	.long	0\n");
	fclose(sfp);

	fclose(fp);	

	return 0;
}
