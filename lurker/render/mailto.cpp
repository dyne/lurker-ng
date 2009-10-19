const char* find_email_end(const char* start, const char* end) {
	const char* out = 0;
	const char* c = start;
	goto find_email_end_start;
//find_email_end_l0:
	++c;
find_email_end_start:
	if (c == end) return out;
	if (*c < 65) 
		if (*c < 48) 
			if (*c < 35) 
				if (*c < 34) goto find_email_end_l1;
				else goto find_email_end_l2;
			else goto find_email_end_l1;
		else 
			if (*c < 58) goto find_email_end_l39;
			else goto find_email_end_l1;
	else 
		if (*c < 97) 
			if (*c < 91) goto find_email_end_l39;
			else goto find_email_end_l1;
		else 
			if (*c < 123) goto find_email_end_l39;
			else goto find_email_end_l1;
find_email_end_l1:
	return out;
find_email_end_l2:
	++c;
	if (c == end) return out;
	if (*c < 14) 
		if (*c < 10) 
			if (*c < 1) 
				if (*c < 0) goto find_email_end_l2;
				else goto find_email_end_l1;
			else goto find_email_end_l2;
		else 
			if (*c < 13) 
				if (*c < 11) goto find_email_end_l1;
				else goto find_email_end_l2;
			else goto find_email_end_l3;
	else 
		if (*c < 92) 
			if (*c < 35) 
				if (*c < 34) goto find_email_end_l2;
				else goto find_email_end_l6;
			else goto find_email_end_l2;
		else 
			if (*c < 93) goto find_email_end_l38;
			else goto find_email_end_l2;
find_email_end_l3:
	++c;
	if (c == end) return out;
	if (*c < 11) 
		if (*c < 10) goto find_email_end_l1;
		else goto find_email_end_l4;
	else goto find_email_end_l1;
find_email_end_l4:
	++c;
	if (c == end) return out;
	if (*c < 32) 
		if (*c < 10) 
			if (*c < 9) goto find_email_end_l1;
			else goto find_email_end_l5;
		else goto find_email_end_l1;
	else 
		if (*c < 33) goto find_email_end_l5;
		else goto find_email_end_l1;
find_email_end_l5:
	++c;
	if (c == end) return out;
	if (*c < 14) 
		if (*c < 10) 
			if (*c < 1) 
				if (*c < 0) goto find_email_end_l2;
				else goto find_email_end_l1;
			else 
				if (*c < 9) goto find_email_end_l2;
				else goto find_email_end_l5;
		else 
			if (*c < 13) 
				if (*c < 11) goto find_email_end_l1;
				else goto find_email_end_l2;
			else goto find_email_end_l1;
	else 
		if (*c < 35) 
			if (*c < 33) 
				if (*c < 32) goto find_email_end_l2;
				else goto find_email_end_l5;
			else 
				if (*c < 34) goto find_email_end_l2;
				else goto find_email_end_l6;
		else 
			if (*c < 93) 
				if (*c < 92) goto find_email_end_l2;
				else goto find_email_end_l38;
			else goto find_email_end_l2;
find_email_end_l6:
	++c;
	if (c == end) return out;
	if (*c < 65) 
		if (*c < 64) goto find_email_end_l1;
		else goto find_email_end_l7;
	else goto find_email_end_l1;
find_email_end_l7:
	++c;
	if (c == end) return out;
	if (*c < 91) 
		if (*c < 58) 
			if (*c < 48) goto find_email_end_l1;
			else goto find_email_end_l8;
		else 
			if (*c < 65) goto find_email_end_l1;
			else goto find_email_end_l8;
	else 
		if (*c < 97) 
			if (*c < 92) goto find_email_end_l13;
			else goto find_email_end_l1;
		else 
			if (*c < 123) goto find_email_end_l8;
			else goto find_email_end_l1;
find_email_end_l8:
	++c;
	if (c == end) return out;
	if (*c < 58) 
		if (*c < 47) 
			if (*c < 46) 
				if (*c < 45) goto find_email_end_l1;
				else goto find_email_end_l9;
			else goto find_email_end_l10;
		else 
			if (*c < 48) goto find_email_end_l1;
			else goto find_email_end_l8;
	else 
		if (*c < 97) 
			if (*c < 91) 
				if (*c < 65) goto find_email_end_l1;
				else goto find_email_end_l8;
			else goto find_email_end_l1;
		else 
			if (*c < 123) goto find_email_end_l8;
			else goto find_email_end_l1;
find_email_end_l9:
	++c;
	if (c == end) return out;
	if (*c < 65) 
		if (*c < 48) 
			if (*c < 46) 
				if (*c < 45) goto find_email_end_l1;
				else goto find_email_end_l9;
			else goto find_email_end_l1;
		else 
			if (*c < 58) goto find_email_end_l8;
			else goto find_email_end_l1;
	else 
		if (*c < 97) 
			if (*c < 91) goto find_email_end_l8;
			else goto find_email_end_l1;
		else 
			if (*c < 123) goto find_email_end_l8;
			else goto find_email_end_l1;
find_email_end_l10:
	++c;
	if (c == end) return out;
	if (*c < 91) 
		if (*c < 58) 
			if (*c < 48) goto find_email_end_l1;
			else goto find_email_end_l8;
		else 
			if (*c < 65) goto find_email_end_l1;
			else goto find_email_end_l11;
	else 
		if (*c < 123) 
			if (*c < 97) goto find_email_end_l1;
			else goto find_email_end_l11;
		else goto find_email_end_l1;
find_email_end_l11:
	++c;
	out = c;
	if (c == end) return out;
	if (*c < 58) 
		if (*c < 47) 
			if (*c < 46) 
				if (*c < 45) goto find_email_end_l1;
				else goto find_email_end_l12;
			else goto find_email_end_l10;
		else 
			if (*c < 48) goto find_email_end_l1;
			else goto find_email_end_l11;
	else 
		if (*c < 97) 
			if (*c < 91) 
				if (*c < 65) goto find_email_end_l1;
				else goto find_email_end_l11;
			else goto find_email_end_l1;
		else 
			if (*c < 123) goto find_email_end_l11;
			else goto find_email_end_l1;
find_email_end_l12:
	++c;
	if (c == end) return out;
	if (*c < 65) 
		if (*c < 48) 
			if (*c < 46) 
				if (*c < 45) goto find_email_end_l1;
				else goto find_email_end_l12;
			else goto find_email_end_l1;
		else 
			if (*c < 58) goto find_email_end_l11;
			else goto find_email_end_l1;
	else 
		if (*c < 97) 
			if (*c < 91) goto find_email_end_l11;
			else goto find_email_end_l1;
		else 
			if (*c < 123) goto find_email_end_l11;
			else goto find_email_end_l1;
find_email_end_l13:
	++c;
	if (c == end) return out;
	if (*c < 50) 
		if (*c < 49) 
			if (*c < 48) goto find_email_end_l1;
			else goto find_email_end_l14;
		else goto find_email_end_l34;
	else 
		if (*c < 58) 
			if (*c < 51) goto find_email_end_l36;
			else goto find_email_end_l35;
		else goto find_email_end_l1;
find_email_end_l14:
	++c;
	if (c == end) return out;
	if (*c < 47) 
		if (*c < 46) goto find_email_end_l1;
		else goto find_email_end_l15;
	else goto find_email_end_l1;
find_email_end_l15:
	++c;
	if (c == end) return out;
	if (*c < 50) 
		if (*c < 49) 
			if (*c < 48) goto find_email_end_l1;
			else goto find_email_end_l16;
		else goto find_email_end_l30;
	else 
		if (*c < 58) 
			if (*c < 51) goto find_email_end_l32;
			else goto find_email_end_l31;
		else goto find_email_end_l1;
find_email_end_l16:
	++c;
	if (c == end) return out;
	if (*c < 47) 
		if (*c < 46) goto find_email_end_l1;
		else goto find_email_end_l17;
	else goto find_email_end_l1;
find_email_end_l17:
	++c;
	if (c == end) return out;
	if (*c < 50) 
		if (*c < 49) 
			if (*c < 48) goto find_email_end_l1;
			else goto find_email_end_l18;
		else goto find_email_end_l26;
	else 
		if (*c < 58) 
			if (*c < 51) goto find_email_end_l28;
			else goto find_email_end_l27;
		else goto find_email_end_l1;
find_email_end_l18:
	++c;
	if (c == end) return out;
	if (*c < 47) 
		if (*c < 46) goto find_email_end_l1;
		else goto find_email_end_l19;
	else goto find_email_end_l1;
find_email_end_l19:
	++c;
	if (c == end) return out;
	if (*c < 50) 
		if (*c < 49) 
			if (*c < 48) goto find_email_end_l1;
			else goto find_email_end_l20;
		else goto find_email_end_l22;
	else 
		if (*c < 58) 
			if (*c < 51) goto find_email_end_l24;
			else goto find_email_end_l23;
		else goto find_email_end_l1;
find_email_end_l20:
	++c;
	if (c == end) return out;
	if (*c < 94) 
		if (*c < 93) goto find_email_end_l1;
		else goto find_email_end_l21;
	else goto find_email_end_l1;
find_email_end_l21:
	++c;
	out = c;
	if (c == end) return out;
	goto find_email_end_l1;
find_email_end_l22:
	++c;
	if (c == end) return out;
	if (*c < 93) 
		if (*c < 58) 
			if (*c < 48) goto find_email_end_l1;
			else goto find_email_end_l23;
		else goto find_email_end_l1;
	else 
		if (*c < 94) goto find_email_end_l21;
		else goto find_email_end_l1;
find_email_end_l23:
	++c;
	if (c == end) return out;
	if (*c < 93) 
		if (*c < 58) 
			if (*c < 48) goto find_email_end_l1;
			else goto find_email_end_l20;
		else goto find_email_end_l1;
	else 
		if (*c < 94) goto find_email_end_l21;
		else goto find_email_end_l1;
find_email_end_l24:
	++c;
	if (c == end) return out;
	if (*c < 58) 
		if (*c < 53) 
			if (*c < 48) goto find_email_end_l1;
			else goto find_email_end_l23;
		else 
			if (*c < 54) goto find_email_end_l25;
			else goto find_email_end_l20;
	else 
		if (*c < 94) 
			if (*c < 93) goto find_email_end_l1;
			else goto find_email_end_l21;
		else goto find_email_end_l1;
find_email_end_l25:
	++c;
	if (c == end) return out;
	if (*c < 93) 
		if (*c < 54) 
			if (*c < 48) goto find_email_end_l1;
			else goto find_email_end_l20;
		else goto find_email_end_l1;
	else 
		if (*c < 94) goto find_email_end_l21;
		else goto find_email_end_l1;
find_email_end_l26:
	++c;
	if (c == end) return out;
	if (*c < 48) 
		if (*c < 47) 
			if (*c < 46) goto find_email_end_l1;
			else goto find_email_end_l19;
		else goto find_email_end_l1;
	else 
		if (*c < 58) goto find_email_end_l27;
		else goto find_email_end_l1;
find_email_end_l27:
	++c;
	if (c == end) return out;
	if (*c < 48) 
		if (*c < 47) 
			if (*c < 46) goto find_email_end_l1;
			else goto find_email_end_l19;
		else goto find_email_end_l1;
	else 
		if (*c < 58) goto find_email_end_l18;
		else goto find_email_end_l1;
find_email_end_l28:
	++c;
	if (c == end) return out;
	if (*c < 53) 
		if (*c < 47) 
			if (*c < 46) goto find_email_end_l1;
			else goto find_email_end_l19;
		else 
			if (*c < 48) goto find_email_end_l1;
			else goto find_email_end_l27;
	else 
		if (*c < 58) 
			if (*c < 54) goto find_email_end_l29;
			else goto find_email_end_l18;
		else goto find_email_end_l1;
find_email_end_l29:
	++c;
	if (c == end) return out;
	if (*c < 48) 
		if (*c < 47) 
			if (*c < 46) goto find_email_end_l1;
			else goto find_email_end_l19;
		else goto find_email_end_l1;
	else 
		if (*c < 54) goto find_email_end_l18;
		else goto find_email_end_l1;
find_email_end_l30:
	++c;
	if (c == end) return out;
	if (*c < 48) 
		if (*c < 47) 
			if (*c < 46) goto find_email_end_l1;
			else goto find_email_end_l17;
		else goto find_email_end_l1;
	else 
		if (*c < 58) goto find_email_end_l31;
		else goto find_email_end_l1;
find_email_end_l31:
	++c;
	if (c == end) return out;
	if (*c < 48) 
		if (*c < 47) 
			if (*c < 46) goto find_email_end_l1;
			else goto find_email_end_l17;
		else goto find_email_end_l1;
	else 
		if (*c < 58) goto find_email_end_l16;
		else goto find_email_end_l1;
find_email_end_l32:
	++c;
	if (c == end) return out;
	if (*c < 53) 
		if (*c < 47) 
			if (*c < 46) goto find_email_end_l1;
			else goto find_email_end_l17;
		else 
			if (*c < 48) goto find_email_end_l1;
			else goto find_email_end_l31;
	else 
		if (*c < 58) 
			if (*c < 54) goto find_email_end_l33;
			else goto find_email_end_l16;
		else goto find_email_end_l1;
find_email_end_l33:
	++c;
	if (c == end) return out;
	if (*c < 48) 
		if (*c < 47) 
			if (*c < 46) goto find_email_end_l1;
			else goto find_email_end_l17;
		else goto find_email_end_l1;
	else 
		if (*c < 54) goto find_email_end_l16;
		else goto find_email_end_l1;
find_email_end_l34:
	++c;
	if (c == end) return out;
	if (*c < 48) 
		if (*c < 47) 
			if (*c < 46) goto find_email_end_l1;
			else goto find_email_end_l15;
		else goto find_email_end_l1;
	else 
		if (*c < 58) goto find_email_end_l35;
		else goto find_email_end_l1;
find_email_end_l35:
	++c;
	if (c == end) return out;
	if (*c < 48) 
		if (*c < 47) 
			if (*c < 46) goto find_email_end_l1;
			else goto find_email_end_l15;
		else goto find_email_end_l1;
	else 
		if (*c < 58) goto find_email_end_l14;
		else goto find_email_end_l1;
find_email_end_l36:
	++c;
	if (c == end) return out;
	if (*c < 53) 
		if (*c < 47) 
			if (*c < 46) goto find_email_end_l1;
			else goto find_email_end_l15;
		else 
			if (*c < 48) goto find_email_end_l1;
			else goto find_email_end_l35;
	else 
		if (*c < 58) 
			if (*c < 54) goto find_email_end_l37;
			else goto find_email_end_l14;
		else goto find_email_end_l1;
find_email_end_l37:
	++c;
	if (c == end) return out;
	if (*c < 48) 
		if (*c < 47) 
			if (*c < 46) goto find_email_end_l1;
			else goto find_email_end_l15;
		else goto find_email_end_l1;
	else 
		if (*c < 54) goto find_email_end_l14;
		else goto find_email_end_l1;
find_email_end_l38:
	++c;
	if (c == end) return out;
	if (*c < 11) 
		if (*c < 1) 
			if (*c < 0) goto find_email_end_l2;
			else goto find_email_end_l1;
		else 
			if (*c < 10) goto find_email_end_l2;
			else goto find_email_end_l1;
	else 
		if (*c < 14) 
			if (*c < 13) goto find_email_end_l2;
			else goto find_email_end_l1;
		else goto find_email_end_l2;
find_email_end_l39:
	++c;
	if (c == end) return out;
	if (*c < 47) 
		if (*c < 42) 
			if (*c < 35) 
				if (*c < 34) 
					if (*c < 33) goto find_email_end_l1;
					else goto find_email_end_l39;
				else goto find_email_end_l1;
			else 
				if (*c < 40) goto find_email_end_l39;
				else goto find_email_end_l1;
		else 
			if (*c < 45) 
				if (*c < 44) goto find_email_end_l39;
				else goto find_email_end_l1;
			else 
				if (*c < 46) goto find_email_end_l39;
				else goto find_email_end_l40;
	else 
		if (*c < 65) 
			if (*c < 63) 
				if (*c < 58) goto find_email_end_l39;
				else goto find_email_end_l1;
			else 
				if (*c < 64) goto find_email_end_l39;
				else goto find_email_end_l7;
		else 
			if (*c < 94) 
				if (*c < 91) goto find_email_end_l39;
				else goto find_email_end_l1;
			else 
				if (*c < 127) goto find_email_end_l39;
				else goto find_email_end_l1;
find_email_end_l40:
	++c;
	if (c == end) return out;
	if (*c < 47) 
		if (*c < 42) 
			if (*c < 35) 
				if (*c < 34) 
					if (*c < 33) goto find_email_end_l1;
					else goto find_email_end_l39;
				else goto find_email_end_l1;
			else 
				if (*c < 40) goto find_email_end_l39;
				else goto find_email_end_l1;
		else 
			if (*c < 45) 
				if (*c < 44) goto find_email_end_l39;
				else goto find_email_end_l1;
			else 
				if (*c < 46) goto find_email_end_l39;
				else goto find_email_end_l1;
	else 
		if (*c < 65) 
			if (*c < 63) 
				if (*c < 58) goto find_email_end_l39;
				else goto find_email_end_l1;
			else 
				if (*c < 64) goto find_email_end_l39;
				else goto find_email_end_l1;
		else 
			if (*c < 94) 
				if (*c < 91) goto find_email_end_l39;
				else goto find_email_end_l1;
			else 
				if (*c < 127) goto find_email_end_l39;
				else goto find_email_end_l1;
}
char* find_email_starts(const char* start, const char* end, char* scratch) {
	const char* c = end;
	scratch += (end - start);
	goto find_email_starts_start;
find_email_starts_l0:
	*--scratch = 0;
find_email_starts_start:
	if (c-- == start) return scratch;
	if (*c < 94) 
		if (*c < 91) 
			if (*c < 65) goto find_email_starts_l0;
			else goto find_email_starts_l1;
		else 
			if (*c < 93) goto find_email_starts_l0;
			else goto find_email_starts_l5;
	else 
		if (*c < 123) 
			if (*c < 97) goto find_email_starts_l0;
			else goto find_email_starts_l1;
		else goto find_email_starts_l0;
find_email_starts_l1:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 93) 
		if (*c < 65) 
			if (*c < 47) 
				if (*c < 46) goto find_email_starts_l0;
				else goto find_email_starts_l2;
			else goto find_email_starts_l0;
		else 
			if (*c < 91) goto find_email_starts_l1;
			else goto find_email_starts_l0;
	else 
		if (*c < 97) 
			if (*c < 94) goto find_email_starts_l5;
			else goto find_email_starts_l0;
		else 
			if (*c < 123) goto find_email_starts_l1;
			else goto find_email_starts_l0;
