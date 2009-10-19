/* Copyright: Public domain
 * Produced with reg2c for wesley@terpstra.ca
 * cvs id tag: $Id$
 *
 * Regular expression: ((((([a-zA-Z]-?)*[a-zA-Z])://((([A-Za-z0-9][a-zA-Z0-9\-]*[a-zA-Z0-9]|[a-zA-Z0-9])\.)*([A-Za-z0-9][a-zA-Z0-9\-]*[a-zA-Z0-9]|[a-zA-Z0-9])))|(www\.((([A-Za-z0-9][a-zA-Z0-9\-]*[a-zA-Z0-9]|[a-zA-Z0-9])\.)*([A-Za-z0-9][a-zA-Z0-9\-]*[a-zA-Z0-9]|[a-zA-Z0-9]))\.([A-Za-z0-9][a-zA-Z0-9\-]*[a-zA-Z0-9]|[a-zA-Z0-9])))(:([0-9]+))?(((/([A-Za-z@!%~0-9:.,_\-+]*|#))*\/([A-Za-z@!%~0-9:.,_\-+]*[A-Za-z0-9]|#)))?(\#([A-Za-z@!%~0-9:.,_\-+]*[A-Za-z0-9]|#))?(\?([A-Za-z%~0-9.,_=/:;+\&\-]*[A-Za-z0-9]|#))?)
 */

const unsigned char* url_scan_pass1(const unsigned char* s, const unsigned char* e)
{
	const unsigned char* last = 0;
	goto url_scan_pass131;
	
url_scan_pass10:
	last = e;
	if (s == e) return last;
	switch (*(--e))
	{
	case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto url_scan_pass10;
	case 45: 
		goto url_scan_pass13;
	case 47: 
		goto url_scan_pass16;
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: 
		goto url_scan_pass119;
	case 46: 
		goto url_scan_pass129;
	default: goto url_scan_pass131;
	}
url_scan_pass11:
	last = e;
	if (s == e) return last;
	switch (*(--e))
	{
	case 47: 
		goto url_scan_pass16;
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto url_scan_pass111;
	case 45: 
		goto url_scan_pass123;
	case 46: 
		goto url_scan_pass127;
	default: goto url_scan_pass131;
	}
url_scan_pass13:
	if (s == e) return last;
	switch (*(--e))
	{
	case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto url_scan_pass10;
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: 
		goto url_scan_pass119;
	default: goto url_scan_pass131;
	}
url_scan_pass15:
	if (s == e) return last;
	switch (*(--e))
	{
	case 58: 
		goto url_scan_pass13;
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto url_scan_pass119;
	default: goto url_scan_pass131;
	}
url_scan_pass16:
	if (s == e) return last;
	switch (*(--e))
	{
	case 47: 
		goto url_scan_pass15;
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto url_scan_pass119;
	default: goto url_scan_pass131;
	}
url_scan_pass17:
	if (s == e) return last;
	switch (*(--e))
	{
	case 119: 
		goto url_scan_pass11;
	case 47: 
		goto url_scan_pass16;
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 120: case 121: case 122: 
		goto url_scan_pass111;
	case 45: 
		goto url_scan_pass123;
	case 46: 
		goto url_scan_pass127;
	default: goto url_scan_pass131;
	}
url_scan_pass19:
	if (s == e) return last;
	switch (*(--e))
	{
	case 47: 
		goto url_scan_pass16;
	case 119: 
		goto url_scan_pass17;
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 120: case 121: case 122: 
		goto url_scan_pass111;
	case 45: 
		goto url_scan_pass123;
	case 46: 
		goto url_scan_pass127;
	default: goto url_scan_pass131;
	}
url_scan_pass111:
	if (s == e) return last;
	switch (*(--e))
	{
	case 47: 
		goto url_scan_pass16;
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto url_scan_pass111;
	case 45: 
		goto url_scan_pass123;
	case 46: 
		goto url_scan_pass127;
	default: goto url_scan_pass131;
	}
url_scan_pass119:
	if (s == e) return last;
	switch (*(--e))
	{
	case 47: 
		goto url_scan_pass16;
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto url_scan_pass119;
	case 46: 
		goto url_scan_pass129;
	default: goto url_scan_pass131;
	}
url_scan_pass123:
	if (s == e) return last;
	switch (*(--e))
	{
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto url_scan_pass111;
	case 45: 
		goto url_scan_pass123;
	default: goto url_scan_pass131;
	}
url_scan_pass127:
	if (s == e) return last;
	switch (*(--e))
	{
	case 119: 
		goto url_scan_pass19;
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 120: case 121: case 122: 
		goto url_scan_pass111;
	default: goto url_scan_pass131;
	}
url_scan_pass129:
	if (s == e) return last;
	switch (*(--e))
	{
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto url_scan_pass111;
	default: goto url_scan_pass131;
	}
url_scan_pass131:
	if (s == e) return last;
	switch (*(--e))
	{
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto url_scan_pass119;
	default: goto url_scan_pass131;
	}
}

