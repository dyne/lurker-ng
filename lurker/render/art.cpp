const char* find_art_end(const char* start, const char* end) {
	const char* out = 0;
	const char* c = start;
	goto find_art_end_start;
find_art_end_l0:
	++c;
find_art_end_start:
	if (c == end) return out;
	if (*c < 11) 
		if (*c < 10) goto find_art_end_l1;
		else goto find_art_end_l2;
	else goto find_art_end_l1;
find_art_end_l1:
	return out;
find_art_end_l2:
	++c;
	if (c == end) return out;
	if (*c < 11) 
		if (*c < 10) 
			if (*c < 9) goto find_art_end_l3;
			else goto find_art_end_l5;
		else goto find_art_end_l1;
	else 
		if (*c < 33) 
			if (*c < 32) goto find_art_end_l3;
			else goto find_art_end_l9;
		else goto find_art_end_l3;
find_art_end_l3:
	++c;
	if (c == end) return out;
	if (*c < 11) 
		if (*c < 10) 
			if (*c < 9) goto find_art_end_l3;
			else goto find_art_end_l4;
		else goto find_art_end_l2;
	else 
		if (*c < 33) 
			if (*c < 32) goto find_art_end_l3;
			else goto find_art_end_l6;
		else goto find_art_end_l3;
find_art_end_l4:
	++c;
	out = c;
	if (c == end) return out;
	if (*c < 11) 
		if (*c < 10) goto find_art_end_l4;
		else goto find_art_end_l5;
	else goto find_art_end_l4;
find_art_end_l5:
	++c;
	if (c == end) return out;
	if (*c < 11) 
		if (*c < 10) 
			if (*c < 9) goto find_art_end_l4;
			else goto find_art_end_l5;
		else goto find_art_end_l1;
	else 
		if (*c < 33) 
			if (*c < 32) goto find_art_end_l4;
			else goto find_art_end_l5;
		else goto find_art_end_l4;
find_art_end_l6:
	++c;
	if (c == end) return out;
	if (*c < 11) 
		if (*c < 10) 
			if (*c < 9) goto find_art_end_l3;
			else goto find_art_end_l4;
		else goto find_art_end_l2;
	else 
		if (*c < 33) 
			if (*c < 32) goto find_art_end_l3;
			else goto find_art_end_l7;
		else goto find_art_end_l3;
find_art_end_l7:
	++c;
	if (c == end) return out;
	if (*c < 11) 
		if (*c < 10) 
			if (*c < 9) goto find_art_end_l3;
			else goto find_art_end_l4;
		else goto find_art_end_l2;
	else 
		if (*c < 33) 
			if (*c < 32) goto find_art_end_l3;
			else goto find_art_end_l8;
		else goto find_art_end_l3;
find_art_end_l8:
	++c;
	if (c == end) return out;
	if (*c < 11) 
		if (*c < 10) 
			if (*c < 9) goto find_art_end_l3;
			else goto find_art_end_l4;
		else goto find_art_end_l2;
	else 
		if (*c < 33) 
			if (*c < 32) goto find_art_end_l3;
			else goto find_art_end_l4;
		else goto find_art_end_l3;
find_art_end_l9:
	++c;
	if (c == end) return out;
	if (*c < 11) 
		if (*c < 10) 
			if (*c < 9) goto find_art_end_l3;
			else goto find_art_end_l5;
		else goto find_art_end_l1;
	else 
		if (*c < 33) 
			if (*c < 32) goto find_art_end_l3;
			else goto find_art_end_l10;
		else goto find_art_end_l3;
find_art_end_l10:
	++c;
	if (c == end) return out;
	if (*c < 11) 
		if (*c < 10) 
			if (*c < 9) goto find_art_end_l3;
			else goto find_art_end_l5;
		else goto find_art_end_l1;
	else 
		if (*c < 33) 
			if (*c < 32) goto find_art_end_l3;
			else goto find_art_end_l11;
		else goto find_art_end_l3;
find_art_end_l11:
	++c;
	if (c == end) return out;
	if (*c < 11) 
		if (*c < 10) 
			if (*c < 9) goto find_art_end_l3;
			else goto find_art_end_l5;
		else goto find_art_end_l1;
	else 
		if (*c < 33) 
			if (*c < 32) goto find_art_end_l3;
			else goto find_art_end_l5;
		else goto find_art_end_l3;
}
char* find_art_starts(const char* start, const char* end, char* scratch) {
	const char* c = end;
	scratch += (end - start);
	goto find_art_starts_start;
find_art_starts_l0:
	*--scratch = 0;
find_art_starts_start:
	if (c-- == start) return scratch;
	if (*c < 11) 
		if (*c < 10) 
			if (*c < 9) goto find_art_starts_l1;
			else goto find_art_starts_l4;
		else goto find_art_starts_l0;
	else 
		if (*c < 33) 
			if (*c < 32) goto find_art_starts_l1;
			else goto find_art_starts_l8;
		else goto find_art_starts_l1;
find_art_starts_l1:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 11) 
		if (*c < 10) 
			if (*c < 9) goto find_art_starts_l1;
			else goto find_art_starts_l2;
		else goto find_art_starts_l0;
	else 
		if (*c < 33) 
			if (*c < 32) goto find_art_starts_l1;
			else goto find_art_starts_l5;
		else goto find_art_starts_l1;