find_email_starts_l2:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 93) 
		if (*c < 65) 
			if (*c < 58) 
				if (*c < 48) goto find_email_starts_l0;
				else goto find_email_starts_l3;
			else goto find_email_starts_l0;
		else 
			if (*c < 91) goto find_email_starts_l3;
			else goto find_email_starts_l0;
	else 
		if (*c < 97) 
			if (*c < 94) goto find_email_starts_l5;
			else goto find_email_starts_l0;
		else 
			if (*c < 123) goto find_email_starts_l3;
			else goto find_email_starts_l0;
find_email_starts_l3:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 65) 
		if (*c < 48) 
			if (*c < 46) 
				if (*c < 45) goto find_email_starts_l0;
				else goto find_email_starts_l4;
			else 
				if (*c < 47) goto find_email_starts_l2;
				else goto find_email_starts_l0;
		else 
			if (*c < 64) 
				if (*c < 58) goto find_email_starts_l3;
				else goto find_email_starts_l0;
			else goto find_email_starts_l16;
	else 
		if (*c < 94) 
			if (*c < 93) 
				if (*c < 91) goto find_email_starts_l3;
				else goto find_email_starts_l0;
			else goto find_email_starts_l5;
		else 
			if (*c < 123) 
				if (*c < 97) goto find_email_starts_l0;
				else goto find_email_starts_l3;
			else goto find_email_starts_l0;
find_email_starts_l4:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 91) 
		if (*c < 48) 
			if (*c < 46) 
				if (*c < 45) goto find_email_starts_l0;
				else goto find_email_starts_l4;
			else goto find_email_starts_l0;
		else 
			if (*c < 65) 
				if (*c < 58) goto find_email_starts_l3;
				else goto find_email_starts_l0;
			else goto find_email_starts_l3;
	else 
		if (*c < 97) 
			if (*c < 94) 
				if (*c < 93) goto find_email_starts_l0;
				else goto find_email_starts_l5;
			else goto find_email_starts_l0;
		else 
			if (*c < 123) goto find_email_starts_l3;
			else goto find_email_starts_l0;
find_email_starts_l5:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 91) 
		if (*c < 58) 
			if (*c < 54) 
				if (*c < 48) goto find_email_starts_l0;
				else goto find_email_starts_l6;
			else goto find_email_starts_l91;
		else 
			if (*c < 65) goto find_email_starts_l0;
			else goto find_email_starts_l1;
	else 
		if (*c < 97) 
			if (*c < 94) 
				if (*c < 93) goto find_email_starts_l0;
				else goto find_email_starts_l5;
			else goto find_email_starts_l0;
		else 
			if (*c < 123) goto find_email_starts_l1;
			else goto find_email_starts_l0;
find_email_starts_l6:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 65) 
		if (*c < 49) 
			if (*c < 47) 
				if (*c < 46) goto find_email_starts_l0;
				else goto find_email_starts_l7;
			else 
				if (*c < 48) goto find_email_starts_l0;
				else goto find_email_starts_l87;
		else 
			if (*c < 58) 
				if (*c < 54) goto find_email_starts_l89;
				else goto find_email_starts_l90;
			else goto find_email_starts_l0;
	else 
		if (*c < 94) 
			if (*c < 93) 
				if (*c < 91) goto find_email_starts_l1;
				else goto find_email_starts_l0;
			else goto find_email_starts_l5;
		else 
			if (*c < 123) 
				if (*c < 97) goto find_email_starts_l0;
				else goto find_email_starts_l1;
			else goto find_email_starts_l0;
find_email_starts_l7:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 91) 
		if (*c < 58) 
			if (*c < 54) 
				if (*c < 48) goto find_email_starts_l0;
				else goto find_email_starts_l8;
			else goto find_email_starts_l86;
		else 
			if (*c < 65) goto find_email_starts_l0;
			else goto find_email_starts_l1;
	else 
		if (*c < 97) 
			if (*c < 94) 
				if (*c < 93) goto find_email_starts_l0;
				else goto find_email_starts_l5;
			else goto find_email_starts_l0;
		else 
			if (*c < 123) goto find_email_starts_l1;
			else goto find_email_starts_l0;
find_email_starts_l8:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 65) 
		if (*c < 49) 
			if (*c < 47) 
				if (*c < 46) goto find_email_starts_l0;
				else goto find_email_starts_l9;
			else 
				if (*c < 48) goto find_email_starts_l0;
				else goto find_email_starts_l82;
		else 
			if (*c < 58) 
				if (*c < 54) goto find_email_starts_l84;
				else goto find_email_starts_l85;
			else goto find_email_starts_l0;
	else 
		if (*c < 94) 
			if (*c < 93) 
				if (*c < 91) goto find_email_starts_l1;
				else goto find_email_starts_l0;
			else goto find_email_starts_l5;
		else 
			if (*c < 123) 
				if (*c < 97) goto find_email_starts_l0;
				else goto find_email_starts_l1;
			else goto find_email_starts_l0;
find_email_starts_l9:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 91) 
		if (*c < 58) 
			if (*c < 54) 
				if (*c < 48) goto find_email_starts_l0;
				else goto find_email_starts_l10;
			else goto find_email_starts_l81;
		else 
			if (*c < 65) goto find_email_starts_l0;
			else goto find_email_starts_l1;
	else 
		if (*c < 97) 
			if (*c < 94) 
				if (*c < 93) goto find_email_starts_l0;
				else goto find_email_starts_l5;
			else goto find_email_starts_l0;
		else 
			if (*c < 123) goto find_email_starts_l1;
			else goto find_email_starts_l0;
find_email_starts_l10:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 65) 
		if (*c < 49) 
			if (*c < 47) 
				if (*c < 46) goto find_email_starts_l0;
				else goto find_email_starts_l11;
			else 
				if (*c < 48) goto find_email_starts_l0;
				else goto find_email_starts_l77;
		else 
			if (*c < 58) 
				if (*c < 54) goto find_email_starts_l79;
				else goto find_email_starts_l80;
			else goto find_email_starts_l0;
	else 
		if (*c < 94) 
			if (*c < 93) 
				if (*c < 91) goto find_email_starts_l1;
				else goto find_email_starts_l0;
			else goto find_email_starts_l5;
		else 
			if (*c < 123) 
				if (*c < 97) goto find_email_starts_l0;
				else goto find_email_starts_l1;
			else goto find_email_starts_l0;
find_email_starts_l11:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 91) 
		if (*c < 58) 
			if (*c < 54) 
				if (*c < 48) goto find_email_starts_l0;
				else goto find_email_starts_l12;
			else goto find_email_starts_l76;
		else 
			if (*c < 65) goto find_email_starts_l0;
			else goto find_email_starts_l1;
	else 
		if (*c < 97) 
			if (*c < 94) 
				if (*c < 93) goto find_email_starts_l0;
				else goto find_email_starts_l5;
			else goto find_email_starts_l0;
		else 
			if (*c < 123) goto find_email_starts_l1;
			else goto find_email_starts_l0;
