/*  $Id: wbuffer.c,v 1.8 2002-06-24 11:50:44 terpstra Exp $
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
#define _ISOC99_SOURCE

/* #define DEBUG 1 */
/* #define PROFILE 1 */

#include "common.h"
#include "io.h"
#include "keyword.h"

#include "flatfile.h"
#include "wbuffer.h"
#include "btree.h"

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <st.h>

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

typedef lu_quad my_wbuffer_aptr;
typedef lu_word my_wbuffer_kptr;

typedef my_wbuffer_kptr	my_wbuffer_sptr;

typedef struct My_WBuffer_Slot_T
{
	my_wbuffer_kptr	keyword;
	
	my_wbuffer_aptr	offset;
	my_wbuffer_aptr	size;
	my_wbuffer_aptr	fill;
} My_WBuffer_Slot;

typedef struct My_WBuffer_Keyword_T
{
	char key[LU_KEYWORD_LEN+1];
	
	lu_byte		skew;
	my_wbuffer_kptr	left;
	my_wbuffer_kptr	right;
	
	message_id	hits;
	my_wbuffer_sptr	slot;
} My_WBuffer_Keyword;

typedef struct My_WBuffer_Digest_T
{
	char		key[LU_KEYWORD_LEN+1];
	message_id	hits;
} My_WBuffer_Digest;

/*------------------------------------------------- Private variables */

static message_id*		my_wbuffer_acache;
static My_WBuffer_Keyword*	my_wbuffer_kcache;
static My_WBuffer_Slot*		my_wbuffer_scache;

static my_wbuffer_kptr		my_wbuffer_kroot;
static message_id		my_wbuffer_kcache_fill; /* also slot fill */

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
	my_wbuffer_sptr s = my_wbuffer_kcache[k].slot;
	message_id out;
	
	if (my_wbuffer_scache[s].fill)
	{
#ifdef PROFILE
#ifdef DEBUG
		printf("f"); fflush(stdout);
#endif
		pflushes++;
#endif
		out = lu_flatfile_write_keyword_block(
			my_wbuffer_kcache[k].key,
			&my_wbuffer_acache[my_wbuffer_scache[s].offset],
			my_wbuffer_scache[s].fill);
		
		if (out == lu_common_minvalid)
			return -1;
		
		assert (out == my_wbuffer_kcache[k].hits);
		my_wbuffer_scache[s].fill = 0;
	}
	
	return 0;
}

/* Swap two records buffer locations.
 */
static int my_swap_slot(my_wbuffer_kptr a, my_wbuffer_kptr b)
{
	my_wbuffer_aptr i, e, atmp;
	my_wbuffer_sptr as, bs;
	message_id* ap;
	message_id* bp;
	message_id  tmp;
	
	assert (a != b);
	
#ifdef PROFILE
#ifdef DEBUG
		printf("s"); fflush(stdout);
#endif
		pswaps++;
#endif
	
	as = my_wbuffer_kcache[a].slot;
	bs = my_wbuffer_kcache[b].slot;
	
	/* Make sure they will fit in their new homes */
	if (my_wbuffer_scache[as].fill > my_wbuffer_scache[bs].size)
		if (my_flush_buffer(a) != 0) return -1;
	if (my_wbuffer_scache[bs].fill > my_wbuffer_scache[as].size)
		if (my_flush_buffer(b) != 0) return -1;
	
	if (my_wbuffer_scache[as].fill > my_wbuffer_scache[bs].fill)
		e = my_wbuffer_scache[as].fill;
	else	e = my_wbuffer_scache[bs].fill;
	
	/* Swap the buffer contents */
	ap = &my_wbuffer_acache[my_wbuffer_scache[as].offset];
	bp = &my_wbuffer_acache[my_wbuffer_scache[bs].offset];
	
	for (i = 0; i != e; i++)
	{
		tmp = bp[i];
		bp[i] = ap[i];
		ap[i] = tmp;
	}
	
	/* Swap the slots  */
	my_wbuffer_kcache[a].slot = bs;
	my_wbuffer_kcache[b].slot = as;
	my_wbuffer_scache[as].keyword = b;
	my_wbuffer_scache[bs].keyword = a;
	
	/* Swap the fill */
	atmp = my_wbuffer_scache[as].fill;
	my_wbuffer_scache[as].fill = my_wbuffer_scache[bs].fill;
	my_wbuffer_scache[bs].fill = atmp;
	
	return 0;
}

