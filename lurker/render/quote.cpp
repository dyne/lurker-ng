/* Copyright: Public domain
 * Produced with reg2c for wesley@terpstra.ca
 * cvs id tag: $Id: quote.cpp,v 1.2 2003-04-21 18:26:21 terpstra Exp $
 *
 * Regular expression: (
([a-zA-Z]?[a-zA-Z]?[a-zA-Z]?[a-zA-Z]?>)[-	-ÿ]*
(([a-zA-Z]?[a-zA-Z]?[a-zA-Z]?[a-zA-Z]?>)[-	-ÿ]*
)*)
 */

const unsigned char* quote_scan_pass1(const unsigned char* s, const unsigned char* e)
{
	const unsigned char* last = 0;
	goto quote_scan_pass17;
	
quote_scan_pass10:
	last = e;
	if (s == e) return last;
	switch (*(--e))
	{
	case 62: 
		goto quote_scan_pass11;
	case 0: 
		goto quote_scan_pass17;
	default: goto quote_scan_pass16;
	}
quote_scan_pass11:
	if (s == e) return last;
	switch (*(--e))
	{
	case 10: 
		goto quote_scan_pass10;
	case 62: 
		goto quote_scan_pass11;
	case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto quote_scan_pass12;
	case 0: 
		goto quote_scan_pass17;
	default: goto quote_scan_pass16;
	}
quote_scan_pass12:
	if (s == e) return last;
	switch (*(--e))
	{
	case 10: 
		goto quote_scan_pass10;
	case 62: 
		goto quote_scan_pass11;
	case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto quote_scan_pass13;
	case 0: 
		goto quote_scan_pass17;
	default: goto quote_scan_pass16;
	}
quote_scan_pass13:
	if (s == e) return last;
	switch (*(--e))
	{
	case 10: 
		goto quote_scan_pass10;
	case 62: 
		goto quote_scan_pass11;
	case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto quote_scan_pass14;
	case 0: 
		goto quote_scan_pass17;
	default: goto quote_scan_pass16;
	}
quote_scan_pass14:
	if (s == e) return last;
	switch (*(--e))
	{
	case 10: 
		goto quote_scan_pass10;
	case 62: 
		goto quote_scan_pass11;
	case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto quote_scan_pass15;
	case 0: 
		goto quote_scan_pass17;
	default: goto quote_scan_pass16;
	}
quote_scan_pass15:
	if (s == e) return last;
	switch (*(--e))
	{
	case 10: 
		goto quote_scan_pass10;
	case 62: 
		goto quote_scan_pass11;
	case 0: 
		goto quote_scan_pass17;
	default: goto quote_scan_pass16;
	}
quote_scan_pass16:
	if (s == e) return last;
	switch (*(--e))
	{
	case 62: 
		goto quote_scan_pass11;
	case 0: 
		goto quote_scan_pass17;
	default: goto quote_scan_pass16;
	}
quote_scan_pass17:
	if (s == e) return last;
	switch (*(--e))
	{
	case 10: 
		goto quote_scan_pass16;
	default: goto quote_scan_pass17;
	}
}

const unsigned char* quote_scan_pass2(const unsigned char* s, const unsigned char* e)
{
	const unsigned char* last = 0;
	goto quote_scan_pass22;
	
quote_scan_pass20:
	return last;
quote_scan_pass21:
	last = s;
	if (s == e) return last;
	switch (*s++)
	{
	case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto quote_scan_pass24;
	case 62: 
		goto quote_scan_pass28;
	default: goto quote_scan_pass20;
	}
quote_scan_pass22:
	if (s == e) return last;
	switch (*s++)
	{
	case 10: 
		goto quote_scan_pass23;
	default: goto quote_scan_pass20;
	}
quote_scan_pass23:
	if (s == e) return last;
	switch (*s++)
	{
	case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto quote_scan_pass24;
	case 62: 
		goto quote_scan_pass28;
	default: goto quote_scan_pass20;
	}
quote_scan_pass24:
	if (s == e) return last;
	switch (*s++)
	{
	case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto quote_scan_pass25;
	case 62: 
		goto quote_scan_pass28;
	default: goto quote_scan_pass20;
	}
quote_scan_pass25:
	if (s == e) return last;
	switch (*s++)
	{
	case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto quote_scan_pass26;
	case 62: 
		goto quote_scan_pass28;
	default: goto quote_scan_pass20;
	}
quote_scan_pass26:
	if (s == e) return last;
	switch (*s++)
	{
	case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto quote_scan_pass27;
	case 62: 
		goto quote_scan_pass28;
	default: goto quote_scan_pass20;
	}
quote_scan_pass27:
	if (s == e) return last;
	switch (*s++)
	{
	case 62: 
		goto quote_scan_pass28;
	default: goto quote_scan_pass20;
	}
quote_scan_pass28:
	if (s == e) return last;
	switch (*s++)
	{
	case 0: 
		goto quote_scan_pass20;
	case 10: 
		goto quote_scan_pass21;
	default: goto quote_scan_pass28;
	}
}

void quote_scan(const unsigned char** s, const unsigned char** e)
{
	*s = quote_scan_pass1(*s, *e);
	if (*s) *e = quote_scan_pass2(*s, *e);
}
