/*  $Id: wbuffer.c,v 1.5 2002-06-21 13:15:37 terpstra Exp $
 *  
 *  wbuffer.c - Implements a buffering system that delays appends to the flatfile
 *  
 *  Copyright (C) 2002 - Wesley W. Terpstra
 *  
 *  License: GPL
 *  
 *  Authors: 'Wesley W. Terpstra' <wesley@terpstra.ca>
 *  
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; version 2.1.
 *    
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *    
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#define _XOPEN_SOURCE 500
#define _BSD_SOURCE

/* #define DEBUG 1 */
/* #define PROFILE 1 */

#include "common.h"
#include "io.h"
#include "keyword.h"

#include "flatfile.h"
#include "wbuffer.h"
#include "btree.h"

#include <stdlib.h>
#include <math.h>

/* Okay! It's time I got off my ass and wrote the write buffering since it is
 * now officially the bottleneck. First of all, a mathematical derivation that
 * shows why we allocate cache the way we do. I am fairly sure that this could
 * be turned into a proof that this algorithm is the best, but I am a practical
 * programmer and I need functionality now! :-)
 * 
 * Okay, suppose you have a fixed amount of RAM (don't we all?) and you want
 * to reduce the number of flushes you make to disk based on a given key.
 * We assume here that these are append operations (which is our case).
 * 
 * At any given point in time, some of your RAM is used for each keyword or
 * else it is free. We consider flushing any number of appends at once to be
 * as slow as flushing a single append. This is actually quite accurate on 
 * modern hard disks.
 * 
 * So, suppose we have a static and known input frequency for each keyword.
 * (which we do). Call this f(x) for each keyword x. 
 * Note: sum { x: f(x) } = 1 and f(x) > 0 for all x
 * 
 * We want our algorithm to use the same space allocating function for each
 * keyword, so let s(x, t) describe how much storage each keyword uses. 
 * 
 * Now, here is the step that makes this not a proof that this is the best
 * possible algorithm: Forget about the time dependence. Give static storage
 * to each keyword. (this sound lame, but if you think about it -- we expect
 * each record to be half full, so this only really wastes half the RAM. As
 * we will see, the gain from doubling you RAM is diminishing, so this is
 * a constant factor difference [and < 2 at that]. Further still, the 
 * frequency [knowledge of the future] is static so we expect the storage
 * allocation to not change in the long term)
 * 
 * Supposing you get an append of keyword x, the cost is only paid when the
 * record is flushed. If we have a buffer size of s(x), the flush will send
 * out s(x) records.
 * 
 * Now, what is the expected cost at each step?
 * X = I_{a flush is needed}
 *   = sum { x: I_{the new record is x and x needs a flush} }
 *   = sum { x: I_{the new record is x} * I_{x needs a flush} }
 *
 * Take expectations, and note that x needing a flush and the new record being
 * x are independent.
 * E(X) = sum { x: f(x)*1/(1+s(x)) } = sum { x: f(x)/s(x) }
 * 
 * We want to minimize the expected cost. However, we also have the constraint
 * that sum { x: s(x) } = N --- that is, we have finite RAM. So, use our old
 * friend Lagrange multipliers:
 * 
 * Z = sum { x: f(x)/s(x) } + L*(sum { x: s(x) } - N)
 * differentiate:
 *   dZ/ds(x) = -f(x)/s(x)^2 + L = 0   ->  sqrt(f(x)/L) = s(x)  [1]
 *   dZ/dL    = sum { x: s(x) } - N  = 0
 *
 * Sum [1] across all keywords to get: sum{ y: sqrt(f(y)) } / sqrt(L) = N
 *
 * Now, divide [1] by this to get: 
 *    N*sqrt(f(x)) / sum { y: sqrt(f(y)) } = s(x)
 * Also, note that h(x)/A = f(x) <-- h is the number of hits we have seen
 * Then N*sqrt(h(x)/A) / sum { y: sqrt(h(y)/A) } =
 *      N*sqrt(h(x)) / sum { y: sqrt(h(y)) } = s(X)
 * So, we can just use hit count instead of frequency.
 *
 * This is the basis for our algorithm.
 * 
 * Also, notice that E(X) is thus sum: { y: sqrt(f(y)) }^2 / N
 * So, we have to double our buffer to get a 2* speed-up (not bad)
 *
 * Ok, back to reality. Sadly (and happily), we can't use s(x) = the
 * equation above since we live in an integer world. However, we will do our
 * best to approximate this solution. One nice thing about the real world is 
 * that s(x) < 1 is impossible since we simply flush it immediately then.
 * 
 * How do we exploit this? Well, so we recognize that unless the cache size
 * will round to 2 or more, there is no point in caching. Also, we may have
 * a limit on the number of different keywords cacheable (due to storing
 * their name). Anyways, the point is that less keywords than the whole data
 * set will have storage. Now, how do we allocate storage to those who will
 * get some?
 *
 * I propose that the above derivation can be repeated after first breaking
 * the probability into cases for no cache and with cache. This will arrive
 * at: s(x) = N*sqrt(h(x)) / sum { cached keywords: sqrt(h(y)) }
 *       or = 1 if not cached
 * This leads to E(X) = sum { cache kws: sqrt(f(y)) }^2/N + Pr(not cached)
 * 
 * Fair enough! Now, on to practical matters like how to manage the buffer.
 * In terms of how to decide who to cache: Not a problem! We keep a counter
 * for each cached record. If we have to flush an uncached record, it will
 * tell us the new size. If the order is ever disrupted, we shuffle records.
 * 
 * In terms of divisions in the buffer, again simple: Just leave them fixed
 * (initial even distribution), until we have imported some function of the
 * messages, then flush the whole thing and reassign space with correct
 * weight.
 * 
 * The more ugly matter is, of course, the blasted keyword string names
 * themselves. As i don't yet know what to do, I propose a bad solution:
 * MALLOC()!!! That's right! We will malloc all of them.
 * (and them collect stastics and decide what to do next)
 *  
 */

