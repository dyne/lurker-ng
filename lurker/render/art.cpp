/* Copyright: Public domain
 * Produced with reg2c for wesley@terpstra.ca
 * cvs id tag: $Id: art.cpp,v 1.2 2003-04-21 18:26:19 terpstra Exp $
 *
 * Regular expression: 
([-	-ÿ]+
)*([-	-ÿ]*[-	-!-\-/-=?-ÿ]  [-	-ÿ]*[-	-!-\-/-=?-ÿ]  [-	-ÿ]*
)([-	-ÿ]+
)*([-	-ÿ]*[-	-!-\-/-=?-ÿ]  [-	-ÿ]*[-	-!-\-/-=?-ÿ]  [-	-ÿ]*
)([-	-ÿ]+
)*
 */

const unsigned char* art_scan_pass1(const unsigned char* s, const unsigned char* e)
{
	const unsigned char* last = 0;
	goto art_scan_pass129;
	
art_scan_pass10:
	last = e;
	if (s == e) return last;
	switch (*(--e))
	{
	case 10: 
		goto art_scan_pass128;
	case 0: 
		goto art_scan_pass129;
	default: goto art_scan_pass11;
	}
art_scan_pass11:
	if (s == e) return last;
	switch (*(--e))
	{
	case 10: 
		goto art_scan_pass10;
	case 0: 
		goto art_scan_pass129;
	default: goto art_scan_pass11;
	}
art_scan_pass113:
	if (s == e) return last;
	switch (*(--e))
	{
	case 32: 
		goto art_scan_pass113;
	case 46: case 62: 
		goto art_scan_pass115;
	case 10: 
		goto art_scan_pass119;
	case 0: 
		goto art_scan_pass129;
	default: goto art_scan_pass11;
	}
art_scan_pass114:
	if (s == e) return last;
	switch (*(--e))
	{
	case 32: 
		goto art_scan_pass113;
	case 10: 
		goto art_scan_pass119;
	case 0: 
		goto art_scan_pass129;
	default: goto art_scan_pass115;
	}
art_scan_pass115:
	if (s == e) return last;
	switch (*(--e))
	{
	case 32: 
		goto art_scan_pass114;
	case 10: 
		goto art_scan_pass119;
	case 0: 
		goto art_scan_pass129;
	default: goto art_scan_pass115;
	}
art_scan_pass116:
	if (s == e) return last;
	switch (*(--e))
	{
	case 32: 
		goto art_scan_pass116;
	case 46: case 62: 
		goto art_scan_pass118;
	case 10: 
		goto art_scan_pass119;
	case 0: 
		goto art_scan_pass129;
	default: goto art_scan_pass115;
	}
art_scan_pass117:
	if (s == e) return last;
	switch (*(--e))
	{
	case 32: 
		goto art_scan_pass116;
	case 10: 
		goto art_scan_pass119;
	case 0: 
		goto art_scan_pass129;
	default: goto art_scan_pass118;
	}
art_scan_pass118:
	if (s == e) return last;
	switch (*(--e))
	{
	case 32: 
		goto art_scan_pass117;
	case 10: 
		goto art_scan_pass119;
	case 0: 
		goto art_scan_pass129;
	default: goto art_scan_pass118;
	}
art_scan_pass119:
	if (s == e) return last;
	switch (*(--e))
	{
	case 32: 
		goto art_scan_pass117;
	case 10: 
		goto art_scan_pass128;
	case 0: 
		goto art_scan_pass129;
	default: goto art_scan_pass118;
	}
art_scan_pass120:
	if (s == e) return last;
	switch (*(--e))
	{
	case 10: 
		goto art_scan_pass119;
	case 0: 
		goto art_scan_pass129;
	default: goto art_scan_pass120;
	}
art_scan_pass123:
	if (s == e) return last;
	switch (*(--e))
	{
	case 32: 
		goto art_scan_pass123;
	case 46: case 62: 
		goto art_scan_pass125;
	case 10: 
		goto art_scan_pass128;
	case 0: 
		goto art_scan_pass129;
	default: goto art_scan_pass120;
	}
art_scan_pass124:
	if (s == e) return last;
	switch (*(--e))
	{
	case 32: 
		goto art_scan_pass123;
	case 10: 
		goto art_scan_pass128;
	case 0: 
		goto art_scan_pass129;
	default: goto art_scan_pass125;
	}
art_scan_pass125:
	if (s == e) return last;
	switch (*(--e))
	{
	case 32: 
		goto art_scan_pass124;
	case 10: 
		goto art_scan_pass128;
	case 0: 
		goto art_scan_pass129;
	default: goto art_scan_pass125;
	}
art_scan_pass126:
	if (s == e) return last;
	switch (*(--e))
	{
	case 32: 
		goto art_scan_pass126;
	case 10: case 46: case 62: 
		goto art_scan_pass128;
	case 0: 
		goto art_scan_pass129;
	default: goto art_scan_pass125;
	}
art_scan_pass127:
	if (s == e) return last;
	switch (*(--e))
	{
	case 32: 
		goto art_scan_pass126;
	case 0: 
		goto art_scan_pass129;
	default: goto art_scan_pass128;
	}
art_scan_pass128:
	if (s == e) return last;
	switch (*(--e))
	{
	case 32: 
		goto art_scan_pass127;
	case 0: 
		goto art_scan_pass129;
	default: goto art_scan_pass128;
	}
art_scan_pass129:
	if (s == e) return last;
	switch (*(--e))
	{
	case 10: 
		goto art_scan_pass128;
	default: goto art_scan_pass129;
	}
}