const unsigned char* url_scan_pass2(const unsigned char* s, const unsigned char* e)
{
	const unsigned char* last = 0;
	goto url_scan_pass212;
	
url_scan_pass20:
	return last;
url_scan_pass21:
	last = s;
	if (s == e) return last;
	switch (*s++)
	{
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto url_scan_pass21;
	case 47: 
		goto url_scan_pass24;
	case 35: 
		goto url_scan_pass25;
	case 63: 
		goto url_scan_pass26;
	case 46: 
		goto url_scan_pass215;
	case 45: 
		goto url_scan_pass216;
	case 58: 
		goto url_scan_pass222;
	default: goto url_scan_pass20;
	}
url_scan_pass23:
	last = s;
	if (s == e) return last;
	switch (*s++)
	{
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: 
		goto url_scan_pass23;
	case 47: 
		goto url_scan_pass24;
	case 35: 
		goto url_scan_pass25;
	case 63: 
		goto url_scan_pass26;
	default: goto url_scan_pass20;
	}
url_scan_pass24:
	last = s;
	if (s == e) return last;
	switch (*s++)
	{
	case 47: case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto url_scan_pass24;
	case 35: 
		goto url_scan_pass25;
	case 63: 
		goto url_scan_pass26;
	case 33: case 37: case 43: case 44: case 45: case 46: case 58: case 64: case 95: case 126: 
		goto url_scan_pass223;
	default: goto url_scan_pass20;
	}
url_scan_pass25:
	last = s;
	if (s == e) return last;
	switch (*s++)
	{
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto url_scan_pass25;
	case 63: 
		goto url_scan_pass26;
	case 33: case 37: case 43: case 44: case 45: case 46: case 58: case 64: case 95: case 126: 
		goto url_scan_pass224;
	default: goto url_scan_pass20;
	}
url_scan_pass26:
	last = s;
	if (s == e) return last;
	switch (*s++)
	{
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto url_scan_pass26;
	case 37: case 38: case 43: case 44: case 45: case 46: case 47: case 58: case 59: case 61: case 95: case 126: 
		goto url_scan_pass225;
	default: goto url_scan_pass20;
	}
url_scan_pass27:
	if (s == e) return last;
	switch (*s++)
	{
	case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto url_scan_pass27;
	case 45: 
		goto url_scan_pass211;
	case 58: 
		goto url_scan_pass213;
	default: goto url_scan_pass20;
	}
url_scan_pass28:
	if (s == e) return last;
	switch (*s++)
	{
	case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 120: case 121: case 122: 
		goto url_scan_pass27;
	case 119: 
		goto url_scan_pass29;
	case 45: 
		goto url_scan_pass211;
	case 58: 
		goto url_scan_pass213;
	default: goto url_scan_pass20;
	}
url_scan_pass29:
	if (s == e) return last;
	switch (*s++)
	{
	case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 120: case 121: case 122: 
		goto url_scan_pass27;
	case 119: 
		goto url_scan_pass210;
	case 45: 
		goto url_scan_pass211;
	case 58: 
		goto url_scan_pass213;
	default: goto url_scan_pass20;
	}
url_scan_pass210:
	if (s == e) return last;
	switch (*s++)
	{
	case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto url_scan_pass27;
	case 45: 
		goto url_scan_pass211;
	case 58: 
		goto url_scan_pass213;
	case 46: 
		goto url_scan_pass217;
	default: goto url_scan_pass20;
	}
url_scan_pass211:
	if (s == e) return last;
	switch (*s++)
	{
	case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto url_scan_pass27;
	default: goto url_scan_pass20;
	}
url_scan_pass212:
	if (s == e) return last;
	switch (*s++)
	{
	case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 120: case 121: case 122: 
		goto url_scan_pass27;
	case 119: 
		goto url_scan_pass28;
	default: goto url_scan_pass20;
	}
url_scan_pass213:
	if (s == e) return last;
	switch (*s++)
	{
	case 47: 
		goto url_scan_pass214;
	default: goto url_scan_pass20;
	}
url_scan_pass214:
	if (s == e) return last;
	switch (*s++)
	{
	case 47: 
		goto url_scan_pass215;
	default: goto url_scan_pass20;
	}
url_scan_pass215:
	if (s == e) return last;
	switch (*s++)
	{
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto url_scan_pass21;
	default: goto url_scan_pass20;
	}
url_scan_pass216:
	if (s == e) return last;
	switch (*s++)
	{
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto url_scan_pass21;
	case 45: 
		goto url_scan_pass216;
	default: goto url_scan_pass20;
	}
url_scan_pass217:
	if (s == e) return last;
	switch (*s++)
	{
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto url_scan_pass219;
	default: goto url_scan_pass20;
	}
url_scan_pass219:
	if (s == e) return last;
	switch (*s++)
	{
	case 46: 
		goto url_scan_pass215;
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto url_scan_pass219;
	case 45: 
		goto url_scan_pass220;
	default: goto url_scan_pass20;
	}
url_scan_pass220:
	if (s == e) return last;
	switch (*s++)
	{
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto url_scan_pass219;
	case 45: 
		goto url_scan_pass220;
	default: goto url_scan_pass20;
	}
url_scan_pass222:
	if (s == e) return last;
	switch (*s++)
	{
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: 
		goto url_scan_pass23;
	default: goto url_scan_pass20;
	}
url_scan_pass223:
	if (s == e) return last;
	switch (*s++)
	{
	case 47: case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto url_scan_pass24;
	case 33: case 37: case 43: case 44: case 45: case 46: case 58: case 64: case 95: case 126: 
		goto url_scan_pass223;
	default: goto url_scan_pass20;
	}
url_scan_pass224:
	if (s == e) return last;
	switch (*s++)
	{
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto url_scan_pass25;
	case 33: case 37: case 43: case 44: case 45: case 46: case 58: case 64: case 95: case 126: 
		goto url_scan_pass224;
	default: goto url_scan_pass20;
	}
url_scan_pass225:
	if (s == e) return last;
	switch (*s++)
	{
	case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: 
		goto url_scan_pass26;
	case 37: case 38: case 43: case 44: case 45: case 46: case 47: case 58: case 59: case 61: case 95: case 126: 
		goto url_scan_pass225;
	default: goto url_scan_pass20;
	}
}

void url_scan(const unsigned char** s, const unsigned char** e)
{
	*s = url_scan_pass1(*s, *e);
	if (*s) *e = url_scan_pass2(*s, *e);
}
