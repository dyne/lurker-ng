/*  $Id: wbuffer.c,v 1.4 2002-06-17 14:22:55 terpstra Exp $
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

#include "common.h"
#include "io.h"

#include "flatfile.h"
#include "wbuffer.h"

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
 * (initial even distribution), until we have imported say LU_WBUFFER_SYNC
 * messages, then flush the whole thing and reassign space with correct
 * weight.
 * 
 * The more ugly matter is, of course, the blasted keyword string names
 * themselves. As i don't yet know what to do, I propose a bad solution:
 * MALLOC()!!! That's right! We will malloc all of them.
 * (and them collect stastics and decide what to do next)
 *  
 */

/*------------------------------------------------- Public component methods */

int lu_wbuffer_init()
{
	return 0;
}

int lu_wbuffer_open()
{
	return 0;
}

int lu_wbuffer_sync()
{
	return 0;
}

int lu_wbuffer_close()
{
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
	return lu_flatfile_write_keyword_block(
		keyword, &msg, 1);
}

int lu_wbuffer_flush(
	const char* keyword)
{
	return 0;
}