find_email_starts_l12:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 91) 
		if (*c < 54) 
			if (*c < 49) 
				if (*c < 48) goto find_email_starts_l0;
				else goto find_email_starts_l13;
			else goto find_email_starts_l74;
		else 
			if (*c < 65) 
				if (*c < 58) goto find_email_starts_l75;
				else goto find_email_starts_l0;
			else goto find_email_starts_l1;
	else 
		if (*c < 94) 
			if (*c < 93) 
				if (*c < 92) goto find_email_starts_l15;
				else goto find_email_starts_l0;
			else goto find_email_starts_l5;
		else 
			if (*c < 123) 
				if (*c < 97) goto find_email_starts_l0;
				else goto find_email_starts_l1;
			else goto find_email_starts_l0;
find_email_starts_l13:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 93) 
		if (*c < 65) 
			if (*c < 51) 
				if (*c < 49) goto find_email_starts_l0;
				else goto find_email_starts_l14;
			else goto find_email_starts_l0;
		else 
			if (*c < 91) goto find_email_starts_l1;
			else goto find_email_starts_l0;
	else 
		if (*c < 97) 
			if (*c < 94) goto find_email_starts_l5;
			else goto find_email_starts_l0;
		else 
			if (*c < 123) goto find_email_starts_l1;
			else goto find_email_starts_l0;
find_email_starts_l14:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 93) 
		if (*c < 91) 
			if (*c < 65) goto find_email_starts_l0;
			else goto find_email_starts_l1;
		else 
			if (*c < 92) goto find_email_starts_l15;
			else goto find_email_starts_l0;
	else 
		if (*c < 97) 
			if (*c < 94) goto find_email_starts_l5;
			else goto find_email_starts_l0;
		else 
			if (*c < 123) goto find_email_starts_l1;
			else goto find_email_starts_l0;
find_email_starts_l15:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 93) 
		if (*c < 65) 
			if (*c < 64) goto find_email_starts_l0;
			else goto find_email_starts_l16;
		else 
			if (*c < 91) goto find_email_starts_l1;
			else goto find_email_starts_l0;
	else 
		if (*c < 97) 
			if (*c < 94) goto find_email_starts_l5;
			else goto find_email_starts_l0;
		else 
			if (*c < 123) goto find_email_starts_l1;
			else goto find_email_starts_l0;
find_email_starts_l16:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 58) 
		if (*c < 44) 
			if (*c < 35) 
				if (*c < 34) 
					if (*c < 33) goto find_email_starts_l0;
					else goto find_email_starts_l17;
				else goto find_email_starts_l24;
			else 
				if (*c < 42) 
					if (*c < 40) goto find_email_starts_l17;
					else goto find_email_starts_l0;
				else goto find_email_starts_l17;
		else 
			if (*c < 47) 
				if (*c < 46) 
					if (*c < 45) goto find_email_starts_l0;
					else goto find_email_starts_l17;
				else goto find_email_starts_l0;
			else 
				if (*c < 48) goto find_email_starts_l17;
				else goto find_email_starts_l19;
	else 
		if (*c < 93) 
			if (*c < 65) 
				if (*c < 64) 
					if (*c < 63) goto find_email_starts_l0;
					else goto find_email_starts_l17;
				else goto find_email_starts_l0;
			else 
				if (*c < 91) goto find_email_starts_l20;
				else goto find_email_starts_l0;
		else 
			if (*c < 123) 
				if (*c < 97) 
					if (*c < 94) goto find_email_starts_l5;
					else goto find_email_starts_l17;
				else goto find_email_starts_l20;
			else 
				if (*c < 127) goto find_email_starts_l17;
				else goto find_email_starts_l0;
find_email_starts_l17:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 58) 
		if (*c < 44) 
			if (*c < 35) 
				if (*c < 34) 
					if (*c < 33) goto find_email_starts_l0;
					else goto find_email_starts_l17;
				else goto find_email_starts_l0;
			else 
				if (*c < 42) 
					if (*c < 40) goto find_email_starts_l17;
					else goto find_email_starts_l0;
				else goto find_email_starts_l17;
		else 
			if (*c < 47) 
				if (*c < 46) 
					if (*c < 45) goto find_email_starts_l0;
					else goto find_email_starts_l17;
				else goto find_email_starts_l18;
			else 
				if (*c < 48) goto find_email_starts_l17;
				else goto find_email_starts_l19;
	else 
		if (*c < 93) 
			if (*c < 65) 
				if (*c < 64) 
					if (*c < 63) goto find_email_starts_l0;
					else goto find_email_starts_l17;
				else goto find_email_starts_l0;
			else 
				if (*c < 91) goto find_email_starts_l20;
				else goto find_email_starts_l0;
		else 
			if (*c < 123) 
				if (*c < 97) 
					if (*c < 94) goto find_email_starts_l5;
					else goto find_email_starts_l17;
				else goto find_email_starts_l20;
			else 
				if (*c < 127) goto find_email_starts_l17;
				else goto find_email_starts_l0;
find_email_starts_l18:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 58) 
		if (*c < 44) 
			if (*c < 35) 
				if (*c < 34) 
					if (*c < 33) goto find_email_starts_l0;
					else goto find_email_starts_l17;
				else goto find_email_starts_l0;
			else 
				if (*c < 42) 
					if (*c < 40) goto find_email_starts_l17;
					else goto find_email_starts_l0;
				else goto find_email_starts_l17;
		else 
			if (*c < 47) 
				if (*c < 46) 
					if (*c < 45) goto find_email_starts_l0;
					else goto find_email_starts_l17;
				else goto find_email_starts_l0;
			else 
				if (*c < 48) goto find_email_starts_l17;
				else goto find_email_starts_l19;
	else 
		if (*c < 93) 
			if (*c < 65) 
				if (*c < 64) 
					if (*c < 63) goto find_email_starts_l0;
					else goto find_email_starts_l17;
				else goto find_email_starts_l0;
			else 
				if (*c < 91) goto find_email_starts_l20;
				else goto find_email_starts_l0;
		else 
			if (*c < 123) 
				if (*c < 97) 
					if (*c < 94) goto find_email_starts_l5;
					else goto find_email_starts_l17;
				else goto find_email_starts_l20;
			else 
				if (*c < 127) goto find_email_starts_l17;
				else goto find_email_starts_l0;
find_email_starts_l19:
	*--scratch = 1;
	if (c-- == start) return scratch;
	if (*c < 58) 
		if (*c < 44) 
			if (*c < 35) 
				if (*c < 34) 
					if (*c < 33) goto find_email_starts_l0;
					else goto find_email_starts_l17;
				else goto find_email_starts_l0;
			else 
				if (*c < 42) 
					if (*c < 40) goto find_email_starts_l17;
					else goto find_email_starts_l0;
				else goto find_email_starts_l17;
		else 
			if (*c < 47) 
				if (*c < 46) 
					if (*c < 45) goto find_email_starts_l0;
					else goto find_email_starts_l17;
				else goto find_email_starts_l18;
			else 
				if (*c < 48) goto find_email_starts_l17;
				else goto find_email_starts_l19;
	else 
		if (*c < 93) 
			if (*c < 65) 
				if (*c < 64) 
					if (*c < 63) goto find_email_starts_l0;
					else goto find_email_starts_l17;
				else goto find_email_starts_l0;
			else 
				if (*c < 91) goto find_email_starts_l20;
				else goto find_email_starts_l0;
		else 
			if (*c < 123) 
				if (*c < 97) 
					if (*c < 94) goto find_email_starts_l5;
					else goto find_email_starts_l17;
				else goto find_email_starts_l20;
			else 
				if (*c < 127) goto find_email_starts_l17;
				else goto find_email_starts_l0;
find_email_starts_l20:
	*--scratch = 1;
	if (c-- == start) return scratch;
	if (*c < 58) 
		if (*c < 44) 
			if (*c < 35) 
				if (*c < 34) 
					if (*c < 33) goto find_email_starts_l0;
					else goto find_email_starts_l17;
				else goto find_email_starts_l0;
			else 
				if (*c < 42) 
					if (*c < 40) goto find_email_starts_l17;
					else goto find_email_starts_l0;
				else goto find_email_starts_l17;
		else 
			if (*c < 47) 
				if (*c < 46) 
					if (*c < 45) goto find_email_starts_l0;
					else goto find_email_starts_l17;
				else goto find_email_starts_l21;
			else 
				if (*c < 48) goto find_email_starts_l17;
				else goto find_email_starts_l19;
	else 
		if (*c < 93) 
			if (*c < 65) 
				if (*c < 64) 
					if (*c < 63) goto find_email_starts_l0;
					else goto find_email_starts_l17;
				else goto find_email_starts_l0;
			else 
				if (*c < 91) goto find_email_starts_l20;
				else goto find_email_starts_l0;
		else 
			if (*c < 123) 
				if (*c < 97) 
					if (*c < 94) goto find_email_starts_l5;
					else goto find_email_starts_l17;
				else goto find_email_starts_l20;
			else 
				if (*c < 127) goto find_email_starts_l17;
				else goto find_email_starts_l0;
find_email_starts_l21:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 58) 
		if (*c < 44) 
			if (*c < 35) 
				if (*c < 34) 
					if (*c < 33) goto find_email_starts_l0;
					else goto find_email_starts_l17;
				else goto find_email_starts_l0;
			else 
				if (*c < 42) 
					if (*c < 40) goto find_email_starts_l17;
					else goto find_email_starts_l0;
				else goto find_email_starts_l17;
		else 
			if (*c < 47) 
				if (*c < 46) 
					if (*c < 45) goto find_email_starts_l0;
					else goto find_email_starts_l17;
				else goto find_email_starts_l0;
			else 
				if (*c < 48) goto find_email_starts_l17;
				else goto find_email_starts_l22;
	else 
		if (*c < 93) 
			if (*c < 65) 
				if (*c < 64) 
					if (*c < 63) goto find_email_starts_l0;
					else goto find_email_starts_l17;
				else goto find_email_starts_l0;
			else 
				if (*c < 91) goto find_email_starts_l22;
				else goto find_email_starts_l0;
		else 
			if (*c < 123) 
				if (*c < 97) 
					if (*c < 94) goto find_email_starts_l5;
					else goto find_email_starts_l17;
				else goto find_email_starts_l22;
			else 
				if (*c < 127) goto find_email_starts_l17;
				else goto find_email_starts_l0;
find_email_starts_l22:
	*--scratch = 1;
	if (c-- == start) return scratch;
	if (*c < 58) 
		if (*c < 44) 
			if (*c < 35) 
				if (*c < 34) 
					if (*c < 33) goto find_email_starts_l0;
					else goto find_email_starts_l17;
				else goto find_email_starts_l0;
			else 
				if (*c < 42) 
					if (*c < 40) goto find_email_starts_l17;
					else goto find_email_starts_l0;
				else goto find_email_starts_l17;
		else 
			if (*c < 47) 
				if (*c < 46) 
					if (*c < 45) goto find_email_starts_l0;
					else goto find_email_starts_l23;
				else goto find_email_starts_l21;
			else 
				if (*c < 48) goto find_email_starts_l17;
				else goto find_email_starts_l22;
	else 
		if (*c < 93) 
			if (*c < 65) 
				if (*c < 64) 
					if (*c < 63) goto find_email_starts_l0;
					else goto find_email_starts_l17;
				else goto find_email_starts_l16;
			else 
				if (*c < 91) goto find_email_starts_l22;
				else goto find_email_starts_l0;
		else 
			if (*c < 123) 
				if (*c < 97) 
					if (*c < 94) goto find_email_starts_l5;
					else goto find_email_starts_l17;
				else goto find_email_starts_l22;
			else 
				if (*c < 127) goto find_email_starts_l17;
				else goto find_email_starts_l0;
find_email_starts_l23:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 58) 
		if (*c < 44) 
			if (*c < 35) 
				if (*c < 34) 
					if (*c < 33) goto find_email_starts_l0;
					else goto find_email_starts_l17;
				else goto find_email_starts_l0;
			else 
				if (*c < 42) 
					if (*c < 40) goto find_email_starts_l17;
					else goto find_email_starts_l0;
				else goto find_email_starts_l17;
		else 
			if (*c < 47) 
				if (*c < 46) 
					if (*c < 45) goto find_email_starts_l0;
					else goto find_email_starts_l23;
				else goto find_email_starts_l18;
			else 
				if (*c < 48) goto find_email_starts_l17;
				else goto find_email_starts_l22;
	else 
		if (*c < 93) 
			if (*c < 65) 
				if (*c < 64) 
					if (*c < 63) goto find_email_starts_l0;
					else goto find_email_starts_l17;
				else goto find_email_starts_l0;
			else 
				if (*c < 91) goto find_email_starts_l22;
				else goto find_email_starts_l0;
		else 
			if (*c < 123) 
				if (*c < 97) 
					if (*c < 94) goto find_email_starts_l5;
					else goto find_email_starts_l17;
				else goto find_email_starts_l22;
			else 
				if (*c < 127) goto find_email_starts_l17;
				else goto find_email_starts_l0;
