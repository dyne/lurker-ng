/* Copyright: Public domain
 * Produced with reg2c for (null)
 * cvs id tag: $Id$
 *
 * Regular expression: (
((-- |_____+ *|-----+ *)(
[-	-ÿ]+)+))
 */

const unsigned char* art_scan_pass1(const unsigned char* s, const unsigned char* e)
{
	const unsigned char* last = 0;
	goto art_scan_pass117;
	
art_scan_pass10:
	last = e;
	if (s == e) return last;
	switch (*(--e))
	{
	case 32: 
		goto art_scan_pass15;
	case 95: 
		goto art_scan_pass110;
	case 45: 
		goto art_scan_pass115;
	case 0: case 10: 
		goto art_scan_pass117;
	default: goto art_scan_pass116;
	}
art_scan_pass11:
	if (s == e) return last;
	switch (*(--e))
	{
	case 10: 
		goto art_scan_pass10;
	case 95: 
		goto art_scan_pass11;
	case 0: 
		goto art_scan_pass117;
	default: goto art_scan_pass116;
	}
art_scan_pass12:
	if (s == e) return last;
	switch (*(--e))
	{
	case 10: 
		goto art_scan_pass10;
	case 45: 
		goto art_scan_pass12;
	case 0: 
		goto art_scan_pass117;
	default: goto art_scan_pass116;
	}
art_scan_pass13:
	if (s == e) return last;
	switch (*(--e))
	{
	case 10: 
		goto art_scan_pass10;
	case 45: 
		goto art_scan_pass113;
	case 0: 
		goto art_scan_pass117;
	default: goto art_scan_pass116;
	}
art_scan_pass14:
	if (s == e) return last;
	switch (*(--e))
	{
	case 45: 
		goto art_scan_pass13;
	case 10: 
		goto art_scan_pass16;
	case 0: 
		goto art_scan_pass117;
	default: goto art_scan_pass116;
	}
art_scan_pass15:
	if (s == e) return last;
	switch (*(--e))
	{
	case 45: 
		goto art_scan_pass14;
	case 10: 
		goto art_scan_pass16;
	case 95: 
		goto art_scan_pass110;
	case 32: 
		goto art_scan_pass111;
	case 0: 
		goto art_scan_pass117;
	default: goto art_scan_pass116;
	}
art_scan_pass16:
	if (s == e) return last;
	switch (*(--e))
	{
	case 32: 
		goto art_scan_pass15;
	case 95: 
		goto art_scan_pass110;
	case 45: 
		goto art_scan_pass115;
	case 0: case 10: 
		goto art_scan_pass117;
	default: goto art_scan_pass116;
	}
art_scan_pass17:
	if (s == e) return last;
	switch (*(--e))
	{
	case 95: 
		goto art_scan_pass11;
	case 10: 
		goto art_scan_pass16;
	case 0: 
		goto art_scan_pass117;
	default: goto art_scan_pass116;
	}
art_scan_pass18:
	if (s == e) return last;
	switch (*(--e))
	{
	case 10: 
		goto art_scan_pass16;
	case 95: 
		goto art_scan_pass17;
	case 0: 
		goto art_scan_pass117;
	default: goto art_scan_pass116;
	}
art_scan_pass19:
	if (s == e) return last;
	switch (*(--e))
	{
	case 10: 
		goto art_scan_pass16;
	case 95: 
		goto art_scan_pass18;
	case 0: 
		goto art_scan_pass117;
	default: goto art_scan_pass116;
	}
art_scan_pass110:
	if (s == e) return last;
	switch (*(--e))
	{
	case 10: 
		goto art_scan_pass16;
	case 95: 
		goto art_scan_pass19;
	case 0: 
		goto art_scan_pass117;
	default: goto art_scan_pass116;
	}
art_scan_pass111:
	if (s == e) return last;
	switch (*(--e))
	{
	case 10: 
		goto art_scan_pass16;
	case 95: 
		goto art_scan_pass110;
	case 32: 
		goto art_scan_pass111;
	case 45: 
		goto art_scan_pass115;
	case 0: 
		goto art_scan_pass117;
	default: goto art_scan_pass116;
	}
art_scan_pass112:
	if (s == e) return last;
	switch (*(--e))
	{
	case 45: 
		goto art_scan_pass12;
	case 10: 
		goto art_scan_pass16;
	case 0: 
		goto art_scan_pass117;
	default: goto art_scan_pass116;
	}
art_scan_pass113:
	if (s == e) return last;
	switch (*(--e))
	{
	case 10: 
		goto art_scan_pass16;
	case 45: 
		goto art_scan_pass112;
	case 0: 
		goto art_scan_pass117;
	default: goto art_scan_pass116;
	}
art_scan_pass114:
	if (s == e) return last;
	switch (*(--e))
	{
	case 10: 
		goto art_scan_pass16;
	case 45: 
		goto art_scan_pass113;
	case 0: 
		goto art_scan_pass117;
	default: goto art_scan_pass116;
	}
art_scan_pass115:
	if (s == e) return last;
	switch (*(--e))
	{
	case 10: 
		goto art_scan_pass16;
	case 45: 
		goto art_scan_pass114;
	case 0: 
		goto art_scan_pass117;
	default: goto art_scan_pass116;
	}
art_scan_pass116:
	if (s == e) return last;
	switch (*(--e))
	{
	case 10: 
		goto art_scan_pass16;
	case 0: 
		goto art_scan_pass117;
	default: goto art_scan_pass116;
	}
art_scan_pass117:
	if (s == e) return last;
	switch (*(--e))
	{
	case 0: case 10: 
		goto art_scan_pass117;
	default: goto art_scan_pass116;
	}
}