/*------------------------------------------------ Constant parameters */

/* Just how many appends do we plan on caching? (4Mb!) */
#define MY_RECORD_CACHE		1048576

/* Max size of my_wbuffer_kptr */
#define MY_WBUFFER_KMAX		0xFFFFUL

/* Just how many keywords will we max cache? */
#define MY_KEYWORD_CACHE	(MY_WBUFFER_KMAX-1)

/*------------------------------------------------- Private data structures */

typedef lu_word my_wbuffer_kptr;
typedef lu_quad my_wbuffer_aptr;

typedef struct My_WBuffer_Keyword_T
{
	char key[LU_KEYWORD_LEN+1];
	
	my_wbuffer_aptr	offset;
	my_wbuffer_aptr	size;
	my_wbuffer_aptr	fill;
	
	message_id	hits;
	
	my_wbuffer_kptr	more_common;
	my_wbuffer_kptr	less_common;
	
	lu_byte		skew;
	my_wbuffer_kptr	left;
	my_wbuffer_kptr	right;
} My_WBuffer_Keyword;

/*------------------------------------------------- Private variables */

static message_id*		my_wbuffer_acache;
static My_WBuffer_Keyword*	my_wbuffer_kcache;

static my_wbuffer_kptr		my_wbuffer_least_common;
static my_wbuffer_kptr		my_wbuffer_most_common;
static my_wbuffer_kptr		my_wbuffer_root;

static message_id		my_wbuffer_kcache_fill;
static message_id		my_wbuffer_count_down;
static message_id		my_wbuffer_next_sync;

#ifdef PROFILE
static message_id		ploads;
static message_id		phits;
static message_id		pknowns;
static message_id		pmisses;
static message_id		pentries;
static message_id		pflushes;
static message_id		pswaps;
#endif

/*------------------------------------------------- Private methods */

LU_BTREE_DEFINE(
	wbuffer,
	my_wbuffer_kptr,
	MY_WBUFFER_KMAX,
	My_WBuffer_Keyword,
	my_wbuffer_kcache,
	strcmp)

/* Dump the contents of a cache record to disk.
 */
