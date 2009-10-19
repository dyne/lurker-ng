const char* find_quote_end(const char* start, const char* end) {
	const char* out = 0;
	const char* c = start;
	goto find_quote_end_start;
find_quote_end_l0:
	++c;
find_quote_end_start:
	if (c == end) return out;
	if (*c < 11) 
		if (*c < 10) goto find_quote_end_l1;
		else goto find_quote_end_l2;
	else goto find_quote_end_l1;
find_quote_end_l1:
	return out;
find_quote_end_l2:
	++c;
	if (c == end) return out;
	if (*c < 91) 
		if (*c < 63) 
			if (*c < 62) goto find_quote_end_l1;
			else goto find_quote_end_l3;
		else 
			if (*c < 65) goto find_quote_end_l1;
			else goto find_quote_end_l5;
	else 
		if (*c < 123) 
			if (*c < 97) goto find_quote_end_l1;
			else goto find_quote_end_l5;
		else goto find_quote_end_l1;
find_quote_end_l3:
	++c;
	if (c == end) return out;
	if (*c < 33) 
		if (*c < 32) goto find_quote_end_l1;
		else goto find_quote_end_l4;
	else goto find_quote_end_l1;
find_quote_end_l4:
	++c;
	out = c;
	if (c == end) return out;
	if (*c < 11) 
		if (*c < 10) goto find_quote_end_l4;
		else goto find_quote_end_l2;
	else goto find_quote_end_l4;
find_quote_end_l5:
	++c;
	if (c == end) return out;
	if (*c < 91) 
		if (*c < 63) 
			if (*c < 62) goto find_quote_end_l1;
			else goto find_quote_end_l3;
		else 
			if (*c < 65) goto find_quote_end_l1;
			else goto find_quote_end_l6;
	else 
		if (*c < 123) 
			if (*c < 97) goto find_quote_end_l1;
			else goto find_quote_end_l6;
		else goto find_quote_end_l1;
find_quote_end_l6:
	++c;
	if (c == end) return out;
	if (*c < 91) 
		if (*c < 63) 
			if (*c < 62) goto find_quote_end_l1;
			else goto find_quote_end_l3;
		else 
			if (*c < 65) goto find_quote_end_l1;
			else goto find_quote_end_l7;
	else 
		if (*c < 123) 
			if (*c < 97) goto find_quote_end_l1;
			else goto find_quote_end_l7;
		else goto find_quote_end_l1;
find_quote_end_l7:
	++c;
	if (c == end) return out;
	if (*c < 91) 
		if (*c < 63) 
			if (*c < 62) goto find_quote_end_l1;
			else goto find_quote_end_l3;
		else 
			if (*c < 65) goto find_quote_end_l1;
			else goto find_quote_end_l8;
	else 
		if (*c < 123) 
			if (*c < 97) goto find_quote_end_l1;
			else goto find_quote_end_l8;
		else goto find_quote_end_l1;
find_quote_end_l8:
	++c;
	if (c == end) return out;
	if (*c < 63) 
		if (*c < 62) goto find_quote_end_l1;
		else goto find_quote_end_l3;
	else goto find_quote_end_l1;
}
char* find_quote_starts(const char* start, const char* end, char* scratch) {
	const char* c = end;
	scratch += (end - start);
	goto find_quote_starts_start;
find_quote_starts_l0:
	*--scratch = 0;
find_quote_starts_start:
	if (c-- == start) return scratch;
	if (*c < 33) 
		if (*c < 32) goto find_quote_starts_l0;
		else goto find_quote_starts_l1;
	else goto find_quote_starts_l0;
find_quote_starts_l1:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 62) 
		if (*c < 33) 
			if (*c < 32) goto find_quote_starts_l0;
			else goto find_quote_starts_l1;
		else goto find_quote_starts_l0;
	else 
		if (*c < 63) goto find_quote_starts_l2;
		else goto find_quote_starts_l0;
find_quote_starts_l2:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 65) 
		if (*c < 32) 
			if (*c < 11) 
				if (*c < 10) goto find_quote_starts_l0;
				else goto find_quote_starts_l3;
			else goto find_quote_starts_l0;
		else 
			if (*c < 33) goto find_quote_starts_l1;
			else goto find_quote_starts_l0;
	else 
		if (*c < 97) 
			if (*c < 91) goto find_quote_starts_l4;
			else goto find_quote_starts_l0;
		else 
			if (*c < 123) goto find_quote_starts_l4;
			else goto find_quote_starts_l0;
find_quote_starts_l3:
	*--scratch = 1;
	if (c-- == start) return scratch;
	if (*c < 33) 
		if (*c < 32) goto find_quote_starts_l0;
		else goto find_quote_starts_l1;
	else goto find_quote_starts_l0;
find_quote_starts_l4:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 65) 
		if (*c < 32) 
			if (*c < 11) 
				if (*c < 10) goto find_quote_starts_l0;
				else goto find_quote_starts_l3;
			else goto find_quote_starts_l0;
		else 
			if (*c < 33) goto find_quote_starts_l1;
			else goto find_quote_starts_l0;
	else 
		if (*c < 97) 
			if (*c < 91) goto find_quote_starts_l5;
			else goto find_quote_starts_l0;
		else 
			if (*c < 123) goto find_quote_starts_l5;
			else goto find_quote_starts_l0;
find_quote_starts_l5:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 65) 
		if (*c < 32) 
			if (*c < 11) 
				if (*c < 10) goto find_quote_starts_l0;
				else goto find_quote_starts_l3;
			else goto find_quote_starts_l0;
		else 
			if (*c < 33) goto find_quote_starts_l1;
			else goto find_quote_starts_l0;
	else 
		if (*c < 97) 
			if (*c < 91) goto find_quote_starts_l6;
			else goto find_quote_starts_l0;
		else 
			if (*c < 123) goto find_quote_starts_l6;
			else goto find_quote_starts_l0;
find_quote_starts_l6:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 65) 
		if (*c < 32) 
			if (*c < 11) 
				if (*c < 10) goto find_quote_starts_l0;
				else goto find_quote_starts_l3;
			else goto find_quote_starts_l0;
		else 
			if (*c < 33) goto find_quote_starts_l1;
			else goto find_quote_starts_l0;
	else 
		if (*c < 97) 
			if (*c < 91) goto find_quote_starts_l7;
			else goto find_quote_starts_l0;
		else 
			if (*c < 123) goto find_quote_starts_l7;
			else goto find_quote_starts_l0;
find_quote_starts_l7:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 32) 
		if (*c < 11) 
			if (*c < 10) goto find_quote_starts_l0;
			else goto find_quote_starts_l3;
		else goto find_quote_starts_l0;
	else 
		if (*c < 33) goto find_quote_starts_l1;
		else goto find_quote_starts_l0;
}