const unsigned char* art_scan_pass2(const unsigned char* s, const unsigned char* e)
{
	const unsigned char* last = 0;
	goto art_scan_pass22;
	
art_scan_pass20:
	return last;
art_scan_pass21:
	last = s;
	if (s == e) return last;
	switch (*s++)
	{
	case 0: case 10: 
		goto art_scan_pass20;
	default: goto art_scan_pass29;
	}
art_scan_pass22:
	if (s == e) return last;
	switch (*s++)
	{
	case 10: 
		goto art_scan_pass23;
	default: goto art_scan_pass20;
	}
art_scan_pass23:
	if (s == e) return last;
	switch (*s++)
	{
	case 0: case 10: 
		goto art_scan_pass20;
	case 32: case 46: case 62: 
		goto art_scan_pass25;
	default: goto art_scan_pass26;
	}
art_scan_pass24:
	if (s == e) return last;
	switch (*s++)
	{
	case 0: case 10: 
		goto art_scan_pass20;
	case 32: case 46: case 62: 
		goto art_scan_pass230;
	default: goto art_scan_pass213;
	}
art_scan_pass25:
	if (s == e) return last;
	switch (*s++)
	{
	case 0: 
		goto art_scan_pass20;
	case 10: 
		goto art_scan_pass23;
	case 32: case 46: case 62: 
		goto art_scan_pass25;
	default: goto art_scan_pass26;
	}
art_scan_pass26:
	if (s == e) return last;
	switch (*s++)
	{
	case 0: 
		goto art_scan_pass20;
	case 10: 
		goto art_scan_pass23;
	case 46: case 62: 
		goto art_scan_pass25;
	case 32: 
		goto art_scan_pass215;
	default: goto art_scan_pass26;
	}
art_scan_pass27:
	if (s == e) return last;
	switch (*s++)
	{
	case 0: 
		goto art_scan_pass20;
	case 10: 
		goto art_scan_pass23;
	case 32: 
		goto art_scan_pass216;
	case 46: case 62: 
		goto art_scan_pass224;
	default: goto art_scan_pass27;
	}
art_scan_pass28:
	if (s == e) return last;
	switch (*s++)
	{
	case 0: 
		goto art_scan_pass20;
	case 10: 
		goto art_scan_pass24;
	default: goto art_scan_pass28;
	}
art_scan_pass29:
	if (s == e) return last;
	switch (*s++)
	{
	case 0: 
		goto art_scan_pass20;
	case 10: 
		goto art_scan_pass21;
	default: goto art_scan_pass29;
	}
art_scan_pass211:
	if (s == e) return last;
	switch (*s++)
	{
	case 0: 
		goto art_scan_pass20;
	case 10: 
		goto art_scan_pass24;
	case 32: 
		goto art_scan_pass220;
	case 46: case 62: 
		goto art_scan_pass228;
	default: goto art_scan_pass211;
	}
art_scan_pass213:
	if (s == e) return last;
	switch (*s++)
	{
	case 0: 
		goto art_scan_pass20;
	case 10: 
		goto art_scan_pass24;
	case 32: 
		goto art_scan_pass222;
	case 46: case 62: 
		goto art_scan_pass230;
	default: goto art_scan_pass213;
	}
art_scan_pass215:
	if (s == e) return last;
	switch (*s++)
	{
	case 0: 
		goto art_scan_pass20;
	case 10: 
		goto art_scan_pass23;
	case 46: case 62: 
		goto art_scan_pass25;
	case 32: 
		goto art_scan_pass224;
	default: goto art_scan_pass26;
	}
art_scan_pass216:
	if (s == e) return last;
	switch (*s++)
	{
	case 0: 
		goto art_scan_pass20;
	case 10: 
		goto art_scan_pass23;
	case 32: 
		goto art_scan_pass28;
	case 46: case 62: 
		goto art_scan_pass224;
	default: goto art_scan_pass27;
	}
art_scan_pass220:
	if (s == e) return last;
	switch (*s++)
	{
	case 0: 
		goto art_scan_pass20;
	case 10: 
		goto art_scan_pass24;
	case 32: 
		goto art_scan_pass29;
	case 46: case 62: 
		goto art_scan_pass228;
	default: goto art_scan_pass211;
	}
art_scan_pass222:
	if (s == e) return last;
	switch (*s++)
	{
	case 0: 
		goto art_scan_pass20;
	case 10: 
		goto art_scan_pass24;
	case 32: 
		goto art_scan_pass228;
	case 46: case 62: 
		goto art_scan_pass230;
	default: goto art_scan_pass213;
	}
art_scan_pass224:
	if (s == e) return last;
	switch (*s++)
	{
	case 0: 
		goto art_scan_pass20;
	case 10: 
		goto art_scan_pass23;
	case 32: case 46: case 62: 
		goto art_scan_pass224;
	default: goto art_scan_pass27;
	}
art_scan_pass228:
	if (s == e) return last;
	switch (*s++)
	{
	case 0: 
		goto art_scan_pass20;
	case 10: 
		goto art_scan_pass24;
	case 32: case 46: case 62: 
		goto art_scan_pass228;
	default: goto art_scan_pass211;
	}
art_scan_pass230:
	if (s == e) return last;
	switch (*s++)
	{
	case 0: 
		goto art_scan_pass20;
	case 10: 
		goto art_scan_pass24;
	case 32: case 46: case 62: 
		goto art_scan_pass230;
	default: goto art_scan_pass213;
	}
}

void art_scan(const unsigned char** s, const unsigned char** e)
{
	*s = art_scan_pass1(*s, *e);
	if (*s) *e = art_scan_pass2(*s, *e);
}
