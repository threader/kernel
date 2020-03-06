/*
 * trace_seq.c
 *
 * Copyright (C) 2008-2014 Red Hat Inc, Steven Rostedt <srostedt@redhat.com>
 *
 * The trace_seq is a handy tool that allows you to pass a descriptor around
 * to a buffer that other functions can write to. It is similar to the
 * seq_file functionality but has some differences.
 *
 * To use it, the trace_seq must be initialized with trace_seq_init().
 * This will set up the counters within the descriptor. You can call
 * trace_seq_init() more than once to reset the trace_seq to start
 * from scratch.
 * 
 * The buffer size is currently PAGE_SIZE, although it may become dynamic
 * in the future.
 *
 * A write to the buffer will either succed or fail. That is, unlike
 * sprintf() there will not be a partial write (well it may write into
 * the buffer but it wont update the pointers). This allows users to
 * try to write something into the trace_seq buffer and if it fails
 * they can flush it and try again.
 *
 */
#include <linux/uaccess.h>
#include <linux/seq_file.h>
#include <linux/trace_seq.h>

/* How much buffer is left on the trace_seq? */
#define TRACE_SEQ_BUF_LEFT(s) ((PAGE_SIZE - 1) - (s)->len)

/* How much buffer is written? */
#define TRACE_SEQ_BUF_USED(s) min((s)->len, (unsigned int)(PAGE_SIZE - 1))

/**
 * trace_seq_bitmask - write a bitmask array in its ASCII representation
 * @s:		trace sequence descriptor
 * @maskp:	points to an array of unsigned longs that represent a bitmask
 * @nmaskbits:	The number of bits that are valid in @maskp
 *
 * Writes a ASCII representation of a bitmask string into @s.
 *
 * Returns 1 if we successfully written all the contents to
 *   the buffer.
 * Returns 0 if we the length to write is bigger than the
 *   reserved buffer space. In this case, nothing gets written.
 */
int trace_seq_bitmask(struct trace_seq *s, const unsigned long *maskp,
		      int nmaskbits)
{
	unsigned int len = TRACE_SEQ_BUF_LEFT(s);
	int ret;

	if (s->full || !len)
		return 0;

	ret = bitmap_scnprintf(s->buffer, len, maskp, nmaskbits);
	s->len += ret;

	return 1;
}
EXPORT_SYMBOL_GPL(trace_seq_bitmask);

#define MAX_MEMHEX_BYTES	8U
#define HEX_CHARS		(MAX_MEMHEX_BYTES*2 + 1)
