/*  $Id: wbuffer.c,v 1.3 2002-07-09 22:42:45 terpstra Exp $
 *  
 *  wbuffer.c - Implements a buffering system that write combines
 *  
 *  Copyright (C) 2002 - Wesley W. Terpstra
 *  
 *  License: LGPL
 *  
 *  Authors: 'Wesley W. Terpstra' <wesley@terpstra.ca>
 *  
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation;
 *    version 2 of the License.
 *    
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *    
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */

#define _XOPEN_SOURCE 500
#define _BSD_SOURCE
#define _ISOC99_SOURCE

/* #define DEBUG 1 */
/* #define PROFILE 1 */

#include "../config.h"
#include "private.h"
#include "avl.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifdef HAVE_ERRNO_H
# include <errno.h>
#endif

#ifdef HAVE_ASSERT_H
# include <assert.h> 
#endif

#ifndef assert
# define assert(x) do { if (!x) { printf("\nASSERT FAILURE: %s:%i: '%s'\n", __FI
#endif



/*!!!!!!!!!!!!!!! THIS IS A BUG: fix it later */
#define LU_KEYWORD_LEN	100


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

/*------------------------------------------------ Type information */

typedef size_t		aptr;
typedef unsigned short	kptr;

typedef kptr	sptr;

typedef struct Slot_T
{
	kptr	keyword;
	
	aptr	offset;
	aptr	size;
	aptr	fill;
} Slot;

typedef struct Keyword_T
{
	kptr	left;
	kptr	right;
	
	aptr	hits;
	sptr	slot;
	
	char	skew;
	char	key[LU_KEYWORD_LEN+1];
} Keyword;

struct Kap_Wbuffer
{
	aptr		num_appends;
	sptr		num_slots;
	
	unsigned char*	acache;
	
	Keyword*	kcache;
	Slot*		scache;
	
	kptr		kroot;
	kptr		kfill;
	
	size_t		count_down;
	size_t		next_sync;
	
	size_t		record_size; /* taken from append */
};

#ifdef PROFILE
static long		ploads;
static long		phits;
static long		pknowns;
static long		pmisses;
static long		pentries;
static long		pflushes;
static long		pswaps;
#endif

/* Constants for use by the avl tree - set before each op */
#define	KINVALID	0xFFFFUL

static Keyword*	avl_cache; /*!!! change avl to take this as param */

/*------------------------------------------------- Private methods */

AVL_DEFINE(
	wb,
	kptr,
	KINVALID,
	Keyword,
	avl_cache,
	strcmp)

/* Dump the contents of a cache record to disk.
 */
static int flush_buffer(Kap k, kptr kp)
{
	int		out;
	size_t		count;
	Keyword*	kw = &k->wbuffer->kcache[kp];
	Slot*		sl = &k->wbuffer->scache[kw->slot];
	
	if (sl->fill)
	{
#ifdef PROFILE
#ifdef DEBUG
		printf("f"); fflush(stdout);
#endif
		pflushes++;
#endif
		out = kap_append_real(k, 
			kw->key,
			&k->wbuffer->acache[sl->offset*k->wbuffer->record_size],
			sl->fill,
			&count);
		
		if (out) return out;
		
		assert (count == kw->hits);
		
		sl->fill = 0;
	}
	
	return 0;
}

static void swap_buf(unsigned char* a, unsigned char* b, size_t len)
{
	size_t tmp;
	unsigned char tmp2;
	
	while (len >= sizeof(size_t))
	{
		tmp = *((size_t*)a);
		*((size_t*)a) = *((size_t*)b);
		*((size_t*)b) = tmp;
		
		len -= sizeof(size_t);
		a   += sizeof(size_t);
		b   += sizeof(size_t);
	}
	
	while (len)
	{
		tmp2 = *a;
		*a = *b;
		*b = tmp2;
		
		len--;
		a++;
		b++;
	}
}

/* Swap two records buffer locations.
 */