const unsigned char* art_scan_pass2(const unsigned char* s, const unsigned char* e)
{
	const unsigned char* last = 0;
	goto art_scan_pass23;
	
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
		goto art_scan_pass218;
	default: goto art_scan_pass21;
	}
art_scan_pass23:
	if (s == e) return last;
	switch (*s++)
	{
	case 10: 
		goto art_scan_pass24;
	default: goto art_scan_pass20;
	}
art_scan_pass24:
	if (s == e) return last;
	switch (*s++)
	{
	case 45: 
		goto art_scan_pass25;
	case 95: 
		goto art_scan_pass27;
	default: goto art_scan_pass20;
	}
art_scan_pass25:
	if (s == e) return last;
	switch (*s++)
	{
	case 45: 
		goto art_scan_pass26;
	default: goto art_scan_pass20;
	}
art_scan_pass26:
	if (s == e) return last;
	switch (*s++)
	{
	case 45: 
		goto art_scan_pass213;
	case 32: 
		goto art_scan_pass217;
	default: goto art_scan_pass20;
	}
art_scan_pass27:
	if (s == e) return last;
	switch (*s++)
	{
	case 95: 
		goto art_scan_pass28;
	default: goto art_scan_pass20;
	}
art_scan_pass28:
	if (s == e) return last;
	switch (*s++)
	{
	case 95: 
		goto art_scan_pass29;
	default: goto art_scan_pass20;
	}
art_scan_pass29:
	if (s == e) return last;
	switch (*s++)
	{
	case 95: 
		goto art_scan_pass210;
	default: goto art_scan_pass20;
	}
art_scan_pass210:
	if (s == e) return last;
	switch (*s++)
	{
	case 95: 
		goto art_scan_pass211;
	default: goto art_scan_pass20;
	}
art_scan_pass211:
	if (s == e) return last;
	switch (*s++)
	{
	case 95: 
		goto art_scan_pass211;
	case 32: 
		goto art_scan_pass212;
	case 10: 
		goto art_scan_pass218;
	default: goto art_scan_pass20;
	}
art_scan_pass212:
	if (s == e) return last;
	switch (*s++)
	{
	case 32: 
		goto art_scan_pass212;
	case 10: 
		goto art_scan_pass218;
	default: goto art_scan_pass20;
	}
art_scan_pass213:
	if (s == e) return last;
	switch (*s++)
	{
	case 45: 
		goto art_scan_pass214;
	default: goto art_scan_pass20;
	}
art_scan_pass214:
	if (s == e) return last;
	switch (*s++)
	{
	case 45: 
		goto art_scan_pass215;
	default: goto art_scan_pass20;
	}
art_scan_pass215:
	if (s == e) return last;
	switch (*s++)
	{
	case 32: 
		goto art_scan_pass212;
	case 45: 
		goto art_scan_pass215;
	case 10: 
		goto art_scan_pass218;
	default: goto art_scan_pass20;
	}
art_scan_pass217:
	if (s == e) return last;
	switch (*s++)
	{
	case 10: 
		goto art_scan_pass218;
	default: goto art_scan_pass20;
	}
art_scan_pass218:
	if (s == e) return last;
	switch (*s++)
	{
	case 0: case 10: 
		goto art_scan_pass20;
	default: goto art_scan_pass21;
	}
}

void art_scan(const unsigned char** s, const unsigned char** e)
{
	*s = art_scan_pass1(*s, *e);
	if (*s) *e = art_scan_pass2(*s, *e);
}
