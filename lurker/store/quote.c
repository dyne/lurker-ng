/* Meticulously hand-crafted regex scanner for: quote_scan */
const char* quote_scan_pass1(const char* s, const char* e)
{
	const char* last = 0;
	goto quote_scan_pass19;
	
quote_scan_pass10:
	return last;
quote_scan_pass11:
	last = e;
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x20': 
		goto quote_scan_pass17;
	case '\x00': 
		goto quote_scan_pass19;
	default: goto quote_scan_pass18;
	}
quote_scan_pass12:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x0a': 
		goto quote_scan_pass11;
	case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto quote_scan_pass13;
	case '\x20': 
		goto quote_scan_pass17;
	case '\x00': 
		goto quote_scan_pass19;
	default: goto quote_scan_pass18;
	}
quote_scan_pass13:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x0a': 
		goto quote_scan_pass11;
	case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto quote_scan_pass14;
	case '\x20': 
		goto quote_scan_pass17;
	case '\x00': 
		goto quote_scan_pass19;
	default: goto quote_scan_pass18;
	}
quote_scan_pass14:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x0a': 
		goto quote_scan_pass11;
	case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto quote_scan_pass15;
	case '\x20': 
		goto quote_scan_pass17;
	case '\x00': 
		goto quote_scan_pass19;
	default: goto quote_scan_pass18;
	}
quote_scan_pass15:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x0a': 
		goto quote_scan_pass11;
	case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto quote_scan_pass16;
	case '\x20': 
		goto quote_scan_pass17;
	case '\x00': 
		goto quote_scan_pass19;
	default: goto quote_scan_pass18;
	}
quote_scan_pass16:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x0a': 
		goto quote_scan_pass11;
	case '\x20': 
		goto quote_scan_pass17;
	case '\x00': 
		goto quote_scan_pass19;
	default: goto quote_scan_pass18;
	}
quote_scan_pass17:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x3a': case '\x3e': 
		goto quote_scan_pass12;
	case '\x20': 
		goto quote_scan_pass17;
	case '\x00': 
		goto quote_scan_pass19;
	default: goto quote_scan_pass18;
	}
quote_scan_pass18:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x20': 
		goto quote_scan_pass17;
	case '\x00': 
		goto quote_scan_pass19;
	default: goto quote_scan_pass18;
	}
quote_scan_pass19:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x0a': 
		goto quote_scan_pass18;
	default: goto quote_scan_pass19;
	}
}

const char* quote_scan_pass2(const char* s, const char* e)
{
	const char* last = 0;
	goto quote_scan_pass22;
	
quote_scan_pass20:
	return last;
quote_scan_pass21:
	last = s;
	if (s == e) return last;
	switch (*s++)
	{
	case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto quote_scan_pass24;
	case '\x3a': case '\x3e': 
		goto quote_scan_pass28;
	default: goto quote_scan_pass20;
	}
quote_scan_pass22:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x0a': 
		goto quote_scan_pass23;
	default: goto quote_scan_pass20;
	}
quote_scan_pass23:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto quote_scan_pass24;
	case '\x3a': case '\x3e': 
		goto quote_scan_pass28;
	default: goto quote_scan_pass20;
	}
quote_scan_pass24:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto quote_scan_pass25;
	case '\x3a': case '\x3e': 
		goto quote_scan_pass28;
	default: goto quote_scan_pass20;
	}
quote_scan_pass25:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto quote_scan_pass26;
	case '\x3a': case '\x3e': 
		goto quote_scan_pass28;
	default: goto quote_scan_pass20;
	}
quote_scan_pass26:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x41': case '\x42': case '\x43': case '\x44': case '\x45': case '\x46': case '\x47': case '\x48': case '\x49': case '\x4a': case '\x4b': case '\x4c': case '\x4d': case '\x4e': case '\x4f': case '\x50': case '\x51': case '\x52': case '\x53': case '\x54': case '\x55': case '\x56': case '\x57': case '\x58': case '\x59': case '\x5a': case '\x61': case '\x62': case '\x63': case '\x64': case '\x65': case '\x66': case '\x67': case '\x68': case '\x69': case '\x6a': case '\x6b': case '\x6c': case '\x6d': case '\x6e': case '\x6f': case '\x70': case '\x71': case '\x72': case '\x73': case '\x74': case '\x75': case '\x76': case '\x77': case '\x78': case '\x79': case '\x7a': 
		goto quote_scan_pass27;
	case '\x3a': case '\x3e': 
		goto quote_scan_pass28;
	default: goto quote_scan_pass20;
	}
quote_scan_pass27:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x3a': case '\x3e': 
		goto quote_scan_pass28;
	default: goto quote_scan_pass20;
	}
quote_scan_pass28:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x20': 
		goto quote_scan_pass29;
	default: goto quote_scan_pass20;
	}
quote_scan_pass29:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x00': 
		goto quote_scan_pass20;
	case '\x0a': 
		goto quote_scan_pass21;
	default: goto quote_scan_pass29;
	}
}

void quote_scan(const char** s, const char** e)
{
	*s = quote_scan_pass1(*s, *e);
	if (*s) *e = quote_scan_pass2(*s, *e);
}
