/*  $Id: wbuffer.c,v 1.3 2002-06-14 11:16:59 terpstra Exp $
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
 * out 1+s(x) records (why flush unless we are over?).
 * 
 * Now, what is the expected cost at each step?
 * X = I_{a flush is needed}
 *   = sum { x: I_{the new record is x and x needs a flush} }
 *   = sum { x: I_{the new record is x} * I_{x needs a flush} }
 *
 * Take expectations, and note that x needing a flush and the new record being
 * x are independent.
 * E(X) = sum { x: f(x)*1/(1+s(x)) } = sum { x: f(x)/(1+s(x)) }
 * 
 * We want to minimize the expected cost. However, we also have the constraint
 * that sum { x: s(x) } = N --- that is, we have finite RAM. So, use our old
 * friend Lagrange multipliers:
 * 
 * Z = sum { x: f(x)/(1+s(x)) } + L*(sum { x: s(x) } - N)
 * differentiate:
 *   dZ/ds(x) = -f(x)/(1+s(x))^2 + L = 0   ->  sqrt(f(x)/L) = (1+s(x))^2
 *   dZ/dL    = sum { x: s(x) } - N  = 0
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