find_email_starts_l24:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 34) 
		if (*c < 11) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 10) goto find_email_starts_l26;
				else goto find_email_starts_l0;
		else 
			if (*c < 32) 
				if (*c < 14) 
					if (*c < 13) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 33) goto find_email_starts_l26;
				else goto find_email_starts_l25;
	else 
		if (*c < 93) 
			if (*c < 91) 
				if (*c < 65) 
					if (*c < 35) goto find_email_starts_l30;
					else goto find_email_starts_l25;
				else goto find_email_starts_l31;
			else 
				if (*c < 92) goto find_email_starts_l25;
				else goto find_email_starts_l73;
		else 
			if (*c < 97) 
				if (*c < 94) goto find_email_starts_l35;
				else goto find_email_starts_l25;
			else 
				if (*c < 123) goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l25:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 33) 
		if (*c < 11) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 10) goto find_email_starts_l26;
				else goto find_email_starts_l0;
		else 
			if (*c < 14) 
				if (*c < 13) goto find_email_starts_l25;
				else goto find_email_starts_l0;
			else 
				if (*c < 32) goto find_email_starts_l25;
				else goto find_email_starts_l26;
	else 
		if (*c < 93) 
			if (*c < 65) 
				if (*c < 35) 
					if (*c < 34) goto find_email_starts_l25;
					else goto find_email_starts_l30;
				else goto find_email_starts_l25;
			else 
				if (*c < 91) goto find_email_starts_l31;
				else goto find_email_starts_l25;
		else 
			if (*c < 97) 
				if (*c < 94) goto find_email_starts_l35;
				else goto find_email_starts_l25;
			else 
				if (*c < 123) goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l26:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 33) 
		if (*c < 11) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 10) goto find_email_starts_l26;
				else goto find_email_starts_l27;
		else 
			if (*c < 14) 
				if (*c < 13) goto find_email_starts_l25;
				else goto find_email_starts_l0;
			else 
				if (*c < 32) goto find_email_starts_l25;
				else goto find_email_starts_l26;
	else 
		if (*c < 93) 
			if (*c < 65) 
				if (*c < 35) 
					if (*c < 34) goto find_email_starts_l25;
					else goto find_email_starts_l30;
				else goto find_email_starts_l25;
			else 
				if (*c < 91) goto find_email_starts_l31;
				else goto find_email_starts_l25;
		else 
			if (*c < 97) 
				if (*c < 94) goto find_email_starts_l35;
				else goto find_email_starts_l25;
			else 
				if (*c < 123) goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l27:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 93) 
		if (*c < 65) 
			if (*c < 14) 
				if (*c < 13) goto find_email_starts_l0;
				else goto find_email_starts_l28;
			else goto find_email_starts_l0;
		else 
			if (*c < 91) goto find_email_starts_l1;
			else goto find_email_starts_l0;
	else 
		if (*c < 97) 
			if (*c < 94) goto find_email_starts_l5;
			else goto find_email_starts_l0;
		else 
			if (*c < 123) goto find_email_starts_l1;
			else goto find_email_starts_l0;
find_email_starts_l28:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 34) 
		if (*c < 11) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 10) goto find_email_starts_l29;
				else goto find_email_starts_l0;
		else 
			if (*c < 32) 
				if (*c < 14) 
					if (*c < 13) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 33) goto find_email_starts_l29;
				else goto find_email_starts_l25;
	else 
		if (*c < 93) 
			if (*c < 91) 
				if (*c < 65) 
					if (*c < 35) goto find_email_starts_l30;
					else goto find_email_starts_l25;
				else goto find_email_starts_l31;
			else 
				if (*c < 92) goto find_email_starts_l25;
				else goto find_email_starts_l73;
		else 
			if (*c < 97) 
				if (*c < 94) goto find_email_starts_l35;
				else goto find_email_starts_l25;
			else 
				if (*c < 123) goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l29:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 33) 
		if (*c < 11) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 10) goto find_email_starts_l29;
				else goto find_email_starts_l0;
		else 
			if (*c < 14) 
				if (*c < 13) goto find_email_starts_l25;
				else goto find_email_starts_l0;
			else 
				if (*c < 32) goto find_email_starts_l25;
				else goto find_email_starts_l29;
	else 
		if (*c < 93) 
			if (*c < 65) 
				if (*c < 35) 
					if (*c < 34) goto find_email_starts_l25;
					else goto find_email_starts_l30;
				else goto find_email_starts_l25;
			else 
				if (*c < 91) goto find_email_starts_l31;
				else goto find_email_starts_l25;
		else 
			if (*c < 97) 
				if (*c < 94) goto find_email_starts_l35;
				else goto find_email_starts_l25;
			else 
				if (*c < 123) goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l30:
	*--scratch = 1;
	if (c-- == start) return scratch;
	if (*c < 93) 
		if (*c < 91) 
			if (*c < 65) goto find_email_starts_l0;
			else goto find_email_starts_l1;
		else 
			if (*c < 92) goto find_email_starts_l0;
			else goto find_email_starts_l24;
	else 
		if (*c < 97) 
			if (*c < 94) goto find_email_starts_l5;
			else goto find_email_starts_l0;
		else 
			if (*c < 123) goto find_email_starts_l1;
			else goto find_email_starts_l0;
find_email_starts_l31:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 34) 
		if (*c < 11) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 10) goto find_email_starts_l26;
				else goto find_email_starts_l0;
		else 
			if (*c < 32) 
				if (*c < 14) 
					if (*c < 13) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 33) goto find_email_starts_l26;
				else goto find_email_starts_l25;
	else 
		if (*c < 91) 
			if (*c < 47) 
				if (*c < 46) 
					if (*c < 35) goto find_email_starts_l30;
					else goto find_email_starts_l25;
				else goto find_email_starts_l32;
			else 
				if (*c < 65) goto find_email_starts_l25;
				else goto find_email_starts_l31;
		else 
			if (*c < 97) 
				if (*c < 94) 
					if (*c < 93) goto find_email_starts_l25;
					else goto find_email_starts_l35;
				else goto find_email_starts_l25;
			else 
				if (*c < 123) goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l32:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 34) 
		if (*c < 11) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 10) goto find_email_starts_l26;
				else goto find_email_starts_l0;
		else 
			if (*c < 32) 
				if (*c < 14) 
					if (*c < 13) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 33) goto find_email_starts_l26;
				else goto find_email_starts_l25;
	else 
		if (*c < 91) 
			if (*c < 58) 
				if (*c < 48) 
					if (*c < 35) goto find_email_starts_l30;
					else goto find_email_starts_l25;
				else goto find_email_starts_l33;
			else 
				if (*c < 65) goto find_email_starts_l25;
				else goto find_email_starts_l33;
		else 
			if (*c < 97) 
				if (*c < 94) 
					if (*c < 93) goto find_email_starts_l25;
					else goto find_email_starts_l35;
				else goto find_email_starts_l25;
			else 
				if (*c < 123) goto find_email_starts_l33;
				else goto find_email_starts_l25;
find_email_starts_l33:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 45) 
		if (*c < 13) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 11) 
					if (*c < 10) goto find_email_starts_l26;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
		else 
			if (*c < 33) 
				if (*c < 32) 
					if (*c < 14) goto find_email_starts_l0;
					else goto find_email_starts_l25;
				else goto find_email_starts_l26;
			else 
				if (*c < 35) 
					if (*c < 34) goto find_email_starts_l25;
					else goto find_email_starts_l30;
				else goto find_email_starts_l25;
	else 
		if (*c < 65) 
			if (*c < 48) 
				if (*c < 47) 
					if (*c < 46) goto find_email_starts_l34;
					else goto find_email_starts_l32;
				else goto find_email_starts_l25;
			else 
				if (*c < 64) 
					if (*c < 58) goto find_email_starts_l33;
					else goto find_email_starts_l25;
				else goto find_email_starts_l46;
		else 
			if (*c < 94) 
				if (*c < 93) 
					if (*c < 91) goto find_email_starts_l33;
					else goto find_email_starts_l25;
				else goto find_email_starts_l35;
			else 
				if (*c < 123) 
					if (*c < 97) goto find_email_starts_l25;
					else goto find_email_starts_l33;
				else goto find_email_starts_l25;
find_email_starts_l34:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 35) 
		if (*c < 13) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 11) 
					if (*c < 10) goto find_email_starts_l26;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
		else 
			if (*c < 33) 
				if (*c < 32) 
					if (*c < 14) goto find_email_starts_l0;
					else goto find_email_starts_l25;
				else goto find_email_starts_l26;
			else 
				if (*c < 34) goto find_email_starts_l25;
				else goto find_email_starts_l30;
	else 
		if (*c < 91) 
			if (*c < 48) 
				if (*c < 46) 
					if (*c < 45) goto find_email_starts_l25;
					else goto find_email_starts_l34;
				else goto find_email_starts_l25;
			else 
				if (*c < 65) 
					if (*c < 58) goto find_email_starts_l33;
					else goto find_email_starts_l25;
				else goto find_email_starts_l33;
		else 
			if (*c < 97) 
				if (*c < 94) 
					if (*c < 93) goto find_email_starts_l25;
					else goto find_email_starts_l35;
				else goto find_email_starts_l25;
			else 
				if (*c < 123) goto find_email_starts_l33;
				else goto find_email_starts_l25;
