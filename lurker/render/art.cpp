/* Copyright: Public domain
 * Produced with reg2c for wesley@terpstra.ca
 * cvs id tag: $Id: art.cpp,v 1.6 2003-06-12 22:56:39 terpstra Exp $
 *
 * Regular expression: ((
((-- (
[-	-ÿ]+)+)|([	 ][-	-ÿ]+)))+
)
 */

const unsigned char* art_scan_pass1(const unsigned char* s, const unsigned char* e)
{
	const unsigned char* last = 0;
	goto art_scan_pass17;
	
art_scan_pass10:
	last = e;
	if (s == e) return last;
	switch (*(--e))
	{
	case 32: 
		goto art_scan_pass13;
	case 10: 
		goto art_scan_pass16;
	case 0: 
		goto art_scan_pass17;
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
		goto art_scan_pass17;
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
		goto art_scan_pass17;
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
		goto art_scan_pass17;
	default: goto art_scan_pass15;
	}
art_scan_pass14:
	if (s == e) return last;
	switch (*(--e))
	{
	case 32: 
		goto art_scan_pass13;
	case 10: 
		goto art_scan_pass16;
	case 0: 
		goto art_scan_pass17;
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
		goto art_scan_pass17;
	default: goto art_scan_pass15;
	}
art_scan_pass16:
	if (s == e) return last;
	switch (*(--e))
	{
	case 10: 
		goto art_scan_pass16;
	case 0: 
		goto art_scan_pass17;
	default: goto art_scan_pass15;
	}
art_scan_pass17:
	if (s == e) return last;
	switch (*(--e))
	{
	case 10: 
		goto art_scan_pass16;
	default: goto art_scan_pass17;
	}
}

const unsigned char* art_scan_pass2(const unsigned char* s, const unsigned char* e)
{
	const unsigned char* last = 0;
	goto art_scan_pass28;
	
art_scan_pass20:
	return last;
art_scan_pass21:
	last = s;
	if (s == e) return last;
	switch (*s++)
	{
	case 0: case 10: 
		goto art_scan_pass20;
	default: goto art_scan_pass214;
	}
art_scan_pass27:
	last = s;
	if (s == e) return last;
	switch (*s++)
	{
	case 45: 
		goto art_scan_pass210;
	case 9: case 32: 
		goto art_scan_pass239;
	default: goto art_scan_pass20;
	}
art_scan_pass28:
	if (s == e) return last;
	switch (*s++)
	{
	case 10: 
		goto art_scan_pass29;
	default: goto art_scan_pass20;
	}
art_scan_pass29:
	if (s == e) return last;
	switch (*s++)
	{
	case 45: 
		goto art_scan_pass210;
	case 9: case 32: 
		goto art_scan_pass239;
	default: goto art_scan_pass20;
	}
art_scan_pass210:
	if (s == e) return last;
	switch (*s++)
	{
	case 45: 
		goto art_scan_pass211;
	default: goto art_scan_pass20;
	}
art_scan_pass211:
	if (s == e) return last;
	switch (*s++)
	{
	case 32: 
		goto art_scan_pass212;
	default: goto art_scan_pass20;
	}
art_scan_pass212:
	if (s == e) return last;
	switch (*s++)
	{
	case 10: 
		goto art_scan_pass213;
	default: goto art_scan_pass20;
	}
art_scan_pass213:
	if (s == e) return last;
	switch (*s++)
	{
	case 0: case 10: 
		goto art_scan_pass20;
	default: goto art_scan_pass214;
	}
art_scan_pass214:
	if (s == e) return last;
	switch (*s++)
	{
	case 0: 
		goto art_scan_pass20;
	case 10: 
		goto art_scan_pass21;
	default: goto art_scan_pass214;
	}
art_scan_pass239:
	if (s == e) return last;
	switch (*s++)
	{
	case 0: case 10: 
		goto art_scan_pass20;
	default: goto art_scan_pass240;
	}
art_scan_pass240:
	if (s == e) return last;
	switch (*s++)
	{
	case 0: 
		goto art_scan_pass20;
	case 10: 
		goto art_scan_pass27;
	default: goto art_scan_pass240;
	}
}

void art_scan(const unsigned char** s, const unsigned char** e)
{
	*s = art_scan_pass1(*s, *e);
	if (*s) *e = art_scan_pass2(*s, *e);
}