static int my_flush_buffer(my_wbuffer_kptr k)
{
	message_id out;
	
	if (my_wbuffer_kcache[k].fill)
	{
#ifdef PROFILE
#ifdef DEBUG
		printf("f"); fflush(stdout);
#endif
		pflushes++;
#endif
		out = lu_flatfile_write_keyword_block(
			my_wbuffer_kcache[k].key,
			&my_wbuffer_acache[my_wbuffer_kcache[k].offset],
			my_wbuffer_kcache[k].fill);
		
		if (out == lu_common_minvalid)
			return -1;
		
		assert (out == my_wbuffer_kcache[k].hits);
		
		my_wbuffer_kcache[k].fill = 0;
	}
	
	return 0;
}

/* Swap two records buffer locations.
 */
static int my_swap_record_buffer(my_wbuffer_kptr a, my_wbuffer_kptr b)
{
	my_wbuffer_aptr i, e;
	message_id* ap;
	message_id* bp;
	message_id  tmp;
	
	if (my_wbuffer_kcache[a].fill > my_wbuffer_kcache[b].size)
		if (my_flush_buffer(a) != 0) return -1;
	if (my_wbuffer_kcache[b].fill > my_wbuffer_kcache[a].size)
		if (my_flush_buffer(b) != 0) return -1;
	
	if (my_wbuffer_kcache[a].fill > my_wbuffer_kcache[b].fill)
		e = my_wbuffer_kcache[a].fill;
	else	e = my_wbuffer_kcache[b].fill;
	
	ap = &my_wbuffer_acache[my_wbuffer_kcache[a].offset];
	bp = &my_wbuffer_acache[my_wbuffer_kcache[b].offset];
	
	for (i = 0; i != e; i++)
	{	/* Swap the buffer contents */
		tmp = bp[i];
		bp[i] = ap[i];
		ap[i] = tmp;
	}
	
	/* Swap the offsets */
	tmp = my_wbuffer_kcache[a].offset;
	my_wbuffer_kcache[a].offset = my_wbuffer_kcache[b].offset;
	my_wbuffer_kcache[b].offset = tmp;
	
	/* Swap the sizes */
	tmp = my_wbuffer_kcache[a].size;
	my_wbuffer_kcache[a].size = my_wbuffer_kcache[b].size;
	my_wbuffer_kcache[b].size = tmp;
	
	return 0;
}

/* Swap the position of two records
 */
static int my_swap_record_order(my_wbuffer_kptr a, my_wbuffer_kptr b)
{
	my_wbuffer_kptr amc, alc, bmc, blc;
	
	assert (a != b);
	
#ifdef PROFILE
#ifdef DEBUG
		printf("s"); fflush(stdout);
#endif
		pswaps++;
#endif

	if (my_swap_record_buffer(a, b) != 0)
		return -1;
	
	amc = my_wbuffer_kcache[a].more_common;
	alc = my_wbuffer_kcache[a].less_common;
	bmc = my_wbuffer_kcache[b].more_common;
	blc = my_wbuffer_kcache[b].less_common;
	
	if (a == bmc)
	{	/* a was more common than b, now not */
		assert (b == alc);
		my_wbuffer_kcache[a].more_common = b;
		my_wbuffer_kcache[b].less_common = a;
	}
	else
	{
		my_wbuffer_kcache[a].more_common = bmc;
		my_wbuffer_kcache[b].less_common = alc;
		if (alc != MY_WBUFFER_KMAX)
			my_wbuffer_kcache[alc].more_common = b;
		if (bmc != MY_WBUFFER_KMAX)
			my_wbuffer_kcache[bmc].less_common = a;
	}
	
	if (b == amc)
	{	/* Now a is more common than b */
		assert (a == blc);
		my_wbuffer_kcache[a].less_common = b;
		my_wbuffer_kcache[b].more_common = a;
	}
	else
	{
		my_wbuffer_kcache[a].less_common = blc;
		my_wbuffer_kcache[b].more_common = amc;
		
		if (amc != MY_WBUFFER_KMAX)
			my_wbuffer_kcache[amc].less_common = b;
		if (blc != MY_WBUFFER_KMAX)
			my_wbuffer_kcache[blc].more_common = a;
	}
	
	if      (my_wbuffer_least_common == a) my_wbuffer_least_common = b;
	else if (my_wbuffer_least_common == b) my_wbuffer_least_common = a;
	
	if      (my_wbuffer_most_common == a) my_wbuffer_most_common = b;
	else if (my_wbuffer_most_common == b) my_wbuffer_most_common = a;
	
	return 0;
}