find_email_starts_l35:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 35) 
		if (*c < 13) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 11) 
					if (*c < 10) goto find_email_starts_l26;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
		else 
			if (*c < 33) 
				if (*c < 32) 
					if (*c < 14) goto find_email_starts_l0;
					else goto find_email_starts_l25;
				else goto find_email_starts_l26;
			else 
				if (*c < 34) goto find_email_starts_l25;
				else goto find_email_starts_l30;
	else 
		if (*c < 91) 
			if (*c < 58) 
				if (*c < 54) 
					if (*c < 48) goto find_email_starts_l25;
					else goto find_email_starts_l36;
				else goto find_email_starts_l72;
			else 
				if (*c < 65) goto find_email_starts_l25;
				else goto find_email_starts_l31;
		else 
			if (*c < 97) 
				if (*c < 94) 
					if (*c < 93) goto find_email_starts_l25;
					else goto find_email_starts_l35;
				else goto find_email_starts_l25;
			else 
				if (*c < 123) goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l36:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 46) 
		if (*c < 13) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 11) 
					if (*c < 10) goto find_email_starts_l26;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
		else 
			if (*c < 33) 
				if (*c < 32) 
					if (*c < 14) goto find_email_starts_l0;
					else goto find_email_starts_l25;
				else goto find_email_starts_l26;
			else 
				if (*c < 35) 
					if (*c < 34) goto find_email_starts_l25;
					else goto find_email_starts_l30;
				else goto find_email_starts_l25;
	else 
		if (*c < 65) 
			if (*c < 49) 
				if (*c < 48) 
					if (*c < 47) goto find_email_starts_l37;
					else goto find_email_starts_l25;
				else goto find_email_starts_l68;
			else 
				if (*c < 58) 
					if (*c < 54) goto find_email_starts_l70;
					else goto find_email_starts_l71;
				else goto find_email_starts_l25;
		else 
			if (*c < 94) 
				if (*c < 93) 
					if (*c < 91) goto find_email_starts_l31;
					else goto find_email_starts_l25;
				else goto find_email_starts_l35;
			else 
				if (*c < 123) 
					if (*c < 97) goto find_email_starts_l25;
					else goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l37:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 35) 
		if (*c < 13) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 11) 
					if (*c < 10) goto find_email_starts_l26;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
		else 
			if (*c < 33) 
				if (*c < 32) 
					if (*c < 14) goto find_email_starts_l0;
					else goto find_email_starts_l25;
				else goto find_email_starts_l26;
			else 
				if (*c < 34) goto find_email_starts_l25;
				else goto find_email_starts_l30;
	else 
		if (*c < 91) 
			if (*c < 58) 
				if (*c < 54) 
					if (*c < 48) goto find_email_starts_l25;
					else goto find_email_starts_l38;
				else goto find_email_starts_l67;
			else 
				if (*c < 65) goto find_email_starts_l25;
				else goto find_email_starts_l31;
		else 
			if (*c < 97) 
				if (*c < 94) 
					if (*c < 93) goto find_email_starts_l25;
					else goto find_email_starts_l35;
				else goto find_email_starts_l25;
			else 
				if (*c < 123) goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l38:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 46) 
		if (*c < 13) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 11) 
					if (*c < 10) goto find_email_starts_l26;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
		else 
			if (*c < 33) 
				if (*c < 32) 
					if (*c < 14) goto find_email_starts_l0;
					else goto find_email_starts_l25;
				else goto find_email_starts_l26;
			else 
				if (*c < 35) 
					if (*c < 34) goto find_email_starts_l25;
					else goto find_email_starts_l30;
				else goto find_email_starts_l25;
	else 
		if (*c < 65) 
			if (*c < 49) 
				if (*c < 48) 
					if (*c < 47) goto find_email_starts_l39;
					else goto find_email_starts_l25;
				else goto find_email_starts_l63;
			else 
				if (*c < 58) 
					if (*c < 54) goto find_email_starts_l65;
					else goto find_email_starts_l66;
				else goto find_email_starts_l25;
		else 
			if (*c < 94) 
				if (*c < 93) 
					if (*c < 91) goto find_email_starts_l31;
					else goto find_email_starts_l25;
				else goto find_email_starts_l35;
			else 
				if (*c < 123) 
					if (*c < 97) goto find_email_starts_l25;
					else goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l39:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 35) 
		if (*c < 13) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 11) 
					if (*c < 10) goto find_email_starts_l26;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
		else 
			if (*c < 33) 
				if (*c < 32) 
					if (*c < 14) goto find_email_starts_l0;
					else goto find_email_starts_l25;
				else goto find_email_starts_l26;
			else 
				if (*c < 34) goto find_email_starts_l25;
				else goto find_email_starts_l30;
	else 
		if (*c < 91) 
			if (*c < 58) 
				if (*c < 54) 
					if (*c < 48) goto find_email_starts_l25;
					else goto find_email_starts_l40;
				else goto find_email_starts_l62;
			else 
				if (*c < 65) goto find_email_starts_l25;
				else goto find_email_starts_l31;
		else 
			if (*c < 97) 
				if (*c < 94) 
					if (*c < 93) goto find_email_starts_l25;
					else goto find_email_starts_l35;
				else goto find_email_starts_l25;
			else 
				if (*c < 123) goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l40:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 46) 
		if (*c < 13) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 11) 
					if (*c < 10) goto find_email_starts_l26;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
		else 
			if (*c < 33) 
				if (*c < 32) 
					if (*c < 14) goto find_email_starts_l0;
					else goto find_email_starts_l25;
				else goto find_email_starts_l26;
			else 
				if (*c < 35) 
					if (*c < 34) goto find_email_starts_l25;
					else goto find_email_starts_l30;
				else goto find_email_starts_l25;
	else 
		if (*c < 65) 
			if (*c < 49) 
				if (*c < 48) 
					if (*c < 47) goto find_email_starts_l41;
					else goto find_email_starts_l25;
				else goto find_email_starts_l58;
			else 
				if (*c < 58) 
					if (*c < 54) goto find_email_starts_l60;
					else goto find_email_starts_l61;
				else goto find_email_starts_l25;
		else 
			if (*c < 94) 
				if (*c < 93) 
					if (*c < 91) goto find_email_starts_l31;
					else goto find_email_starts_l25;
				else goto find_email_starts_l35;
			else 
				if (*c < 123) 
					if (*c < 97) goto find_email_starts_l25;
					else goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l41:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 35) 
		if (*c < 13) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 11) 
					if (*c < 10) goto find_email_starts_l26;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
		else 
			if (*c < 33) 
				if (*c < 32) 
					if (*c < 14) goto find_email_starts_l0;
					else goto find_email_starts_l25;
				else goto find_email_starts_l26;
			else 
				if (*c < 34) goto find_email_starts_l25;
				else goto find_email_starts_l30;
	else 
		if (*c < 91) 
			if (*c < 58) 
				if (*c < 54) 
					if (*c < 48) goto find_email_starts_l25;
					else goto find_email_starts_l42;
				else goto find_email_starts_l57;
			else 
				if (*c < 65) goto find_email_starts_l25;
				else goto find_email_starts_l31;
		else 
			if (*c < 97) 
				if (*c < 94) 
					if (*c < 93) goto find_email_starts_l25;
					else goto find_email_starts_l35;
				else goto find_email_starts_l25;
			else 
				if (*c < 123) goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l42:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 48) 
		if (*c < 13) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 11) 
					if (*c < 10) goto find_email_starts_l26;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
		else 
			if (*c < 33) 
				if (*c < 32) 
					if (*c < 14) goto find_email_starts_l0;
					else goto find_email_starts_l25;
				else goto find_email_starts_l26;
			else 
				if (*c < 35) 
					if (*c < 34) goto find_email_starts_l25;
					else goto find_email_starts_l30;
				else goto find_email_starts_l25;
	else 
		if (*c < 92) 
			if (*c < 58) 
				if (*c < 54) 
					if (*c < 49) goto find_email_starts_l43;
					else goto find_email_starts_l55;
				else goto find_email_starts_l56;
			else 
				if (*c < 91) 
					if (*c < 65) goto find_email_starts_l25;
					else goto find_email_starts_l31;
				else goto find_email_starts_l45;
		else 
			if (*c < 97) 
				if (*c < 94) 
					if (*c < 93) goto find_email_starts_l25;
					else goto find_email_starts_l35;
				else goto find_email_starts_l25;
			else 
				if (*c < 123) goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l43:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 34) 
		if (*c < 11) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 10) goto find_email_starts_l26;
				else goto find_email_starts_l0;
		else 
			if (*c < 32) 
				if (*c < 14) 
					if (*c < 13) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 33) goto find_email_starts_l26;
				else goto find_email_starts_l25;
	else 
		if (*c < 91) 
			if (*c < 51) 
				if (*c < 49) 
					if (*c < 35) goto find_email_starts_l30;
					else goto find_email_starts_l25;
				else goto find_email_starts_l44;
			else 
				if (*c < 65) goto find_email_starts_l25;
				else goto find_email_starts_l31;
		else 
			if (*c < 97) 
				if (*c < 94) 
					if (*c < 93) goto find_email_starts_l25;
					else goto find_email_starts_l35;
				else goto find_email_starts_l25;
			else 
				if (*c < 123) goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l44:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 34) 
		if (*c < 11) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 10) goto find_email_starts_l26;
				else goto find_email_starts_l0;
		else 
			if (*c < 32) 
				if (*c < 14) 
					if (*c < 13) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 33) goto find_email_starts_l26;
				else goto find_email_starts_l25;
	else 
		if (*c < 93) 
			if (*c < 91) 
				if (*c < 65) 
					if (*c < 35) goto find_email_starts_l30;
					else goto find_email_starts_l25;
				else goto find_email_starts_l31;
			else 
				if (*c < 92) goto find_email_starts_l45;
				else goto find_email_starts_l25;
		else 
			if (*c < 97) 
				if (*c < 94) goto find_email_starts_l35;
				else goto find_email_starts_l25;
			else 
				if (*c < 123) goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l45:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 34) 
		if (*c < 11) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 10) goto find_email_starts_l26;
				else goto find_email_starts_l0;
		else 
			if (*c < 32) 
				if (*c < 14) 
					if (*c < 13) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 33) goto find_email_starts_l26;
				else goto find_email_starts_l25;
	else 
		if (*c < 93) 
			if (*c < 65) 
				if (*c < 64) 
					if (*c < 35) goto find_email_starts_l30;
					else goto find_email_starts_l25;
				else goto find_email_starts_l46;
			else 
				if (*c < 91) goto find_email_starts_l31;
				else goto find_email_starts_l25;
		else 
			if (*c < 97) 
				if (*c < 94) goto find_email_starts_l35;
				else goto find_email_starts_l25;
			else 
				if (*c < 123) goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l46:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 45) 
		if (*c < 32) 
			if (*c < 10) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else 
					if (*c < 9) goto find_email_starts_l25;
					else goto find_email_starts_l26;
			else 
				if (*c < 13) 
					if (*c < 11) goto find_email_starts_l0;
					else goto find_email_starts_l25;
				else 
					if (*c < 14) goto find_email_starts_l0;
					else goto find_email_starts_l25;
		else 
			if (*c < 40) 
				if (*c < 34) 
					if (*c < 33) goto find_email_starts_l26;
					else goto find_email_starts_l47;
				else 
					if (*c < 35) goto find_email_starts_l54;
					else goto find_email_starts_l47;
			else 
				if (*c < 44) 
					if (*c < 42) goto find_email_starts_l25;
					else goto find_email_starts_l47;
				else goto find_email_starts_l25;
	else 
		if (*c < 65) 
			if (*c < 58) 
				if (*c < 47) 
					if (*c < 46) goto find_email_starts_l47;
					else goto find_email_starts_l25;
				else 
					if (*c < 48) goto find_email_starts_l47;
					else goto find_email_starts_l49;
			else 
				if (*c < 64) 
					if (*c < 63) goto find_email_starts_l25;
					else goto find_email_starts_l47;
				else goto find_email_starts_l25;
		else 
			if (*c < 97) 
				if (*c < 93) 
					if (*c < 91) goto find_email_starts_l50;
					else goto find_email_starts_l25;
				else 
					if (*c < 94) goto find_email_starts_l35;
					else goto find_email_starts_l47;
			else 
				if (*c < 127) 
					if (*c < 123) goto find_email_starts_l50;
					else goto find_email_starts_l47;
				else goto find_email_starts_l25;
find_email_starts_l47:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 45) 
		if (*c < 32) 
			if (*c < 10) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else 
					if (*c < 9) goto find_email_starts_l25;
					else goto find_email_starts_l26;
			else 
				if (*c < 13) 
					if (*c < 11) goto find_email_starts_l0;
					else goto find_email_starts_l25;
				else 
					if (*c < 14) goto find_email_starts_l0;
					else goto find_email_starts_l25;
		else 
			if (*c < 40) 
				if (*c < 34) 
					if (*c < 33) goto find_email_starts_l26;
					else goto find_email_starts_l47;
				else 
					if (*c < 35) goto find_email_starts_l30;
					else goto find_email_starts_l47;
			else 
				if (*c < 44) 
					if (*c < 42) goto find_email_starts_l25;
					else goto find_email_starts_l47;
				else goto find_email_starts_l25;
	else 
		if (*c < 65) 
			if (*c < 58) 
				if (*c < 47) 
					if (*c < 46) goto find_email_starts_l47;
					else goto find_email_starts_l48;
				else 
					if (*c < 48) goto find_email_starts_l47;
					else goto find_email_starts_l49;
			else 
				if (*c < 64) 
					if (*c < 63) goto find_email_starts_l25;
					else goto find_email_starts_l47;
				else goto find_email_starts_l25;
		else 
			if (*c < 97) 
				if (*c < 93) 
					if (*c < 91) goto find_email_starts_l50;
					else goto find_email_starts_l25;
				else 
					if (*c < 94) goto find_email_starts_l35;
					else goto find_email_starts_l47;
			else 
				if (*c < 127) 
					if (*c < 123) goto find_email_starts_l50;
					else goto find_email_starts_l47;
				else goto find_email_starts_l25;
find_email_starts_l48:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 45) 
		if (*c < 32) 
			if (*c < 10) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else 
					if (*c < 9) goto find_email_starts_l25;
					else goto find_email_starts_l26;
			else 
				if (*c < 13) 
					if (*c < 11) goto find_email_starts_l0;
					else goto find_email_starts_l25;
				else 
					if (*c < 14) goto find_email_starts_l0;
					else goto find_email_starts_l25;
		else 
			if (*c < 40) 
				if (*c < 34) 
					if (*c < 33) goto find_email_starts_l26;
					else goto find_email_starts_l47;
				else 
					if (*c < 35) goto find_email_starts_l30;
					else goto find_email_starts_l47;
			else 
				if (*c < 44) 
					if (*c < 42) goto find_email_starts_l25;
					else goto find_email_starts_l47;
				else goto find_email_starts_l25;
	else 
		if (*c < 65) 
			if (*c < 58) 
				if (*c < 47) 
					if (*c < 46) goto find_email_starts_l47;
					else goto find_email_starts_l25;
				else 
					if (*c < 48) goto find_email_starts_l47;
					else goto find_email_starts_l49;
			else 
				if (*c < 64) 
					if (*c < 63) goto find_email_starts_l25;
					else goto find_email_starts_l47;
				else goto find_email_starts_l25;
		else 
			if (*c < 97) 
				if (*c < 93) 
					if (*c < 91) goto find_email_starts_l50;
					else goto find_email_starts_l25;
				else 
					if (*c < 94) goto find_email_starts_l35;
					else goto find_email_starts_l47;
			else 
				if (*c < 127) 
					if (*c < 123) goto find_email_starts_l50;
					else goto find_email_starts_l47;
				else goto find_email_starts_l25;
find_email_starts_l49:
	*--scratch = 1;
	if (c-- == start) return scratch;
	if (*c < 45) 
		if (*c < 32) 
			if (*c < 10) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else 
					if (*c < 9) goto find_email_starts_l25;
					else goto find_email_starts_l26;
			else 
				if (*c < 13) 
					if (*c < 11) goto find_email_starts_l0;
					else goto find_email_starts_l25;
				else 
					if (*c < 14) goto find_email_starts_l0;
					else goto find_email_starts_l25;
		else 
			if (*c < 40) 
				if (*c < 34) 
					if (*c < 33) goto find_email_starts_l26;
					else goto find_email_starts_l47;
				else 
					if (*c < 35) goto find_email_starts_l30;
					else goto find_email_starts_l47;
			else 
				if (*c < 44) 
					if (*c < 42) goto find_email_starts_l25;
					else goto find_email_starts_l47;
				else goto find_email_starts_l25;
	else 
		if (*c < 65) 
			if (*c < 58) 
				if (*c < 47) 
					if (*c < 46) goto find_email_starts_l47;
					else goto find_email_starts_l48;
				else 
					if (*c < 48) goto find_email_starts_l47;
					else goto find_email_starts_l49;
			else 
				if (*c < 64) 
					if (*c < 63) goto find_email_starts_l25;
					else goto find_email_starts_l47;
				else goto find_email_starts_l25;
		else 
			if (*c < 97) 
				if (*c < 93) 
					if (*c < 91) goto find_email_starts_l50;
					else goto find_email_starts_l25;
				else 
					if (*c < 94) goto find_email_starts_l35;
					else goto find_email_starts_l47;
			else 
				if (*c < 127) 
					if (*c < 123) goto find_email_starts_l50;
					else goto find_email_starts_l47;
				else goto find_email_starts_l25;