static int swap_slot(Kap k, kptr a, kptr b)
{
	aptr	e, atmp;
	int	out;
	sptr	stmp;
	
	Keyword* ka = &k->wbuffer->kcache[a];
	Keyword* kb = &k->wbuffer->kcache[b];
	
	Slot* sa = &k->wbuffer->scache[ka->slot];
	Slot* sb = &k->wbuffer->scache[kb->slot];
	
	assert (a != b);
	
#ifdef PROFILE
#ifdef DEBUG
		printf("s"); fflush(stdout);
#endif
		pswaps++;
#endif
	
	/* Make sure they will fit in their new homes */
	if (sa->fill >= sb->size)
		if ((out = flush_buffer(k, a)) != 0) return out;
	if (sb->fill >= sa->size)
		if ((out = flush_buffer(k, b)) != 0) return out;
	
	if (sa->fill > sb->fill)
		e = sa->fill;
	else	e = sb->fill;
	
	/* Swap the buffer contents */
	swap_buf(&k->wbuffer->acache[sa->offset*k->wbuffer->record_size], 
	         &k->wbuffer->acache[sb->offset*k->wbuffer->record_size], 
	         e * k->wbuffer->record_size);
	
	/* Swap the slots  */
	stmp = ka->slot;
	ka->slot = kb->slot;
	kb->slot = stmp;
	
	sa->keyword = b;
	sb->keyword = a;
	
	/* Swap the fill */
	atmp = sa->fill;
	sa->fill = sb->fill;
	sb->fill = atmp;
	
	return 0;
}

/* Make sure that this key lives in the correct location */
static int relocate(Kap k, kptr key)
{
	sptr	slot;
	sptr	l, r, m;
	kptr	kscan;
	aptr	hits;
	int	swaps;
	
	Keyword* kw = &k->wbuffer->kcache[key];
	
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
	while ((slot = kw->slot) != 0 &&
	       (hits = k->wbuffer->kcache[
	       		k->wbuffer->scache[slot-1].keyword
	       		].hits) < kw->hits)
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
			
			kscan = k->wbuffer->scache[m-1].keyword;
			if (k->wbuffer->kcache[kscan].hits == hits)
			{	/* m-1 is better */
				r = m;
			}
			else
			{	/* m-1 is not a candidate */
				l = m;
			}
		}
		
		/* Ok, l now points to the largest record with correct hits */
		kscan = k->wbuffer->scache[l].keyword;
		swap_slot(k, kscan, key);
	}
	
	return 0;
}

static sptr grab_entry(Kap k, const char* key)
{
	sptr	slot;
	kptr	scan;
	kptr	tmp;
	
	Keyword*	kw;
	Slot*		sl;
	
#ifdef PROFILE
#ifdef DEBUG
	printf("l"); fflush(stdout);
#endif
	ploads++;
#endif
	slot = scan = k->wbuffer->kfill++;
	assert(slot != k->wbuffer->num_slots);
	
	kw = &k->wbuffer->kcache[scan];
	sl = &k->wbuffer->scache[slot];
	
	strcpy(kw->key, key);
	kw->hits = 0; /* Filled in below */
	kw->slot = slot;
	
	sl->offset  = 0;
	sl->size    = 0;
	sl->fill    = 0;
	sl->keyword = scan;
	
	avl_cache = k->wbuffer->kcache;
	tmp = my_avl_wb_insert(k->wbuffer->kroot, scan);
	assert (tmp != KINVALID);
	k->wbuffer->kroot = tmp;
	
	return slot;
}