/* Make sure that this key lives in the correct location */
static int my_relocate(my_wbuffer_kptr k)
{
	my_wbuffer_kptr mc;
	my_wbuffer_kptr scan;
	
	while ((mc = my_wbuffer_kcache[k].more_common) != MY_WBUFFER_KMAX &&
	       my_wbuffer_kcache[k].hits > my_wbuffer_kcache[mc].hits)
	{
		for (scan =  mc;
		     scan != MY_WBUFFER_KMAX;
		     scan =  my_wbuffer_kcache[scan].more_common)
		{
			if (my_wbuffer_kcache[scan].hits >
			    my_wbuffer_kcache[mc].hits)
				break;
		}
		
		if (scan == MY_WBUFFER_KMAX) scan = my_wbuffer_most_common;
		else                         scan = my_wbuffer_kcache[scan].less_common;
		
		if (my_swap_record_order(k, scan) != 0)
			return -1;
	}
	
	return 0;
}

/* Calculate who gets storage and how much. See top of file for proof.
 * This method is extremely costly since it flushes the ENTIRE buffer.
 * It should be invoked only rarely since the profile of keywords does not
 * fluctuate that much even if the individual keywords do.
 */
static void my_calc_storage()
{
	double accum, val;
	
	my_wbuffer_aptr off;
	my_wbuffer_aptr size;
	
	my_wbuffer_kptr	scan;
	my_wbuffer_kptr	cutoff;
	
	
	/* Pick an okay place to stop cache.
	 */
	accum = 0;
	for (scan =  my_wbuffer_most_common;
	     scan != MY_WBUFFER_KMAX;
	     scan =  my_wbuffer_kcache[scan].less_common)
	{
		val = sqrt(my_wbuffer_kcache[scan].hits);
		if (MY_RECORD_CACHE * val / (val + accum) <= 2)
			break;
		
		if (my_flush_buffer(scan) != 0) return;
	}
	cutoff = scan;
	
	/* Flush everything that doesn't make the cut */
	for ( ;
	     scan != MY_WBUFFER_KMAX; 
	     scan =  my_wbuffer_kcache[scan].less_common)
	{	/* Don't waste time flushing if we hit a zero */
		if (my_wbuffer_kcache[scan].size == 0) break;
		if (my_flush_buffer(scan) != 0) return;
		
		my_wbuffer_kcache[scan].size = 0;
	}
	
	/* Now, compute a sum properly from smallest to largest */
	accum = 0;
	
	if (cutoff == MY_WBUFFER_KMAX)
		scan = my_wbuffer_least_common;
	else
		scan = my_wbuffer_kcache[cutoff].more_common;
	
	for ( ;
	     scan != MY_WBUFFER_KMAX;
	     scan =  my_wbuffer_kcache[scan].more_common)
	{
		accum += sqrt(my_wbuffer_kcache[scan].hits);
	}
	
	/* Begin assigning the buffer -- start from the large side so that
	 * if we screw up due to rounding, only the small guy suffers
	 */
	off = 0;
	for (scan =  my_wbuffer_most_common;
	     scan != cutoff;
	     scan =  my_wbuffer_kcache[scan].less_common)
	{
		val = sqrt(my_wbuffer_kcache[scan].hits);
		size = MY_RECORD_CACHE * val / accum;
		
		if (size < 2) size = 2;
		
		if (size + off > MY_RECORD_CACHE)
			size = MY_RECORD_CACHE - off;
		
		/* printf("%f %f %d\n", val, accum, size); */
		
		my_wbuffer_kcache[scan].offset = off;
		my_wbuffer_kcache[scan].size   = size;
		
		off += size;
	}
	
	printf("Cache roundoff: %d/%d\n", off, MY_RECORD_CACHE);
}