find_email_starts_l50:
	*--scratch = 1;
	if (c-- == start) return scratch;
	if (*c < 45) 
		if (*c < 32) 
			if (*c < 10) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else 
					if (*c < 9) goto find_email_starts_l25;
					else goto find_email_starts_l26;
			else 
				if (*c < 13) 
					if (*c < 11) goto find_email_starts_l0;
					else goto find_email_starts_l25;
				else 
					if (*c < 14) goto find_email_starts_l0;
					else goto find_email_starts_l25;
		else 
			if (*c < 40) 
				if (*c < 34) 
					if (*c < 33) goto find_email_starts_l26;
					else goto find_email_starts_l47;
				else 
					if (*c < 35) goto find_email_starts_l30;
					else goto find_email_starts_l47;
			else 
				if (*c < 44) 
					if (*c < 42) goto find_email_starts_l25;
					else goto find_email_starts_l47;
				else goto find_email_starts_l25;
	else 
		if (*c < 65) 
			if (*c < 58) 
				if (*c < 47) 
					if (*c < 46) goto find_email_starts_l47;
					else goto find_email_starts_l51;
				else 
					if (*c < 48) goto find_email_starts_l47;
					else goto find_email_starts_l49;
			else 
				if (*c < 64) 
					if (*c < 63) goto find_email_starts_l25;
					else goto find_email_starts_l47;
				else goto find_email_starts_l25;
		else 
			if (*c < 97) 
				if (*c < 93) 
					if (*c < 91) goto find_email_starts_l50;
					else goto find_email_starts_l25;
				else 
					if (*c < 94) goto find_email_starts_l35;
					else goto find_email_starts_l47;
			else 
				if (*c < 127) 
					if (*c < 123) goto find_email_starts_l50;
					else goto find_email_starts_l47;
				else goto find_email_starts_l25;
find_email_starts_l51:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 45) 
		if (*c < 32) 
			if (*c < 10) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else 
					if (*c < 9) goto find_email_starts_l25;
					else goto find_email_starts_l26;
			else 
				if (*c < 13) 
					if (*c < 11) goto find_email_starts_l0;
					else goto find_email_starts_l25;
				else 
					if (*c < 14) goto find_email_starts_l0;
					else goto find_email_starts_l25;
		else 
			if (*c < 40) 
				if (*c < 34) 
					if (*c < 33) goto find_email_starts_l26;
					else goto find_email_starts_l47;
				else 
					if (*c < 35) goto find_email_starts_l30;
					else goto find_email_starts_l47;
			else 
				if (*c < 44) 
					if (*c < 42) goto find_email_starts_l25;
					else goto find_email_starts_l47;
				else goto find_email_starts_l25;
	else 
		if (*c < 65) 
			if (*c < 58) 
				if (*c < 47) 
					if (*c < 46) goto find_email_starts_l47;
					else goto find_email_starts_l25;
				else 
					if (*c < 48) goto find_email_starts_l47;
					else goto find_email_starts_l52;
			else 
				if (*c < 64) 
					if (*c < 63) goto find_email_starts_l25;
					else goto find_email_starts_l47;
				else goto find_email_starts_l25;
		else 
			if (*c < 97) 
				if (*c < 93) 
					if (*c < 91) goto find_email_starts_l52;
					else goto find_email_starts_l25;
				else 
					if (*c < 94) goto find_email_starts_l35;
					else goto find_email_starts_l47;
			else 
				if (*c < 127) 
					if (*c < 123) goto find_email_starts_l52;
					else goto find_email_starts_l47;
				else goto find_email_starts_l25;
find_email_starts_l52:
	*--scratch = 1;
	if (c-- == start) return scratch;
	if (*c < 45) 
		if (*c < 32) 
			if (*c < 10) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else 
					if (*c < 9) goto find_email_starts_l25;
					else goto find_email_starts_l26;
			else 
				if (*c < 13) 
					if (*c < 11) goto find_email_starts_l0;
					else goto find_email_starts_l25;
				else 
					if (*c < 14) goto find_email_starts_l0;
					else goto find_email_starts_l25;
		else 
			if (*c < 40) 
				if (*c < 34) 
					if (*c < 33) goto find_email_starts_l26;
					else goto find_email_starts_l47;
				else 
					if (*c < 35) goto find_email_starts_l30;
					else goto find_email_starts_l47;
			else 
				if (*c < 44) 
					if (*c < 42) goto find_email_starts_l25;
					else goto find_email_starts_l47;
				else goto find_email_starts_l25;
	else 
		if (*c < 65) 
			if (*c < 58) 
				if (*c < 47) 
					if (*c < 46) goto find_email_starts_l53;
					else goto find_email_starts_l51;
				else 
					if (*c < 48) goto find_email_starts_l47;
					else goto find_email_starts_l52;
			else 
				if (*c < 64) 
					if (*c < 63) goto find_email_starts_l25;
					else goto find_email_starts_l47;
				else goto find_email_starts_l46;
		else 
			if (*c < 97) 
				if (*c < 93) 
					if (*c < 91) goto find_email_starts_l52;
					else goto find_email_starts_l25;
				else 
					if (*c < 94) goto find_email_starts_l35;
					else goto find_email_starts_l47;
			else 
				if (*c < 127) 
					if (*c < 123) goto find_email_starts_l52;
					else goto find_email_starts_l47;
				else goto find_email_starts_l25;
find_email_starts_l53:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 45) 
		if (*c < 32) 
			if (*c < 10) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else 
					if (*c < 9) goto find_email_starts_l25;
					else goto find_email_starts_l26;
			else 
				if (*c < 13) 
					if (*c < 11) goto find_email_starts_l0;
					else goto find_email_starts_l25;
				else 
					if (*c < 14) goto find_email_starts_l0;
					else goto find_email_starts_l25;
		else 
			if (*c < 40) 
				if (*c < 34) 
					if (*c < 33) goto find_email_starts_l26;
					else goto find_email_starts_l47;
				else 
					if (*c < 35) goto find_email_starts_l30;
					else goto find_email_starts_l47;
			else 
				if (*c < 44) 
					if (*c < 42) goto find_email_starts_l25;
					else goto find_email_starts_l47;
				else goto find_email_starts_l25;
	else 
		if (*c < 65) 
			if (*c < 58) 
				if (*c < 47) 
					if (*c < 46) goto find_email_starts_l53;
					else goto find_email_starts_l48;
				else 
					if (*c < 48) goto find_email_starts_l47;
					else goto find_email_starts_l52;
			else 
				if (*c < 64) 
					if (*c < 63) goto find_email_starts_l25;
					else goto find_email_starts_l47;
				else goto find_email_starts_l25;
		else 
			if (*c < 97) 
				if (*c < 93) 
					if (*c < 91) goto find_email_starts_l52;
					else goto find_email_starts_l25;
				else 
					if (*c < 94) goto find_email_starts_l35;
					else goto find_email_starts_l47;
			else 
				if (*c < 127) 
					if (*c < 123) goto find_email_starts_l52;
					else goto find_email_starts_l47;
				else goto find_email_starts_l25;