find_art_starts_l2:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 11) 
		if (*c < 10) goto find_art_starts_l2;
		else goto find_art_starts_l3;
	else goto find_art_starts_l2;
find_art_starts_l3:
	*--scratch = 1;
	if (c-- == start) return scratch;
	if (*c < 11) 
		if (*c < 10) 
			if (*c < 9) goto find_art_starts_l2;
			else goto find_art_starts_l4;
		else goto find_art_starts_l0;
	else 
		if (*c < 33) 
			if (*c < 32) goto find_art_starts_l2;
			else goto find_art_starts_l4;
		else goto find_art_starts_l2;
find_art_starts_l4:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 11) 
		if (*c < 10) 
			if (*c < 9) goto find_art_starts_l2;
			else goto find_art_starts_l4;
		else goto find_art_starts_l0;
	else 
		if (*c < 33) 
			if (*c < 32) goto find_art_starts_l2;
			else goto find_art_starts_l4;
		else goto find_art_starts_l2;
find_art_starts_l5:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 11) 
		if (*c < 10) 
			if (*c < 9) goto find_art_starts_l1;
			else goto find_art_starts_l2;
		else goto find_art_starts_l0;
	else 
		if (*c < 33) 
			if (*c < 32) goto find_art_starts_l1;
			else goto find_art_starts_l6;
		else goto find_art_starts_l1;
find_art_starts_l6:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 11) 
		if (*c < 10) 
			if (*c < 9) goto find_art_starts_l1;
			else goto find_art_starts_l2;
		else goto find_art_starts_l0;
	else 
		if (*c < 33) 
			if (*c < 32) goto find_art_starts_l1;
			else goto find_art_starts_l7;
		else goto find_art_starts_l1;
find_art_starts_l7:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 11) 
		if (*c < 10) 
			if (*c < 9) goto find_art_starts_l1;
			else goto find_art_starts_l2;
		else goto find_art_starts_l0;
	else 
		if (*c < 33) 
			if (*c < 32) goto find_art_starts_l1;
			else goto find_art_starts_l2;
		else goto find_art_starts_l1;
find_art_starts_l8:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 11) 
		if (*c < 10) 
			if (*c < 9) goto find_art_starts_l1;
			else goto find_art_starts_l4;
		else goto find_art_starts_l0;
	else 
		if (*c < 33) 
			if (*c < 32) goto find_art_starts_l1;
			else goto find_art_starts_l9;
		else goto find_art_starts_l1;
find_art_starts_l9:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 11) 
		if (*c < 10) 
			if (*c < 9) goto find_art_starts_l1;
			else goto find_art_starts_l4;
		else goto find_art_starts_l0;
	else 
		if (*c < 33) 
			if (*c < 32) goto find_art_starts_l1;
			else goto find_art_starts_l10;
		else goto find_art_starts_l1;
find_art_starts_l10:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 11) 
		if (*c < 10) 
			if (*c < 9) goto find_art_starts_l1;
			else goto find_art_starts_l4;
		else goto find_art_starts_l0;
	else 
		if (*c < 33) 
			if (*c < 32) goto find_art_starts_l1;
			else goto find_art_starts_l4;
		else goto find_art_starts_l1;
}