/*------------------------------------------------- Public component methods */

int lu_wbuffer_init()
{
	my_wbuffer_acache = malloc(sizeof(message_id) * MY_RECORD_CACHE);
	if (!my_wbuffer_acache)
	{
		fputs(_("Failed to allocate storage for wbuffer appends\n"),
			stderr);
		return -1;
	}
	
	my_wbuffer_kcache = malloc(sizeof(My_WBuffer_Keyword) * MY_KEYWORD_CACHE);
	if (!my_wbuffer_kcache)
	{
		fputs(_("Failed to allocate storage for wbuffer keywords\n"),
			stderr);
		return -1;
	}
	
	my_wbuffer_least_common = my_wbuffer_root = 
	my_wbuffer_most_common  = MY_WBUFFER_KMAX;
	
	my_wbuffer_count_down   = my_wbuffer_kcache_fill = 0;
	my_wbuffer_next_sync	= 1024;
	
#ifdef PROFILE
	ploads   = 0;
	phits    = 0;
	pknowns  = 0;
	pmisses  = 0;
	pentries = 0;
	pflushes = 0;
	pswaps   = 0;
#endif
	
	return 0;
}
 
int lu_wbuffer_open()
{
	return 0;
}

int lu_wbuffer_sync()
{
	my_calc_storage();
	return 0;
}

int lu_wbuffer_close()
{
#ifdef PROFILE
	my_wbuffer_kptr scan;
	for (scan =  my_wbuffer_least_common;
	     scan != MY_WBUFFER_KMAX;
	     scan =  my_wbuffer_kcache[scan].more_common)
	{
		printf("%9d:%9d - '%s'\n", 
			my_wbuffer_kcache[scan].size,
			my_wbuffer_kcache[scan].hits,
			my_wbuffer_kcache[scan].key);
	}
	
	printf("WBuffer Loads:   %d\n", ploads);
	printf("WBuffer Hits:    %d\n", phits);
	printf("WBuffer Knowns:  %d\n", pknowns);
	printf("WBuffer Misses:  %d\n", pmisses);
	printf("WBuffer Entries: %d\n", pentries);
	printf("WBuffer Flushes: %d\n", pflushes);
	printf("WBuffer Swaps:   %d\n", pswaps);
#endif
	
	my_calc_storage();
	
	return 0;
}

int lu_wbuffer_quit()
{
	return 0;
}

/*-------------------------------------------------- Buffer methods */

