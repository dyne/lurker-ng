/* Meticulously hand-crafted regex scanner for: url_scan */
const char* url_scan_pass1(const char* s, const char* e)
{
	const char* last = 0;
	goto url_scan_pass115;
	
url_scan_pass10:
	return last;
url_scan_pass11:
	last = e;
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto url_scan_pass11;
	case '\x2f': 
		goto url_scan_pass15;
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': 
		goto url_scan_pass110;
	case '\x2e': 
		goto url_scan_pass114;
	default: goto url_scan_pass115;
	}
url_scan_pass12:
	last = e;
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x2f': 
		goto url_scan_pass15;
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto url_scan_pass18;
	case '\x2d': 
		goto url_scan_pass111;
	case '\x2e': 
		goto url_scan_pass113;
	default: goto url_scan_pass115;
	}
url_scan_pass13:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto url_scan_pass11;
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': 
		goto url_scan_pass110;
	default: goto url_scan_pass115;
	}
url_scan_pass14:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x3a': 
		goto url_scan_pass13;
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto url_scan_pass110;
	default: goto url_scan_pass115;
	}
url_scan_pass15:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x2f': 
		goto url_scan_pass14;
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto url_scan_pass110;
	default: goto url_scan_pass115;
	}
url_scan_pass16:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x77': 
		goto url_scan_pass12;
	case '\x2f': 
		goto url_scan_pass15;
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x78': case '\x79': case '\x7a': 
		goto url_scan_pass18;
	case '\x2d': 
		goto url_scan_pass111;
	case '\x2e': 
		goto url_scan_pass113;
	default: goto url_scan_pass115;
	}
url_scan_pass17:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x2f': 
		goto url_scan_pass15;
	case '\x77': 
		goto url_scan_pass16;
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x78': case '\x79': case '\x7a': 
		goto url_scan_pass18;
	case '\x2d': 
		goto url_scan_pass111;
	case '\x2e': 
		goto url_scan_pass113;
	default: goto url_scan_pass115;
	}
url_scan_pass18:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x2f': 
		goto url_scan_pass15;
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto url_scan_pass18;
	case '\x2d': 
		goto url_scan_pass111;
	case '\x2e': 
		goto url_scan_pass113;
	default: goto url_scan_pass115;
	}
url_scan_pass110:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x2f': 
		goto url_scan_pass15;
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto url_scan_pass110;
	case '\x2e': 
		goto url_scan_pass114;
	default: goto url_scan_pass115;
	}
url_scan_pass111:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto url_scan_pass18;
	case '\x2d': 
		goto url_scan_pass111;
	default: goto url_scan_pass115;
	}
url_scan_pass113:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x77': 
		goto url_scan_pass17;
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x78': case '\x79': case '\x7a': 
		goto url_scan_pass18;
	default: goto url_scan_pass115;
	}
url_scan_pass114:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto url_scan_pass18;
	default: goto url_scan_pass115;
	}
url_scan_pass115:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto url_scan_pass110;
	default: goto url_scan_pass115;
	}
}