/* Make sure that this key lives in the correct location */
static int my_relocate(my_wbuffer_kptr key)
{
	my_wbuffer_sptr slot;
	my_wbuffer_sptr l, r, m;
	my_wbuffer_kptr kscan;
	message_id	hits;
	int		swaps;
	
	/* We must not just swap with our eventual position because
	 * this would put the fellow we replace in a (possibly) undeserved
	 * small location. So, we will swap out the largest of each group
	 * of equal frequency records until we arrive at our resting place.
	 * 
	 * This is actually ok, because we usually only increased by one unless
	 * this is a really common keyword we just haven't seen yet. But, these
	 * are rare---and we probably see them first.
	 */
	swaps = 0;
	while ((slot = my_wbuffer_kcache[key].slot) != 0 &&
	       (hits = my_wbuffer_kcache[my_wbuffer_scache[slot-1].keyword].hits)
	        < my_wbuffer_kcache[key].hits)
	{	/* We deserve a bigger slot */
		
		swaps++;
		/* Find the most common record which has hits equal to hits.
		 * We maintain an interval [ l, r ) of candidates
		 */
		
		l = 0;
		r = slot;
		
		while (r - l > 1)
		{
			m = (l+r)/2;
			assert (m > 0);
			
			kscan = my_wbuffer_scache[m-1].keyword;
			if (my_wbuffer_kcache[kscan].hits == hits)
			{	/* m-1 is better */
				r = m;
			}
			else
			{	/* m-1 is not a candidate */
				l = m;
			}
		}
		
		/* Ok, l now points to the largest record with correct hits */
		kscan = my_wbuffer_scache[l].keyword;
		my_swap_slot(kscan, key);
	}
	
	if (swaps > 1)
	{
		syslog(LOG_WARNING, 
			_("Had to swap %d times to find correct position. wbuffer.hist was probably corrupt; no big deal."),
			swaps);
	}
	
	return 0;
}

static message_id my_grab_entry(const char* key)
{
	my_wbuffer_sptr	slot;
	my_wbuffer_kptr	scan;
	my_wbuffer_kptr	tmp;
	
#ifdef PROFILE
#ifdef DEBUG
	printf("l"); fflush(stdout);
#endif
	ploads++;
#endif
	slot = scan = my_wbuffer_kcache_fill++;
	assert(slot != MY_WBUFFER_KMAX);
	
	strcpy(my_wbuffer_kcache[scan].key, key);
	my_wbuffer_kcache[scan].hits = 0; /* Filled in below */
	my_wbuffer_kcache[scan].slot = slot;
	
	my_wbuffer_scache[slot].offset  = 0;
	my_wbuffer_scache[slot].size    = 0;
	my_wbuffer_scache[slot].fill    = 0;
	my_wbuffer_scache[slot].keyword = scan;
	
	tmp = my_btree_wbuffer_insert(my_wbuffer_kroot, scan);
	assert (tmp != MY_WBUFFER_KMAX);
	my_wbuffer_kroot = tmp;
	
	return slot;
}

static int my_count_hit(my_wbuffer_kptr scan, message_id id)
{
	int		out;
	my_wbuffer_aptr	pos;
	my_wbuffer_sptr	slot;
	message_id	hits;
	
	slot = my_wbuffer_kcache[scan].slot;
	
	if (my_wbuffer_scache[slot].size)
	{
#ifdef PROFILE
#ifdef DEBUG
		printf("h"); fflush(stdout);
#endif
		phits++;
#endif
		/* This keyword has a buffer */
		pos = my_wbuffer_scache[slot].offset +
		      my_wbuffer_scache[slot].fill++;
		
		my_wbuffer_acache[pos] = id;
		my_wbuffer_kcache[scan].hits++;
		
		if (my_wbuffer_scache[slot].fill ==
		    my_wbuffer_scache[slot].size)
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
			my_wbuffer_kcache[scan].key, &id, 1);
			
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
	
	return out;
}