find_email_starts_l54:
	*--scratch = 1;
	if (c-- == start) return scratch;
	if (*c < 33) 
		if (*c < 11) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 10) goto find_email_starts_l26;
				else goto find_email_starts_l0;
		else 
			if (*c < 14) 
				if (*c < 13) goto find_email_starts_l25;
				else goto find_email_starts_l0;
			else 
				if (*c < 32) goto find_email_starts_l25;
				else goto find_email_starts_l26;
	else 
		if (*c < 93) 
			if (*c < 65) 
				if (*c < 35) 
					if (*c < 34) goto find_email_starts_l25;
					else goto find_email_starts_l30;
				else goto find_email_starts_l25;
			else 
				if (*c < 91) goto find_email_starts_l31;
				else goto find_email_starts_l25;
		else 
			if (*c < 97) 
				if (*c < 94) goto find_email_starts_l35;
				else goto find_email_starts_l25;
			else 
				if (*c < 123) goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l55:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 35) 
		if (*c < 13) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 11) 
					if (*c < 10) goto find_email_starts_l26;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
		else 
			if (*c < 33) 
				if (*c < 32) 
					if (*c < 14) goto find_email_starts_l0;
					else goto find_email_starts_l25;
				else goto find_email_starts_l26;
			else 
				if (*c < 34) goto find_email_starts_l25;
				else goto find_email_starts_l30;
	else 
		if (*c < 92) 
			if (*c < 65) 
				if (*c < 51) 
					if (*c < 49) goto find_email_starts_l25;
					else goto find_email_starts_l44;
				else goto find_email_starts_l25;
			else 
				if (*c < 91) goto find_email_starts_l31;
				else goto find_email_starts_l45;
		else 
			if (*c < 97) 
				if (*c < 94) 
					if (*c < 93) goto find_email_starts_l25;
					else goto find_email_starts_l35;
				else goto find_email_starts_l25;
			else 
				if (*c < 123) goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l56:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 35) 
		if (*c < 13) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 11) 
					if (*c < 10) goto find_email_starts_l26;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
		else 
			if (*c < 33) 
				if (*c < 32) 
					if (*c < 14) goto find_email_starts_l0;
					else goto find_email_starts_l25;
				else goto find_email_starts_l26;
			else 
				if (*c < 34) goto find_email_starts_l25;
				else goto find_email_starts_l30;
	else 
		if (*c < 92) 
			if (*c < 65) 
				if (*c < 50) 
					if (*c < 49) goto find_email_starts_l25;
					else goto find_email_starts_l44;
				else goto find_email_starts_l25;
			else 
				if (*c < 91) goto find_email_starts_l31;
				else goto find_email_starts_l45;
		else 
			if (*c < 97) 
				if (*c < 94) 
					if (*c < 93) goto find_email_starts_l25;
					else goto find_email_starts_l35;
				else goto find_email_starts_l25;
			else 
				if (*c < 123) goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l57:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 48) 
		if (*c < 13) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 11) 
					if (*c < 10) goto find_email_starts_l26;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
		else 
			if (*c < 33) 
				if (*c < 32) 
					if (*c < 14) goto find_email_starts_l0;
					else goto find_email_starts_l25;
				else goto find_email_starts_l26;
			else 
				if (*c < 35) 
					if (*c < 34) goto find_email_starts_l25;
					else goto find_email_starts_l30;
				else goto find_email_starts_l25;
	else 
		if (*c < 92) 
			if (*c < 58) 
				if (*c < 53) 
					if (*c < 49) goto find_email_starts_l43;
					else goto find_email_starts_l55;
				else goto find_email_starts_l56;
			else 
				if (*c < 91) 
					if (*c < 65) goto find_email_starts_l25;
					else goto find_email_starts_l31;
				else goto find_email_starts_l45;
		else 
			if (*c < 97) 
				if (*c < 94) 
					if (*c < 93) goto find_email_starts_l25;
					else goto find_email_starts_l35;
				else goto find_email_starts_l25;
			else 
				if (*c < 123) goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l58:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 34) 
		if (*c < 11) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 10) goto find_email_starts_l26;
				else goto find_email_starts_l0;
		else 
			if (*c < 32) 
				if (*c < 14) 
					if (*c < 13) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 33) goto find_email_starts_l26;
				else goto find_email_starts_l25;
	else 
		if (*c < 91) 
			if (*c < 51) 
				if (*c < 49) 
					if (*c < 35) goto find_email_starts_l30;
					else goto find_email_starts_l25;
				else goto find_email_starts_l59;
			else 
				if (*c < 65) goto find_email_starts_l25;
				else goto find_email_starts_l31;
		else 
			if (*c < 97) 
				if (*c < 94) 
					if (*c < 93) goto find_email_starts_l25;
					else goto find_email_starts_l35;
				else goto find_email_starts_l25;
			else 
				if (*c < 123) goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l59:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 34) 
		if (*c < 11) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 10) goto find_email_starts_l26;
				else goto find_email_starts_l0;
		else 
			if (*c < 32) 
				if (*c < 14) 
					if (*c < 13) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 33) goto find_email_starts_l26;
				else goto find_email_starts_l25;
	else 
		if (*c < 91) 
			if (*c < 47) 
				if (*c < 46) 
					if (*c < 35) goto find_email_starts_l30;
					else goto find_email_starts_l25;
				else goto find_email_starts_l41;
			else 
				if (*c < 65) goto find_email_starts_l25;
				else goto find_email_starts_l31;
		else 
			if (*c < 97) 
				if (*c < 94) 
					if (*c < 93) goto find_email_starts_l25;
					else goto find_email_starts_l35;
				else goto find_email_starts_l25;
			else 
				if (*c < 123) goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l60:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 35) 
		if (*c < 13) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 11) 
					if (*c < 10) goto find_email_starts_l26;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
		else 
			if (*c < 33) 
				if (*c < 32) 
					if (*c < 14) goto find_email_starts_l0;
					else goto find_email_starts_l25;
				else goto find_email_starts_l26;
			else 
				if (*c < 34) goto find_email_starts_l25;
				else goto find_email_starts_l30;
	else 
		if (*c < 91) 
			if (*c < 49) 
				if (*c < 47) 
					if (*c < 46) goto find_email_starts_l25;
					else goto find_email_starts_l41;
				else goto find_email_starts_l25;
			else 
				if (*c < 65) 
					if (*c < 51) goto find_email_starts_l59;
					else goto find_email_starts_l25;
				else goto find_email_starts_l31;
		else 
			if (*c < 97) 
				if (*c < 94) 
					if (*c < 93) goto find_email_starts_l25;
					else goto find_email_starts_l35;
				else goto find_email_starts_l25;
			else 
				if (*c < 123) goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l61:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 35) 
		if (*c < 13) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 11) 
					if (*c < 10) goto find_email_starts_l26;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
		else 
			if (*c < 33) 
				if (*c < 32) 
					if (*c < 14) goto find_email_starts_l0;
					else goto find_email_starts_l25;
				else goto find_email_starts_l26;
			else 
				if (*c < 34) goto find_email_starts_l25;
				else goto find_email_starts_l30;
	else 
		if (*c < 91) 
			if (*c < 49) 
				if (*c < 47) 
					if (*c < 46) goto find_email_starts_l25;
					else goto find_email_starts_l41;
				else goto find_email_starts_l25;
			else 
				if (*c < 65) 
					if (*c < 50) goto find_email_starts_l59;
					else goto find_email_starts_l25;
				else goto find_email_starts_l31;
		else 
			if (*c < 97) 
				if (*c < 94) 
					if (*c < 93) goto find_email_starts_l25;
					else goto find_email_starts_l35;
				else goto find_email_starts_l25;
			else 
				if (*c < 123) goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l62:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 46) 
		if (*c < 13) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 11) 
					if (*c < 10) goto find_email_starts_l26;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
		else 
			if (*c < 33) 
				if (*c < 32) 
					if (*c < 14) goto find_email_starts_l0;
					else goto find_email_starts_l25;
				else goto find_email_starts_l26;
			else 
				if (*c < 35) 
					if (*c < 34) goto find_email_starts_l25;
					else goto find_email_starts_l30;
				else goto find_email_starts_l25;
	else 
		if (*c < 65) 
			if (*c < 49) 
				if (*c < 48) 
					if (*c < 47) goto find_email_starts_l41;
					else goto find_email_starts_l25;
				else goto find_email_starts_l58;
			else 
				if (*c < 58) 
					if (*c < 53) goto find_email_starts_l60;
					else goto find_email_starts_l61;
				else goto find_email_starts_l25;
		else 
			if (*c < 94) 
				if (*c < 93) 
					if (*c < 91) goto find_email_starts_l31;
					else goto find_email_starts_l25;
				else goto find_email_starts_l35;
			else 
				if (*c < 123) 
					if (*c < 97) goto find_email_starts_l25;
					else goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l63:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 34) 
		if (*c < 11) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 10) goto find_email_starts_l26;
				else goto find_email_starts_l0;
		else 
			if (*c < 32) 
				if (*c < 14) 
					if (*c < 13) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 33) goto find_email_starts_l26;
				else goto find_email_starts_l25;
	else 
		if (*c < 91) 
			if (*c < 51) 
				if (*c < 49) 
					if (*c < 35) goto find_email_starts_l30;
					else goto find_email_starts_l25;
				else goto find_email_starts_l64;
			else 
				if (*c < 65) goto find_email_starts_l25;
				else goto find_email_starts_l31;
		else 
			if (*c < 97) 
				if (*c < 94) 
					if (*c < 93) goto find_email_starts_l25;
					else goto find_email_starts_l35;
				else goto find_email_starts_l25;
			else 
				if (*c < 123) goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l64:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 34) 
		if (*c < 11) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 10) goto find_email_starts_l26;
				else goto find_email_starts_l0;
		else 
			if (*c < 32) 
				if (*c < 14) 
					if (*c < 13) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 33) goto find_email_starts_l26;
				else goto find_email_starts_l25;
	else 
		if (*c < 91) 
			if (*c < 47) 
				if (*c < 46) 
					if (*c < 35) goto find_email_starts_l30;
					else goto find_email_starts_l25;
				else goto find_email_starts_l39;
			else 
				if (*c < 65) goto find_email_starts_l25;
				else goto find_email_starts_l31;
		else 
			if (*c < 97) 
				if (*c < 94) 
					if (*c < 93) goto find_email_starts_l25;
					else goto find_email_starts_l35;
				else goto find_email_starts_l25;
			else 
				if (*c < 123) goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l65:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 35) 
		if (*c < 13) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 11) 
					if (*c < 10) goto find_email_starts_l26;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
		else 
			if (*c < 33) 
				if (*c < 32) 
					if (*c < 14) goto find_email_starts_l0;
					else goto find_email_starts_l25;
				else goto find_email_starts_l26;
			else 
				if (*c < 34) goto find_email_starts_l25;
				else goto find_email_starts_l30;
	else 
		if (*c < 91) 
			if (*c < 49) 
				if (*c < 47) 
					if (*c < 46) goto find_email_starts_l25;
					else goto find_email_starts_l39;
				else goto find_email_starts_l25;
			else 
				if (*c < 65) 
					if (*c < 51) goto find_email_starts_l64;
					else goto find_email_starts_l25;
				else goto find_email_starts_l31;
		else 
			if (*c < 97) 
				if (*c < 94) 
					if (*c < 93) goto find_email_starts_l25;
					else goto find_email_starts_l35;
				else goto find_email_starts_l25;
			else 
				if (*c < 123) goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l66:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 35) 
		if (*c < 13) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 11) 
					if (*c < 10) goto find_email_starts_l26;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
		else 
			if (*c < 33) 
				if (*c < 32) 
					if (*c < 14) goto find_email_starts_l0;
					else goto find_email_starts_l25;
				else goto find_email_starts_l26;
			else 
				if (*c < 34) goto find_email_starts_l25;
				else goto find_email_starts_l30;
	else 
		if (*c < 91) 
			if (*c < 49) 
				if (*c < 47) 
					if (*c < 46) goto find_email_starts_l25;
					else goto find_email_starts_l39;
				else goto find_email_starts_l25;
			else 
				if (*c < 65) 
					if (*c < 50) goto find_email_starts_l64;
					else goto find_email_starts_l25;
				else goto find_email_starts_l31;
		else 
			if (*c < 97) 
				if (*c < 94) 
					if (*c < 93) goto find_email_starts_l25;
					else goto find_email_starts_l35;
				else goto find_email_starts_l25;
			else 
				if (*c < 123) goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l67:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 46) 
		if (*c < 13) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 11) 
					if (*c < 10) goto find_email_starts_l26;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
		else 
			if (*c < 33) 
				if (*c < 32) 
					if (*c < 14) goto find_email_starts_l0;
					else goto find_email_starts_l25;
				else goto find_email_starts_l26;
			else 
				if (*c < 35) 
					if (*c < 34) goto find_email_starts_l25;
					else goto find_email_starts_l30;
				else goto find_email_starts_l25;
	else 
		if (*c < 65) 
			if (*c < 49) 
				if (*c < 48) 
					if (*c < 47) goto find_email_starts_l39;
					else goto find_email_starts_l25;
				else goto find_email_starts_l63;
			else 
				if (*c < 58) 
					if (*c < 53) goto find_email_starts_l65;
					else goto find_email_starts_l66;
				else goto find_email_starts_l25;
		else 
			if (*c < 94) 
				if (*c < 93) 
					if (*c < 91) goto find_email_starts_l31;
					else goto find_email_starts_l25;
				else goto find_email_starts_l35;
			else 
				if (*c < 123) 
					if (*c < 97) goto find_email_starts_l25;
					else goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l68:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 34) 
		if (*c < 11) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 10) goto find_email_starts_l26;
				else goto find_email_starts_l0;
		else 
			if (*c < 32) 
				if (*c < 14) 
					if (*c < 13) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 33) goto find_email_starts_l26;
				else goto find_email_starts_l25;
	else 
		if (*c < 91) 
			if (*c < 51) 
				if (*c < 49) 
					if (*c < 35) goto find_email_starts_l30;
					else goto find_email_starts_l25;
				else goto find_email_starts_l69;
			else 
				if (*c < 65) goto find_email_starts_l25;
				else goto find_email_starts_l31;
		else 
			if (*c < 97) 
				if (*c < 94) 
					if (*c < 93) goto find_email_starts_l25;
					else goto find_email_starts_l35;
				else goto find_email_starts_l25;
			else 
				if (*c < 123) goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l69:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 34) 
		if (*c < 11) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 10) goto find_email_starts_l26;
				else goto find_email_starts_l0;
		else 
			if (*c < 32) 
				if (*c < 14) 
					if (*c < 13) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 33) goto find_email_starts_l26;
				else goto find_email_starts_l25;
	else 
		if (*c < 91) 
			if (*c < 47) 
				if (*c < 46) 
					if (*c < 35) goto find_email_starts_l30;
					else goto find_email_starts_l25;
				else goto find_email_starts_l37;
			else 
				if (*c < 65) goto find_email_starts_l25;
				else goto find_email_starts_l31;
		else 
			if (*c < 97) 
				if (*c < 94) 
					if (*c < 93) goto find_email_starts_l25;
					else goto find_email_starts_l35;
				else goto find_email_starts_l25;
			else 
				if (*c < 123) goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l70:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 35) 
		if (*c < 13) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 11) 
					if (*c < 10) goto find_email_starts_l26;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
		else 
			if (*c < 33) 
				if (*c < 32) 
					if (*c < 14) goto find_email_starts_l0;
					else goto find_email_starts_l25;
				else goto find_email_starts_l26;
			else 
				if (*c < 34) goto find_email_starts_l25;
				else goto find_email_starts_l30;
	else 
		if (*c < 91) 
			if (*c < 49) 
				if (*c < 47) 
					if (*c < 46) goto find_email_starts_l25;
					else goto find_email_starts_l37;
				else goto find_email_starts_l25;
			else 
				if (*c < 65) 
					if (*c < 51) goto find_email_starts_l69;
					else goto find_email_starts_l25;
				else goto find_email_starts_l31;
		else 
			if (*c < 97) 
				if (*c < 94) 
					if (*c < 93) goto find_email_starts_l25;
					else goto find_email_starts_l35;
				else goto find_email_starts_l25;
			else 
				if (*c < 123) goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l71:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 35) 
		if (*c < 13) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 11) 
					if (*c < 10) goto find_email_starts_l26;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
		else 
			if (*c < 33) 
				if (*c < 32) 
					if (*c < 14) goto find_email_starts_l0;
					else goto find_email_starts_l25;
				else goto find_email_starts_l26;
			else 
				if (*c < 34) goto find_email_starts_l25;
				else goto find_email_starts_l30;
	else 
		if (*c < 91) 
			if (*c < 49) 
				if (*c < 47) 
					if (*c < 46) goto find_email_starts_l25;
					else goto find_email_starts_l37;
				else goto find_email_starts_l25;
			else 
				if (*c < 65) 
					if (*c < 50) goto find_email_starts_l69;
					else goto find_email_starts_l25;
				else goto find_email_starts_l31;
		else 
			if (*c < 97) 
				if (*c < 94) 
					if (*c < 93) goto find_email_starts_l25;
					else goto find_email_starts_l35;
				else goto find_email_starts_l25;
			else 
				if (*c < 123) goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l72:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 46) 
		if (*c < 13) 
			if (*c < 9) 
				if (*c < 1) 
					if (*c < 0) goto find_email_starts_l25;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
			else 
				if (*c < 11) 
					if (*c < 10) goto find_email_starts_l26;
					else goto find_email_starts_l0;
				else goto find_email_starts_l25;
		else 
			if (*c < 33) 
				if (*c < 32) 
					if (*c < 14) goto find_email_starts_l0;
					else goto find_email_starts_l25;
				else goto find_email_starts_l26;
			else 
				if (*c < 35) 
					if (*c < 34) goto find_email_starts_l25;
					else goto find_email_starts_l30;
				else goto find_email_starts_l25;
	else 
		if (*c < 65) 
			if (*c < 49) 
				if (*c < 48) 
					if (*c < 47) goto find_email_starts_l37;
					else goto find_email_starts_l25;
				else goto find_email_starts_l68;
			else 
				if (*c < 58) 
					if (*c < 53) goto find_email_starts_l70;
					else goto find_email_starts_l71;
				else goto find_email_starts_l25;
		else 
			if (*c < 94) 
				if (*c < 93) 
					if (*c < 91) goto find_email_starts_l31;
					else goto find_email_starts_l25;
				else goto find_email_starts_l35;
			else 
				if (*c < 123) 
					if (*c < 97) goto find_email_starts_l25;
					else goto find_email_starts_l31;
				else goto find_email_starts_l25;