static int count_hit(Kap k, kptr scan, unsigned char* buf)
{
	int	out;
	aptr	pos;
	aptr	hits;
	
	Keyword*	kw = &k->wbuffer->kcache[scan];
	Slot*		sl = &k->wbuffer->scache[kw->slot];
	
	if (sl->size)
	{
#ifdef PROFILE
#ifdef DEBUG
		printf("h"); fflush(stdout);
#endif
		phits++;
#endif
		/* This keyword has a buffer */
		pos = sl->offset + sl->fill++;
		
		memcpy(&k->wbuffer->acache[pos*k->wbuffer->record_size], 
			buf,
			k->wbuffer->record_size);
		kw->hits++;
		
		if (sl->fill == sl->size)
		{
			out = flush_buffer(k, scan);
			if (out) return out;
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
		out = kap_append_real(k, kw->key, buf, 1, &hits);
		if (out) return out;
		
		/* Record the hits we are tracking */
		assert (hits > kw->hits);
		kw->hits = hits;
	}
	
	return relocate(k, scan); /* This can only grow the buffer */
}

static int promote_key(Kap k, const char* key, aptr hits)
{
	kptr	tmp;
	sptr	slot = k->wbuffer->kfill-1;
	kptr	scan = k->wbuffer->scache[slot].keyword;
	
	if (hits > k->wbuffer->kcache[scan].hits)
	{	/* Promote this record to cache */
#ifdef PROFILE
#ifdef DEBUG
		printf("e"); fflush(stdout);
#endif
		pentries++;
#endif
		
		flush_buffer(k, scan);
		
		avl_cache = k->wbuffer->kcache;
		tmp = my_avl_wb_remove(k->wbuffer->kroot, scan);
		assert (tmp != KINVALID);
		k->wbuffer->kroot = tmp;
		
		strcpy(k->wbuffer->kcache[scan].key, key);
		k->wbuffer->kcache[scan].hits = hits;
		
		tmp = my_avl_wb_insert(k->wbuffer->kroot, scan);
		assert (tmp != KINVALID);
		k->wbuffer->kroot = tmp;
		
		return relocate(k, scan);
	}
	
	return 0;
}

/* Calculate who gets storage and how much. See top of file for proof.
 * This method is extremely costly since it flushes the ENTIRE buffer.
 * It should be invoked only rarely since the profile of keywords does not
 * fluctuate that much even if the individual keywords do.
 */
static int calc_storage(Kap k, int yield)
{
	double accum, val;
	
	/*!!!!! This doesn't actually HAVE to flush things...
	 * I could make it be smarter
	 */
	
	aptr	off;
	aptr	size;
	
	int	out;
	
	kptr	kw;
	sptr	scan;
	sptr	cutoff;
	
	if (k->wbuffer->kfill == 0)
		return 0;
	
	/* Pick an okay place to stop cache.
	 */
	accum = 0;
	for (scan = 0; scan < k->wbuffer->kfill; scan++)
	{
		kw = k->wbuffer->scache[scan].keyword;
		val = sqrt(k->wbuffer->kcache[kw].hits);
		if (k->wbuffer->num_appends * val / (val + accum) <= 2)
			break;
		
		out = flush_buffer(k, scan);
		if (out != 0) return out;
	}
	cutoff = scan;
	assert (cutoff > 0);
	
	/* Flush everything that doesn't make the cut */
	for (scan = cutoff; scan < k->wbuffer->kfill; scan++)
	{	/* Don't waste time flushing if we hit a zero */
		if (k->wbuffer->scache[scan].size == 0) break;
		
		kw = k->wbuffer->scache[scan].keyword;
		
		out = flush_buffer(k, kw);
		if (out != 0) return out;
		
		k->wbuffer->scache[scan].size = 0;
	}
	
	/* Now, compute a sum properly from smallest to largest */
	accum = 0;
	for (scan = cutoff-1; scan != 0; scan--)
	{
		kw = k->wbuffer->scache[scan].keyword;
		accum += sqrt(k->wbuffer->kcache[kw].hits);
	}
	kw = k->wbuffer->scache[0].keyword;
	accum += sqrt(k->wbuffer->kcache[kw].hits);
	
	/* Begin assigning the buffer -- start from the large side so that
	 * if we screw up due to rounding, only the small guy suffers
	 */
	off = 0;
	for (scan = 0; scan != cutoff; scan++)
	{
		kw = k->wbuffer->scache[scan].keyword;
		val = sqrt(k->wbuffer->kcache[kw].hits);
		size = lrint(k->wbuffer->num_appends * val / accum);
		
		if (size < 2) size = 2;
		
		/* Deal with round-off error */
		if (size + off > k->wbuffer->num_appends)
			size = k->wbuffer->num_appends - off;
		
		/* printf("%f %f %d\n", val, accum, size); */
		
		k->wbuffer->scache[scan].offset = off;
		k->wbuffer->scache[scan].size   = size;
		
		off += size;
	}
	
#if defined(DEBUG) || defined(PROFILE)
	printf("Cache roundoff: %d/%d\n", off, k->wbuffer->num_appends);
#endif
	
	return 0;
}

/*------------------------------------------------- Public component methods */

struct Kap_Wbuffer* wbuffer_init()
{
	struct Kap_Wbuffer* wbuffer = malloc(sizeof(struct Kap_Wbuffer));
	if (!wbuffer) return 0;
	
	wbuffer->num_appends = 1024*1024*4;
	wbuffer->num_slots   = 65532;
	
	wbuffer->acache = 0;
	wbuffer->kcache = 0;
	wbuffer->scache = 0;
	
	return wbuffer;
}

int kap_wbuffer_open(Kap k, const char* dir, const char* prefix)
{
	kap_append_get_recordsize(k, &k->wbuffer->record_size);
	
	k->wbuffer->acache = malloc(k->wbuffer->record_size * k->wbuffer->num_appends);
	k->wbuffer->kcache = malloc(sizeof(Keyword)         * k->wbuffer->num_slots);
	k->wbuffer->scache = malloc(sizeof(Slot)            * k->wbuffer->num_slots);
	
	if (!k->wbuffer->acache || !k->wbuffer->kcache || !k->wbuffer->scache)
		return ENOMEM;
	
	k->wbuffer->kroot = KINVALID;
	k->wbuffer->kfill = 0;
	
	k->wbuffer->count_down  = 0;
	k->wbuffer->next_sync   = 1024;
	
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

int kap_wbuffer_sync(Kap k)
{
	return calc_storage(k, 0);
}

int kap_wbuffer_close(Kap k)
{
	int out;
	
	out = calc_storage(k, 0);
	if (out) return out;
	
	if (k->wbuffer->scache) free(k->wbuffer->scache); k->wbuffer->scache = 0;
	if (k->wbuffer->kcache) free(k->wbuffer->kcache); k->wbuffer->kcache = 0;
	if (k->wbuffer->acache) free(k->wbuffer->acache); k->wbuffer->acache = 0;
	
	return 0;
}

/*-------------------------------------------------- Entry-point methods */

int kap_wbuffer_push(Kap k, const char* keyword, unsigned char* buf)
{
	int	out;
	int	dir;
	aptr	hits;
	kptr	scan;
	
	/* Try to find the record in cache already */
	scan = k->wbuffer->kroot;
	while (scan != KINVALID)
	{
		Keyword* kw = &k->wbuffer->kcache[scan];
		dir = strcmp(keyword, kw->key);
		
		if      (dir < 0) scan = kw->left;
		else if (dir > 0) scan = kw->right;
		else break;
	}
	
	/* If it is not in cache, but we are not utilizing all the record 
	 * names, push it in.
	 */
	if (scan == KINVALID && k->wbuffer->kfill != k->wbuffer->num_slots)
	{
		scan = grab_entry(k, keyword);
	}
	
	/* Are we dealing with a tracked keyword? */
	if (scan != KINVALID)
	{
		out = count_hit(k, scan, buf);
		if (out) return out;
	}
	else
	{
#ifdef PROFILE
#ifdef DEBUG
		printf("m"); fflush(stdout);
#endif
		pmisses++;
#endif
		out = kap_append_real(k, keyword, buf, 1, &hits);
		if (out) return out;
		
		promote_key(k, keyword, hits);
	}
	
	k->wbuffer->count_down++;
	if (k->wbuffer->count_down == k->wbuffer->next_sync)
	{
		k->wbuffer->count_down = 0;
		calc_storage(k, 1);
		
		if (k->wbuffer->next_sync < 8 * k->wbuffer->num_appends)
			k->wbuffer->next_sync <<= 1;
	}
	
	return 0;
}

int kap_wbuffer_flush(Kap k, const char* keyword)
{
	int		dir;
	Keyword*	kw;
	kptr		scan;
	
	/* Try to find the record in cache already */
	scan = k->wbuffer->kroot;
	while (scan != KINVALID)
	{
		kw = &k->wbuffer->kcache[scan];
		
		dir = strcmp(keyword, kw->key);
		
		if      (dir < 0) scan = kw->left;
		else if (dir > 0) scan = kw->right;
		else return flush_buffer(k, scan);
	}
	
	return 0;
}