int lu_wbuffer_append(
	const char* keyword,
	message_id  msg)
{
	int		out;
	int		dir;
	message_id	hits;
	my_wbuffer_kptr	scan;
	my_wbuffer_aptr	pos;
	my_wbuffer_kptr	tmp;
	
	/* Try to find the record in cache already */
	scan = my_wbuffer_root;
	while (scan != MY_WBUFFER_KMAX)
	{
		dir = strcmp(keyword, my_wbuffer_kcache[scan].key);
		
		if      (dir < 0) scan = my_wbuffer_kcache[scan].left;
		else if (dir > 0) scan = my_wbuffer_kcache[scan].right;
		else break;
	}
	
	/* If it is not in cache, but we are not utilizing all the record 
	 * names, push it in.
	 */
	if (scan == MY_WBUFFER_KMAX && my_wbuffer_kcache_fill != MY_KEYWORD_CACHE)
	{
#ifdef PROFILE
#ifdef DEBUG
		printf("l"); fflush(stdout);
#endif
		ploads++;
#endif
		scan = my_wbuffer_kcache_fill++;
		
		my_wbuffer_kcache[scan].less_common = MY_WBUFFER_KMAX;
		my_wbuffer_kcache[scan].more_common = my_wbuffer_least_common;
		
		if (my_wbuffer_least_common != MY_WBUFFER_KMAX)
			my_wbuffer_kcache[my_wbuffer_least_common].less_common = scan;
		else
			my_wbuffer_most_common = scan;
		my_wbuffer_least_common = scan;
		
		strcpy(my_wbuffer_kcache[scan].key, keyword);
		my_wbuffer_kcache[scan].offset = 0;
		my_wbuffer_kcache[scan].size   = 0;
		my_wbuffer_kcache[scan].fill   = 0;
		my_wbuffer_kcache[scan].hits   = 0; /* Filled in below */
		
		tmp = my_btree_wbuffer_insert(my_wbuffer_root, scan);
		assert (tmp != MY_WBUFFER_KMAX);
		my_wbuffer_root = tmp;
	}
	
	/* Are we dealing with a tracked keyword? */
	if (scan != MY_WBUFFER_KMAX)
	{
		if (my_wbuffer_kcache[scan].size)
		{
#ifdef PROFILE
#ifdef DEBUG
			printf("h"); fflush(stdout);
#endif
			phits++;
#endif
			/* This keyword has a buffer */
			pos = my_wbuffer_kcache[scan].offset +
			      my_wbuffer_kcache[scan].fill++;
			
			my_wbuffer_acache[pos] = msg;
			my_wbuffer_kcache[scan].hits++;
			
			if (my_wbuffer_kcache[scan].fill ==
			    my_wbuffer_kcache[scan].size)
			{
				out = my_flush_buffer(scan);
			}
			else
			{
				out = 0;
			}
		}
		else
		{
#ifdef PROFILE
#ifdef DEBUG
			printf("k"); fflush(stdout);
#endif
			pknowns++;
#endif
			hits = lu_flatfile_write_keyword_block(
				keyword, &msg, 1);
			
			if (hits == lu_common_minvalid)
			{
				out = -1;
			}
			else
			{	/* Record the hits we are tracking */
				assert (hits > my_wbuffer_kcache[scan].hits);
				my_wbuffer_kcache[scan].hits = hits;
				out = 0;
			}
		}
		
		my_relocate(scan);	/* This can only grow the buffer */
	}
	else
	{
#ifdef PROFILE
#ifdef DEBUG
		printf("m"); fflush(stdout);
#endif
		pmisses++;
#endif
		hits = lu_flatfile_write_keyword_block(
			keyword, &msg, 1);
		out = (hits == lu_common_minvalid);
		
		if (out == 0 && hits > my_wbuffer_kcache[my_wbuffer_least_common].hits)
		{	/* Promote this record to cache */
#ifdef PROFILE
#ifdef DEBUG
			printf("e"); fflush(stdout);
#endif
			pentries++;
#endif
			my_flush_buffer(my_wbuffer_least_common);
			
			tmp = my_btree_wbuffer_remove(
				my_wbuffer_root, my_wbuffer_least_common);
			assert (tmp != MY_WBUFFER_KMAX);
			my_wbuffer_root = tmp;
			
			strcpy(my_wbuffer_kcache[my_wbuffer_least_common].key, keyword);
			my_wbuffer_kcache[my_wbuffer_least_common].hits = hits;
			
			tmp = my_btree_wbuffer_insert(
				my_wbuffer_root, my_wbuffer_least_common);
			assert (tmp != MY_WBUFFER_KMAX);
			my_wbuffer_root = tmp;
			
			my_relocate(my_wbuffer_least_common);
		}
	}
	
	my_wbuffer_count_down++;
	if (my_wbuffer_count_down == my_wbuffer_next_sync)
	{
		my_wbuffer_count_down = 0;
		my_calc_storage();
		
		if (my_wbuffer_next_sync < 8 * MY_RECORD_CACHE)
			my_wbuffer_next_sync <<= 1;
	}
	
	return out;
}

int lu_wbuffer_flush(
	const char* keyword)
{
	int		dir;
	my_wbuffer_kptr	scan;
	
	/* Try to find the record in cache already */
	scan = my_wbuffer_root;
	while (scan != MY_WBUFFER_KMAX)
	{
		dir = strcmp(keyword, my_wbuffer_kcache[scan].key);
		
		if      (dir < 0) scan = my_wbuffer_kcache[scan].left;
		else if (dir > 0) scan = my_wbuffer_kcache[scan].right;
		else return my_flush_buffer(scan);
	}
	
	return 0;
}
