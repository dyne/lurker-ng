/* Meticulously hand-crafted regex scanner for: mailto_scan */
const char* mailto_scan_pass1(const char* s, const char* e)
{
	const char* last = 0;
	goto mailto_scan_pass118;
	
mailto_scan_pass11:
	last = e;
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto mailto_scan_pass11;
	case '\x2d': 
		goto mailto_scan_pass14;
	case '\x2e': 
		goto mailto_scan_pass15;
	case '\x3d': case '\x5f': 
		goto mailto_scan_pass18;
	case '\x40': 
		goto mailto_scan_pass110;
	default: goto mailto_scan_pass118;
	}
mailto_scan_pass13:
	last = e;
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto mailto_scan_pass13;
	case '\x2e': 
		goto mailto_scan_pass15;
	case '\x2d': case '\x3d': case '\x5f': 
		goto mailto_scan_pass18;
	default: goto mailto_scan_pass118;
	}
mailto_scan_pass14:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto mailto_scan_pass11;
	case '\x2d': 
		goto mailto_scan_pass14;
	case '\x2e': case '\x3d': case '\x5f': 
		goto mailto_scan_pass18;
	default: goto mailto_scan_pass118;
	}
mailto_scan_pass15:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto mailto_scan_pass11;
	case '\x2d': case '\x2e': case '\x3d': case '\x5f': 
		goto mailto_scan_pass18;
	default: goto mailto_scan_pass118;
	}
mailto_scan_pass18:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto mailto_scan_pass13;
	case '\x2d': case '\x2e': case '\x3d': case '\x5f': 
		goto mailto_scan_pass18;
	default: goto mailto_scan_pass118;
	}
mailto_scan_pass110:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto mailto_scan_pass13;
	default: goto mailto_scan_pass118;
	}
mailto_scan_pass111:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x40': 
		goto mailto_scan_pass110;
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto mailto_scan_pass111;
	case '\x2d': 
		goto mailto_scan_pass113;
	case '\x2e': 
		goto mailto_scan_pass114;
	default: goto mailto_scan_pass118;
	}
mailto_scan_pass113:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto mailto_scan_pass111;
	case '\x2d': 
		goto mailto_scan_pass113;
	default: goto mailto_scan_pass118;
	}
mailto_scan_pass114:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto mailto_scan_pass111;
	default: goto mailto_scan_pass118;
	}
mailto_scan_pass117:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x2e': 
		goto mailto_scan_pass114;
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto mailto_scan_pass117;
	default: goto mailto_scan_pass118;
	}
mailto_scan_pass118:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto mailto_scan_pass117;
	default: goto mailto_scan_pass118;
	}
}

const char* mailto_scan_pass2(const char* s, const char* e)
{
	const char* last = 0;
	goto mailto_scan_pass22;
	
mailto_scan_pass20:
	return last;
mailto_scan_pass21:
	last = s;
	if (s == e) return last;
	switch (*s++)
	{
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto mailto_scan_pass21;
	case '\x2e': 
		goto mailto_scan_pass26;
	case '\x2d': 
		goto mailto_scan_pass29;
	default: goto mailto_scan_pass20;
	}
mailto_scan_pass22:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto mailto_scan_pass24;
	default: goto mailto_scan_pass20;
	}
mailto_scan_pass23:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x2d': case '\x2e': case '\x3d': case '\x5f': 
		goto mailto_scan_pass23;
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto mailto_scan_pass24;
	default: goto mailto_scan_pass20;
	}
mailto_scan_pass24:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x2d': case '\x2e': case '\x3d': case '\x5f': 
		goto mailto_scan_pass23;
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto mailto_scan_pass24;
	case '\x40': 
		goto mailto_scan_pass25;
	default: goto mailto_scan_pass20;
	}
mailto_scan_pass25:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto mailto_scan_pass27;
	default: goto mailto_scan_pass20;
	}
mailto_scan_pass26:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto mailto_scan_pass21;
	default: goto mailto_scan_pass20;
	}
mailto_scan_pass27:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x2e': 
		goto mailto_scan_pass26;
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto mailto_scan_pass27;
	case '\x2d': 
		goto mailto_scan_pass28;
	default: goto mailto_scan_pass20;
	}
mailto_scan_pass28:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto mailto_scan_pass27;
	case '\x2d': 
		goto mailto_scan_pass28;
	default: goto mailto_scan_pass20;
	}
mailto_scan_pass29:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto mailto_scan_pass21;
	case '\x2d': 
		goto mailto_scan_pass29;
	default: goto mailto_scan_pass20;
	}
}

void mailto_scan(const char** s, const char** e)
{
	*s = mailto_scan_pass1(*s, *e);
	if (*s) *e = mailto_scan_pass2(*s, *e);
}
