/* Copyright: Public domain
 * Produced with reg2c for wesley@terpstra.ca
 * cvs id tag: $Id$
 *
 * Regular expression: (([A-Za-z0-9][a-zA-Z0-9._=\-]*[a-zA-Z0-9]|[a-zA-Z0-9])@((([A-Za-z0-9][a-zA-Z0-9\-]*[a-zA-Z0-9]|[a-zA-Z0-9])\.)*([A-Za-z0-9][a-zA-Z0-9\-]*[a-zA-Z0-9]|[a-zA-Z0-9]))\.([A-Za-z0-9][a-zA-Z0-9\-]*[a-zA-Z0-9]|[a-zA-Z0-9]))
 */

const unsigned char* mailto_scan_pass1(const unsigned char* s, const unsigned char* e)
{
	const unsigned char* last = 0;
	goto mailto_scan_pass117;
	
mailto_scan_pass10:
	last = e;
	if (s == e) return last;
	switch (*(--e))
	{
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto mailto_scan_pass10;
	case 45: 
		goto mailto_scan_pass13;
	case 46: 
		goto mailto_scan_pass14;
	case 61: case 95: 
		goto mailto_scan_pass17;
	case 64: 
		goto mailto_scan_pass19;
	default: goto mailto_scan_pass117;
	}
mailto_scan_pass12:
	last = e;
	if (s == e) return last;
	switch (*(--e))
	{
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto mailto_scan_pass12;
	case 46: 
		goto mailto_scan_pass14;
	case 45: case 61: case 95: 
		goto mailto_scan_pass17;
	default: goto mailto_scan_pass117;
	}
mailto_scan_pass13:
	if (s == e) return last;
	switch (*(--e))
	{
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto mailto_scan_pass10;
	case 45: 
		goto mailto_scan_pass13;
	case 46: case 61: case 95: 
		goto mailto_scan_pass17;
	default: goto mailto_scan_pass117;
	}
mailto_scan_pass14:
	if (s == e) return last;
	switch (*(--e))
	{
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto mailto_scan_pass10;
	case 45: case 46: case 61: case 95: 
		goto mailto_scan_pass17;
	default: goto mailto_scan_pass117;
	}
mailto_scan_pass17:
	if (s == e) return last;
	switch (*(--e))
	{
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto mailto_scan_pass12;
	case 45: case 46: case 61: case 95: 
		goto mailto_scan_pass17;
	default: goto mailto_scan_pass117;
	}
mailto_scan_pass19:
	if (s == e) return last;
	switch (*(--e))
	{
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto mailto_scan_pass12;
	default: goto mailto_scan_pass117;
	}
mailto_scan_pass110:
	if (s == e) return last;
	switch (*(--e))
	{
	case 64: 
		goto mailto_scan_pass19;
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto mailto_scan_pass110;
	case 45: 
		goto mailto_scan_pass112;
	case 46: 
		goto mailto_scan_pass113;
	default: goto mailto_scan_pass117;
	}
mailto_scan_pass112:
	if (s == e) return last;
	switch (*(--e))
	{
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto mailto_scan_pass110;
	case 45: 
		goto mailto_scan_pass112;
	default: goto mailto_scan_pass117;
	}
mailto_scan_pass113:
	if (s == e) return last;
	switch (*(--e))
	{
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto mailto_scan_pass110;
	default: goto mailto_scan_pass117;
	}
mailto_scan_pass116:
	if (s == e) return last;
	switch (*(--e))
	{
	case 46: 
		goto mailto_scan_pass113;
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto mailto_scan_pass116;
	default: goto mailto_scan_pass117;
	}
mailto_scan_pass117:
	if (s == e) return last;
	switch (*(--e))
	{
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto mailto_scan_pass116;
	default: goto mailto_scan_pass117;
	}
}

const unsigned char* mailto_scan_pass2(const unsigned char* s, const unsigned char* e)
{
	const unsigned char* last = 0;
	goto mailto_scan_pass22;
	
mailto_scan_pass20:
	return last;
mailto_scan_pass21:
	last = s;
	if (s == e) return last;
	switch (*s++)
	{
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto mailto_scan_pass21;
	case 46: 
		goto mailto_scan_pass26;
	case 45: 
		goto mailto_scan_pass29;
	default: goto mailto_scan_pass20;
	}
mailto_scan_pass22:
	if (s == e) return last;
	switch (*s++)
	{
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto mailto_scan_pass24;
	default: goto mailto_scan_pass20;
	}
mailto_scan_pass23:
	if (s == e) return last;
	switch (*s++)
	{
	case 45: case 46: case 61: case 95: 
		goto mailto_scan_pass23;
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto mailto_scan_pass24;
	default: goto mailto_scan_pass20;
	}
mailto_scan_pass24:
	if (s == e) return last;
	switch (*s++)
	{
	case 45: case 46: case 61: case 95: 
		goto mailto_scan_pass23;
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto mailto_scan_pass24;
	case 64: 
		goto mailto_scan_pass25;
	default: goto mailto_scan_pass20;
	}
mailto_scan_pass25:
	if (s == e) return last;
	switch (*s++)
	{
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto mailto_scan_pass27;
	default: goto mailto_scan_pass20;
	}
mailto_scan_pass26:
	if (s == e) return last;
	switch (*s++)
	{
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto mailto_scan_pass21;
	default: goto mailto_scan_pass20;
	}
mailto_scan_pass27:
	if (s == e) return last;
	switch (*s++)
	{
	case 46: 
		goto mailto_scan_pass26;
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto mailto_scan_pass27;
	case 45: 
		goto mailto_scan_pass28;
	default: goto mailto_scan_pass20;
	}
mailto_scan_pass28:
	if (s == e) return last;
	switch (*s++)
	{
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto mailto_scan_pass27;
	case 45: 
		goto mailto_scan_pass28;
	default: goto mailto_scan_pass20;
	}
mailto_scan_pass29:
	if (s == e) return last;
	switch (*s++)
	{
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto mailto_scan_pass21;
	case 45: 
		goto mailto_scan_pass29;
	default: goto mailto_scan_pass20;
	}
}

void mailto_scan(const unsigned char** s, const unsigned char** e)
{
	*s = mailto_scan_pass1(*s, *e);
	if (*s) *e = mailto_scan_pass2(*s, *e);
}