const char* url_scan_pass2(const char* s, const char* e)
{
	const char* last = 0;
	goto url_scan_pass219;
	
url_scan_pass20:
	return last;
url_scan_pass21:
	last = s;
	if (s == e) return last;
	switch (*s++)
	{
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto url_scan_pass22;
	case '\x25': case '\x2c': case '\x2d': case '\x2e': case '\x3a': case '\x5f': case '\x7e': 
		goto url_scan_pass217;
	case '\x3f': 
		goto url_scan_pass235;
	default: goto url_scan_pass20;
	}
url_scan_pass22:
	last = s;
	if (s == e) return last;
	switch (*s++)
	{
	case '\x2e': 
		goto url_scan_pass21;
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto url_scan_pass22;
	case '\x2d': 
		goto url_scan_pass24;
	case '\x25': case '\x2c': case '\x3a': case '\x5f': case '\x7e': 
		goto url_scan_pass217;
	case '\x2f': 
		goto url_scan_pass227;
	case '\x3f': 
		goto url_scan_pass235;
	default: goto url_scan_pass20;
	}
url_scan_pass24:
	last = s;
	if (s == e) return last;
	switch (*s++)
	{
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto url_scan_pass22;
	case '\x2d': 
		goto url_scan_pass24;
	case '\x25': case '\x2c': case '\x2e': case '\x3a': case '\x5f': case '\x7e': 
		goto url_scan_pass217;
	case '\x3f': 
		goto url_scan_pass235;
	default: goto url_scan_pass20;
	}
url_scan_pass25:
	last = s;
	if (s == e) return last;
	switch (*s++)
	{
	case '\x25': case '\x2c': case '\x2d': case '\x2e': case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x3a': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x5f': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': case '\x7e': 
		goto url_scan_pass25;
	case '\x2f': 
		goto url_scan_pass227;
	case '\x3f': 
		goto url_scan_pass235;
	default: goto url_scan_pass20;
	}
url_scan_pass217:
	last = s;
	if (s == e) return last;
	switch (*s++)
	{
	case '\x25': case '\x2c': case '\x2d': case '\x2e': case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x3a': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x5f': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': case '\x7e': 
		goto url_scan_pass217;
	case '\x3f': 
		goto url_scan_pass235;
	default: goto url_scan_pass20;
	}
url_scan_pass218:
	last = s;
	if (s == e) return last;
	switch (*s++)
	{
	case '\x25': case '\x26': case '\x2b': case '\x2c': case '\x2d': case '\x2e': case '\x2f': case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x3a': case '\x3b': case '\x3d': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x5f': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': case '\x7e': 
		goto url_scan_pass218;
	default: goto url_scan_pass20;
	}
url_scan_pass219:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x78': case '\x79': case '\x7a': 
		goto url_scan_pass220;
	case '\x77': 
		goto url_scan_pass221;
	default: goto url_scan_pass20;
	}
url_scan_pass220:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto url_scan_pass220;
	case '\x3a': 
		goto url_scan_pass224;
	default: goto url_scan_pass20;
	}
url_scan_pass221:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x78': case '\x79': case '\x7a': 
		goto url_scan_pass220;
	case '\x77': 
		goto url_scan_pass222;
	case '\x3a': 
		goto url_scan_pass224;
	default: goto url_scan_pass20;
	}
url_scan_pass222:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x78': case '\x79': case '\x7a': 
		goto url_scan_pass220;
	case '\x77': 
		goto url_scan_pass223;
	case '\x3a': 
		goto url_scan_pass224;
	default: goto url_scan_pass20;
	}
url_scan_pass223:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto url_scan_pass220;
	case '\x3a': 
		goto url_scan_pass224;
	case '\x2e': 
		goto url_scan_pass229;
	default: goto url_scan_pass20;
	}
url_scan_pass224:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x2f': 
		goto url_scan_pass225;
	default: goto url_scan_pass20;
	}
url_scan_pass225:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x2f': 
		goto url_scan_pass226;
	default: goto url_scan_pass20;
	}
url_scan_pass226:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto url_scan_pass22;
	default: goto url_scan_pass20;
	}
url_scan_pass227:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x25': case '\x2c': case '\x2d': case '\x2e': case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x3a': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x5f': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': case '\x7e': 
		goto url_scan_pass25;
	default: goto url_scan_pass20;
	}
url_scan_pass229:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto url_scan_pass231;
	default: goto url_scan_pass20;
	}
url_scan_pass231:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x2e': 
		goto url_scan_pass226;
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto url_scan_pass231;
	case '\x2d': 
		goto url_scan_pass232;
	default: goto url_scan_pass20;
	}
url_scan_pass232:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto url_scan_pass231;
	case '\x2d': 
		goto url_scan_pass232;
	default: goto url_scan_pass20;
	}
url_scan_pass235:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x25': case '\x26': case '\x2b': case '\x2c': case '\x2d': case '\x2e': case '\x2f': case '\x30': case '\x31': case '\x32': case '\x33': case '\x34': case '\x35': case '\x36': case '\x37': case '\x38': case '\x39': case '\x3a': case '\x3b': case '\x3d': case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x5f': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': case '\x7e': 
		goto url_scan_pass218;
	default: goto url_scan_pass20;
	}
}

void url_scan(const char** s, const char** e)
{
	*s = url_scan_pass1(*s, *e);
	if (*s) *e = url_scan_pass2(*s, *e);
}