void my_promote_key(const char* key, message_id hits)
{
	my_wbuffer_kptr	tmp;
	my_wbuffer_sptr	slot = my_wbuffer_kcache_fill-1;
	my_wbuffer_kptr	scan = my_wbuffer_scache[slot].keyword;
	
	if (hits > my_wbuffer_kcache[scan].hits)
	{	/* Promote this record to cache */
#ifdef PROFILE
#ifdef DEBUG
		printf("e"); fflush(stdout);
#endif
		pentries++;
#endif
		
		my_flush_buffer(scan);
		
		tmp = my_btree_wbuffer_remove(my_wbuffer_kroot, scan);
		assert (tmp != MY_WBUFFER_KMAX);
		my_wbuffer_kroot = tmp;
		
		strcpy(my_wbuffer_kcache[scan].key, key);
		my_wbuffer_kcache[scan].hits = hits;
		
		tmp = my_btree_wbuffer_insert(my_wbuffer_kroot, scan);
		assert (tmp != MY_WBUFFER_KMAX);
		my_wbuffer_kroot = tmp;
		
		my_relocate(scan);
	}
}

/* Calculate who gets storage and how much. See top of file for proof.
 * This method is extremely costly since it flushes the ENTIRE buffer.
 * It should be invoked only rarely since the profile of keywords does not
 * fluctuate that much even if the individual keywords do.
 */
static void my_calc_storage(int yield)
{
	double accum, val;
	
	/*!!!!! This doesn't actually HAVE to flush things...
	 * I could make it be smarter
	 */
	
	my_wbuffer_aptr off;
	my_wbuffer_aptr size;
	
	my_wbuffer_kptr	kw;
	my_wbuffer_sptr	scan;
	my_wbuffer_sptr	cutoff;
	
	if (my_wbuffer_kcache_fill == 0)
		return;
	
	/* Pick an okay place to stop cache.
	 */
	accum = 0;
	for (scan = 0; scan < my_wbuffer_kcache_fill; scan++)
	{
		kw = my_wbuffer_scache[scan].keyword;
		val = sqrt(my_wbuffer_kcache[kw].hits);
		if (MY_RECORD_CACHE * val / (val + accum) <= 2)
			break;
		
		if (my_flush_buffer(scan) != 0) return;
		if (yield) st_sleep(0);
	}
	cutoff = scan;
	assert (cutoff > 0);
	
	/* Flush everything that doesn't make the cut */
	for (scan = cutoff; scan < my_wbuffer_kcache_fill; scan++)
	{	/* Don't waste time flushing if we hit a zero */
		if (my_wbuffer_scache[scan].size == 0) break;
		
		kw = my_wbuffer_scache[scan].keyword;
		if (my_flush_buffer(kw) != 0) return;
		
		my_wbuffer_scache[scan].size = 0;
		if (yield) st_sleep(0);
	}
	
	/* Now, compute a sum properly from smallest to largest */
	accum = 0;
	for (scan = cutoff-1; scan != 0; scan--)
	{
		kw = my_wbuffer_scache[scan].keyword;
		accum += sqrt(my_wbuffer_kcache[kw].hits);
	}
	kw = my_wbuffer_scache[0].keyword;
	accum += sqrt(my_wbuffer_kcache[kw].hits);
	
	/* Begin assigning the buffer -- start from the large side so that
	 * if we screw up due to rounding, only the small guy suffers
	 */
	off = 0;
	for (scan = 0; scan != cutoff; scan++)
	{
		kw = my_wbuffer_scache[scan].keyword;
		val = sqrt(my_wbuffer_kcache[kw].hits);
		size = lrint(MY_RECORD_CACHE * val / accum);
		
		if (size < 2) size = 2;
		
		if (size + off > MY_RECORD_CACHE)
			size = MY_RECORD_CACHE - off;
		
		/* printf("%f %f %d\n", val, accum, size); */
		
		my_wbuffer_scache[scan].offset = off;
		my_wbuffer_scache[scan].size   = size;
		
		off += size;
	}
	
#if defined(DEBUG) || defined(PROFILE)
	printf("Cache roundoff: %d/%d\n", off, MY_RECORD_CACHE);
#endif
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
	
	my_wbuffer_scache = malloc(sizeof(My_WBuffer_Slot) * MY_KEYWORD_CACHE);
	if (!my_wbuffer_scache)
	{
		fputs(_("Failed to allocate storage for wbuffer slots\n"),
			stderr);
		return -1;
	}
	
	my_wbuffer_kroot = MY_WBUFFER_KMAX;
	
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
	My_WBuffer_Digest	digest;
	my_wbuffer_kptr		key;
	
	int fd = open("wbuffer.hist", O_BINARY | O_RDONLY);
	if (fd == -1) return 0;
	
	while (read(fd, &digest, sizeof(digest)) == sizeof(digest))
	{
		key = my_grab_entry(digest.key);
		my_wbuffer_kcache[key].hits = digest.hits;
	}
	
	close(fd);
	
	return 0;
}

