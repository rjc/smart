#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "libsmart.h"

extern smart_h device_open(smart_protocol_e, char *);
extern void device_close(smart_h);
extern int32_t device_read(smart_h, void *, size_t);

smart_h
smart_open(smart_protocol_e protocol, char *devname)
{

	printf("opening %s\n", devname);
	return device_open(protocol, devname);
}

void
smart_close(smart_h h)
{

	device_close(h);
}

smart_buf_t *
smart_read(smart_h h)
{
	smart_buf_t *sb = NULL;

	sb = malloc(sizeof(smart_buf_t));
	if (sb) {
		sb->bsize = 512;
		sb->b = malloc(sb->bsize);
		if (sb->b == NULL) {
			free(sb);
			sb = NULL;
		} else {
			device_read(h, sb->b, sb->bsize);

			sb->vcount = 30;
		}
	}
	
	return sb;
}

void
smart_free(smart_buf_t *sb)
{
	if (sb == NULL)
		return;

	if (sb->b != NULL) {
		free(sb->b);
		sb->b = NULL;
	}

	free(sb);
}

void
smart_print(smart_h h, smart_buf_t *sb, int32_t which, uint32_t flags)
{
	uint32_t i;
	uint8_t *b = NULL;
	const char *fmt, *lfmt;
	bool do_hex = false;
	bool do_thresh = false;

	if (flags & 0x1)
		do_hex = true;

	if (flags & 0x2)
		do_thresh = true;

	/* Decimal or hex */
	if (do_hex) {
		fmt = "%#01.1x %#01.1lx\n";
		lfmt = "%#01.1x %#01.1x %#01.1x %#01.1x %#01.1x %#01.1lx\n";
	} else {
		fmt = "%d %ld\n";
		lfmt = "%d %d %d %d %d %ld\n";
	}

	b = sb->b;

	b += 2;

	for (i = 0; i < sb->vcount; i++) {
		if (*b != 0) {
			uint64_t raw = 0UL;

			if ((which == -1) || (*b == which)) {
				raw =  (uint64_t)b[10] << 40 |
					 (uint64_t)b[9] << 32 |
					b[8] << 24 |
					b[7] << 16 |
					b[6] << 8 |
					b[5];

				if (do_thresh)
					printf(lfmt, b[0], b[1], b[2], b[3], b[4], raw);
				else
					printf(fmt, b[0], raw);

				if (which != -1)
					break;
			}
		}

		b += 12;
	}
}

