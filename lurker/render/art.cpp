/* Copyright: Public domain
 * Produced with reg2c for wesley@terpstra.ca
 * cvs id tag: $Id: art.cpp,v 1.4 2003-06-12 21:30:41 terpstra Exp $
 *
 * Regular expression: (
((-- (
[-	-ÿ]+)+)|([	 ][-	-ÿ]+)))+
 */

const unsigned char* art_scan_pass1(const unsigned char* s, const unsigned char* e)
{
	const unsigned char* last = 0;
	goto art_scan_pass16;
	
art_scan_pass10:
	last = e;
	if (s == e) return last;
	switch (*(--e))
	{
	case 32: 
		goto art_scan_pass13;
	case 0: case 10: 
		goto art_scan_pass16;
	default: goto art_scan_pass15;
	}
art_scan_pass11:
	if (s == e) return last;
	switch (*(--e))
	{
	case 10: 
		goto art_scan_pass10;
	case 9: case 32: 
		goto art_scan_pass11;
	case 0: 
		goto art_scan_pass16;
	default: goto art_scan_pass15;
	}
art_scan_pass12:
	if (s == e) return last;
	switch (*(--e))
	{
	case 9: case 32: case 45: 
		goto art_scan_pass11;
	case 10: 
		goto art_scan_pass14;
	case 0: 
		goto art_scan_pass16;
	default: goto art_scan_pass15;
	}
art_scan_pass13:
	if (s == e) return last;
	switch (*(--e))
	{
	case 9: case 32: 
		goto art_scan_pass11;
	case 45: 
		goto art_scan_pass12;
	case 10: 
		goto art_scan_pass14;
	case 0: 
		goto art_scan_pass16;
	default: goto art_scan_pass15;
	}
art_scan_pass14:
	if (s == e) return last;
	switch (*(--e))
	{
	case 32: 
		goto art_scan_pass13;
	case 0: case 10: 
		goto art_scan_pass16;
	default: goto art_scan_pass15;
	}
art_scan_pass15:
	if (s == e) return last;
	switch (*(--e))
	{
	case 9: case 32: 
		goto art_scan_pass11;
	case 10: 
		goto art_scan_pass14;
	case 0: 
		goto art_scan_pass16;
	default: goto art_scan_pass15;
	}
art_scan_pass16:
	if (s == e) return last;
	switch (*(--e))
	{
	case 0: case 10: 
		goto art_scan_pass16;
	default: goto art_scan_pass15;
	}
}

const unsigned char* art_scan_pass2(const unsigned char* s, const unsigned char* e)
{
	const unsigned char* last = 0;
	goto art_scan_pass241;
	
art_scan_pass20:
	return last;
art_scan_pass21:
	last = s;
	if (s == e) return last;
	switch (*s++)
	{
	case 0: 
		goto art_scan_pass20;
	case 10: 
		goto art_scan_pass246;
	default: goto art_scan_pass21;
	}
art_scan_pass226:
	last = s;
	if (s == e) return last;
	switch (*s++)
	{
	case 0: 
		goto art_scan_pass20;
	case 10: 
		goto art_scan_pass242;
	default: goto art_scan_pass226;
	}
art_scan_pass241:
	if (s == e) return last;
	switch (*s++)
	{
	case 10: 
		goto art_scan_pass242;
	default: goto art_scan_pass20;
	}
art_scan_pass242:
	if (s == e) return last;
	switch (*s++)
	{
	case 45: 
		goto art_scan_pass243;
	case 9: case 32: 
		goto art_scan_pass251;
	default: goto art_scan_pass20;
	}
art_scan_pass243:
	if (s == e) return last;
	switch (*s++)
	{
	case 45: 
		goto art_scan_pass244;
	default: goto art_scan_pass20;
	}
art_scan_pass244:
	if (s == e) return last;
	switch (*s++)
	{
	case 32: 
		goto art_scan_pass245;
	default: goto art_scan_pass20;
	}
art_scan_pass245:
	if (s == e) return last;
	switch (*s++)
	{
	case 10: 
		goto art_scan_pass246;
	default: goto art_scan_pass20;
	}
art_scan_pass246:
	if (s == e) return last;
	switch (*s++)
	{
	case 0: case 10: 
		goto art_scan_pass20;
	default: goto art_scan_pass21;
	}
art_scan_pass251:
	if (s == e) return last;
	switch (*s++)
	{
	case 0: case 10: 
		goto art_scan_pass20;
	default: goto art_scan_pass226;
	}
}

void art_scan(const unsigned char** s, const unsigned char** e)
{
	*s = art_scan_pass1(*s, *e);
	if (*s) *e = art_scan_pass2(*s, *e);
}