int lu_wbuffer_sync()
{
	my_calc_storage(0);
	return 0;
}

int lu_wbuffer_close()
{
	my_wbuffer_sptr		scan;
	my_wbuffer_kptr		key;
	My_WBuffer_Digest	digest;

	int fd = open("wbuffer.hist", 
		O_BINARY | O_WRONLY | O_TRUNC | O_CREAT, LU_S_WRITE | LU_S_READ);
	if (fd == -1)
	{
		syslog(LOG_WARNING, _("Failed to record cache offsets: %s -- initial import next time will be slow"),
			strerror(errno));
		return 0;
	}
	
	for (scan = 0; scan < my_wbuffer_kcache_fill; scan++)
	{
		key = my_wbuffer_scache[scan].keyword;
		
		memset(&digest.key[0], 0, sizeof(digest.key));
		strcpy(&digest.key[0], my_wbuffer_kcache[key].key);
		digest.hits = my_wbuffer_kcache[key].hits;
		
		if (write(fd, &digest, sizeof(digest)) != sizeof(digest))
		{
			syslog(LOG_WARNING, _("Failed to record cache offsets: %s -- initial import next time will be slow"),
				strerror(errno));
			break;
		}
	}
	
	my_calc_storage(0);
	close(fd);
	
#ifdef PROFILE
	for (scan = my_wbuffer_kcache_fill-1; scan != MY_WBUFFER_KMAX; scan--)
	{
		key = my_wbuffer_scache[scan].keyword;
		
		printf("%9d:%9d - '%s'\n", 
			my_wbuffer_scache[scan].size,
			my_wbuffer_kcache[key].hits,
			my_wbuffer_kcache[key].key);
	}
	
	printf("WBuffer Loads:   %d\n", ploads);
	printf("WBuffer Hits:    %d\n", phits);
	printf("WBuffer Knowns:  %d\n", pknowns);
	printf("WBuffer Misses:  %d\n", pmisses);
	printf("WBuffer Entries: %d\n", pentries);
	printf("WBuffer Flushes: %d\n", pflushes);
	printf("WBuffer Swaps:   %d\n", pswaps);
#endif
	
	my_calc_storage(0);
	
	return 0;
}

int lu_wbuffer_quit()
{
	free(my_wbuffer_scache);
	free(my_wbuffer_kcache);
	free(my_wbuffer_acache);
	
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
	
	/* Try to find the record in cache already */
	scan = my_wbuffer_kroot;
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
		scan = my_grab_entry(keyword);
	}
	
	/* Are we dealing with a tracked keyword? */
	if (scan != MY_WBUFFER_KMAX)
	{
		out = my_count_hit(scan, msg);
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
		
		if (out == 0) my_promote_key(keyword, hits);
	}
	
	my_wbuffer_count_down++;
	if (my_wbuffer_count_down == my_wbuffer_next_sync)
	{
		my_wbuffer_count_down = 0;
		my_calc_storage(1);
		
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
	scan = my_wbuffer_kroot;
	while (scan != MY_WBUFFER_KMAX)
	{
		dir = strcmp(keyword, my_wbuffer_kcache[scan].key);
		
		if      (dir < 0) scan = my_wbuffer_kcache[scan].left;
		else if (dir > 0) scan = my_wbuffer_kcache[scan].right;
		else return my_flush_buffer(scan);
	}
	
	return 0;
}