find_email_starts_l73:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 93) 
		if (*c < 91) 
			if (*c < 65) goto find_email_starts_l0;
			else goto find_email_starts_l1;
		else 
			if (*c < 92) goto find_email_starts_l0;
			else goto find_email_starts_l24;
	else 
		if (*c < 97) 
			if (*c < 94) goto find_email_starts_l5;
			else goto find_email_starts_l0;
		else 
			if (*c < 123) goto find_email_starts_l1;
			else goto find_email_starts_l0;
find_email_starts_l74:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 92) 
		if (*c < 65) 
			if (*c < 51) 
				if (*c < 49) goto find_email_starts_l0;
				else goto find_email_starts_l14;
			else goto find_email_starts_l0;
		else 
			if (*c < 91) goto find_email_starts_l1;
			else goto find_email_starts_l15;
	else 
		if (*c < 97) 
			if (*c < 94) 
				if (*c < 93) goto find_email_starts_l0;
				else goto find_email_starts_l5;
			else goto find_email_starts_l0;
		else 
			if (*c < 123) goto find_email_starts_l1;
			else goto find_email_starts_l0;
find_email_starts_l75:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 92) 
		if (*c < 65) 
			if (*c < 50) 
				if (*c < 49) goto find_email_starts_l0;
				else goto find_email_starts_l14;
			else goto find_email_starts_l0;
		else 
			if (*c < 91) goto find_email_starts_l1;
			else goto find_email_starts_l15;
	else 
		if (*c < 97) 
			if (*c < 94) 
				if (*c < 93) goto find_email_starts_l0;
				else goto find_email_starts_l5;
			else goto find_email_starts_l0;
		else 
			if (*c < 123) goto find_email_starts_l1;
			else goto find_email_starts_l0;
find_email_starts_l76:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 91) 
		if (*c < 53) 
			if (*c < 49) 
				if (*c < 48) goto find_email_starts_l0;
				else goto find_email_starts_l13;
			else goto find_email_starts_l74;
		else 
			if (*c < 65) 
				if (*c < 58) goto find_email_starts_l75;
				else goto find_email_starts_l0;
			else goto find_email_starts_l1;
	else 
		if (*c < 94) 
			if (*c < 93) 
				if (*c < 92) goto find_email_starts_l15;
				else goto find_email_starts_l0;
			else goto find_email_starts_l5;
		else 
			if (*c < 123) 
				if (*c < 97) goto find_email_starts_l0;
				else goto find_email_starts_l1;
			else goto find_email_starts_l0;
find_email_starts_l77:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 93) 
		if (*c < 65) 
			if (*c < 51) 
				if (*c < 49) goto find_email_starts_l0;
				else goto find_email_starts_l78;
			else goto find_email_starts_l0;
		else 
			if (*c < 91) goto find_email_starts_l1;
			else goto find_email_starts_l0;
	else 
		if (*c < 97) 
			if (*c < 94) goto find_email_starts_l5;
			else goto find_email_starts_l0;
		else 
			if (*c < 123) goto find_email_starts_l1;
			else goto find_email_starts_l0;
find_email_starts_l78:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 93) 
		if (*c < 65) 
			if (*c < 47) 
				if (*c < 46) goto find_email_starts_l0;
				else goto find_email_starts_l11;
			else goto find_email_starts_l0;
		else 
			if (*c < 91) goto find_email_starts_l1;
			else goto find_email_starts_l0;
	else 
		if (*c < 97) 
			if (*c < 94) goto find_email_starts_l5;
			else goto find_email_starts_l0;
		else 
			if (*c < 123) goto find_email_starts_l1;
			else goto find_email_starts_l0;
find_email_starts_l79:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 91) 
		if (*c < 49) 
			if (*c < 47) 
				if (*c < 46) goto find_email_starts_l0;
				else goto find_email_starts_l11;
			else goto find_email_starts_l0;
		else 
			if (*c < 65) 
				if (*c < 51) goto find_email_starts_l78;
				else goto find_email_starts_l0;
			else goto find_email_starts_l1;
	else 
		if (*c < 97) 
			if (*c < 94) 
				if (*c < 93) goto find_email_starts_l0;
				else goto find_email_starts_l5;
			else goto find_email_starts_l0;
		else 
			if (*c < 123) goto find_email_starts_l1;
			else goto find_email_starts_l0;
find_email_starts_l80:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 91) 
		if (*c < 49) 
			if (*c < 47) 
				if (*c < 46) goto find_email_starts_l0;
				else goto find_email_starts_l11;
			else goto find_email_starts_l0;
		else 
			if (*c < 65) 
				if (*c < 50) goto find_email_starts_l78;
				else goto find_email_starts_l0;
			else goto find_email_starts_l1;
	else 
		if (*c < 97) 
			if (*c < 94) 
				if (*c < 93) goto find_email_starts_l0;
				else goto find_email_starts_l5;
			else goto find_email_starts_l0;
		else 
			if (*c < 123) goto find_email_starts_l1;
			else goto find_email_starts_l0;
find_email_starts_l81:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 65) 
		if (*c < 49) 
			if (*c < 47) 
				if (*c < 46) goto find_email_starts_l0;
				else goto find_email_starts_l11;
			else 
				if (*c < 48) goto find_email_starts_l0;
				else goto find_email_starts_l77;
		else 
			if (*c < 58) 
				if (*c < 53) goto find_email_starts_l79;
				else goto find_email_starts_l80;
			else goto find_email_starts_l0;
	else 
		if (*c < 94) 
			if (*c < 93) 
				if (*c < 91) goto find_email_starts_l1;
				else goto find_email_starts_l0;
			else goto find_email_starts_l5;
		else 
			if (*c < 123) 
				if (*c < 97) goto find_email_starts_l0;
				else goto find_email_starts_l1;
			else goto find_email_starts_l0;
find_email_starts_l82:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 93) 
		if (*c < 65) 
			if (*c < 51) 
				if (*c < 49) goto find_email_starts_l0;
				else goto find_email_starts_l83;
			else goto find_email_starts_l0;
		else 
			if (*c < 91) goto find_email_starts_l1;
			else goto find_email_starts_l0;
	else 
		if (*c < 97) 
			if (*c < 94) goto find_email_starts_l5;
			else goto find_email_starts_l0;
		else 
			if (*c < 123) goto find_email_starts_l1;
			else goto find_email_starts_l0;
find_email_starts_l83:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 93) 
		if (*c < 65) 
			if (*c < 47) 
				if (*c < 46) goto find_email_starts_l0;
				else goto find_email_starts_l9;
			else goto find_email_starts_l0;
		else 
			if (*c < 91) goto find_email_starts_l1;
			else goto find_email_starts_l0;
	else 
		if (*c < 97) 
			if (*c < 94) goto find_email_starts_l5;
			else goto find_email_starts_l0;
		else 
			if (*c < 123) goto find_email_starts_l1;
			else goto find_email_starts_l0;
find_email_starts_l84:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 91) 
		if (*c < 49) 
			if (*c < 47) 
				if (*c < 46) goto find_email_starts_l0;
				else goto find_email_starts_l9;
			else goto find_email_starts_l0;
		else 
			if (*c < 65) 
				if (*c < 51) goto find_email_starts_l83;
				else goto find_email_starts_l0;
			else goto find_email_starts_l1;
	else 
		if (*c < 97) 
			if (*c < 94) 
				if (*c < 93) goto find_email_starts_l0;
				else goto find_email_starts_l5;
			else goto find_email_starts_l0;
		else 
			if (*c < 123) goto find_email_starts_l1;
			else goto find_email_starts_l0;
find_email_starts_l85:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 91) 
		if (*c < 49) 
			if (*c < 47) 
				if (*c < 46) goto find_email_starts_l0;
				else goto find_email_starts_l9;
			else goto find_email_starts_l0;
		else 
			if (*c < 65) 
				if (*c < 50) goto find_email_starts_l83;
				else goto find_email_starts_l0;
			else goto find_email_starts_l1;
	else 
		if (*c < 97) 
			if (*c < 94) 
				if (*c < 93) goto find_email_starts_l0;
				else goto find_email_starts_l5;
			else goto find_email_starts_l0;
		else 
			if (*c < 123) goto find_email_starts_l1;
			else goto find_email_starts_l0;
find_email_starts_l86:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 65) 
		if (*c < 49) 
			if (*c < 47) 
				if (*c < 46) goto find_email_starts_l0;
				else goto find_email_starts_l9;
			else 
				if (*c < 48) goto find_email_starts_l0;
				else goto find_email_starts_l82;
		else 
			if (*c < 58) 
				if (*c < 53) goto find_email_starts_l84;
				else goto find_email_starts_l85;
			else goto find_email_starts_l0;
	else 
		if (*c < 94) 
			if (*c < 93) 
				if (*c < 91) goto find_email_starts_l1;
				else goto find_email_starts_l0;
			else goto find_email_starts_l5;
		else 
			if (*c < 123) 
				if (*c < 97) goto find_email_starts_l0;
				else goto find_email_starts_l1;
			else goto find_email_starts_l0;
find_email_starts_l87:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 93) 
		if (*c < 65) 
			if (*c < 51) 
				if (*c < 49) goto find_email_starts_l0;
				else goto find_email_starts_l88;
			else goto find_email_starts_l0;
		else 
			if (*c < 91) goto find_email_starts_l1;
			else goto find_email_starts_l0;
	else 
		if (*c < 97) 
			if (*c < 94) goto find_email_starts_l5;
			else goto find_email_starts_l0;
		else 
			if (*c < 123) goto find_email_starts_l1;
			else goto find_email_starts_l0;
find_email_starts_l88:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 93) 
		if (*c < 65) 
			if (*c < 47) 
				if (*c < 46) goto find_email_starts_l0;
				else goto find_email_starts_l7;
			else goto find_email_starts_l0;
		else 
			if (*c < 91) goto find_email_starts_l1;
			else goto find_email_starts_l0;
	else 
		if (*c < 97) 
			if (*c < 94) goto find_email_starts_l5;
			else goto find_email_starts_l0;
		else 
			if (*c < 123) goto find_email_starts_l1;
			else goto find_email_starts_l0;
find_email_starts_l89:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 91) 
		if (*c < 49) 
			if (*c < 47) 
				if (*c < 46) goto find_email_starts_l0;
				else goto find_email_starts_l7;
			else goto find_email_starts_l0;
		else 
			if (*c < 65) 
				if (*c < 51) goto find_email_starts_l88;
				else goto find_email_starts_l0;
			else goto find_email_starts_l1;
	else 
		if (*c < 97) 
			if (*c < 94) 
				if (*c < 93) goto find_email_starts_l0;
				else goto find_email_starts_l5;
			else goto find_email_starts_l0;
		else 
			if (*c < 123) goto find_email_starts_l1;
			else goto find_email_starts_l0;
find_email_starts_l90:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 91) 
		if (*c < 49) 
			if (*c < 47) 
				if (*c < 46) goto find_email_starts_l0;
				else goto find_email_starts_l7;
			else goto find_email_starts_l0;
		else 
			if (*c < 65) 
				if (*c < 50) goto find_email_starts_l88;
				else goto find_email_starts_l0;
			else goto find_email_starts_l1;
	else 
		if (*c < 97) 
			if (*c < 94) 
				if (*c < 93) goto find_email_starts_l0;
				else goto find_email_starts_l5;
			else goto find_email_starts_l0;
		else 
			if (*c < 123) goto find_email_starts_l1;
			else goto find_email_starts_l0;
find_email_starts_l91:
	*--scratch = 0;
	if (c-- == start) return scratch;
	if (*c < 65) 
		if (*c < 49) 
			if (*c < 47) 
				if (*c < 46) goto find_email_starts_l0;
				else goto find_email_starts_l7;
			else 
				if (*c < 48) goto find_email_starts_l0;
				else goto find_email_starts_l87;
		else 
			if (*c < 58) 
				if (*c < 53) goto find_email_starts_l89;
				else goto find_email_starts_l90;
			else goto find_email_starts_l0;
	else 
		if (*c < 94) 
			if (*c < 93) 
				if (*c < 91) goto find_email_starts_l1;
				else goto find_email_starts_l0;
			else goto find_email_starts_l5;
		else 
			if (*c < 123) 
				if (*c < 97) goto find_email_starts_l0;
				else goto find_email_starts_l1;
			else goto find_email_starts_l0;
}
