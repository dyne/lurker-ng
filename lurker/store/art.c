/* Meticulously hand-crafted regex scanner for: art_scan */
const char* art_scan_pass1(const char* s, const char* e)
{
	const char* last = 0;
	goto art_scan_pass116;
	
art_scan_pass10:
	return last;
art_scan_pass11:
	last = e;
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x09': 
		goto art_scan_pass12;
	case '\x20': 
		goto art_scan_pass114;
	case '\x00': 
		goto art_scan_pass116;
	default: goto art_scan_pass115;
	}
art_scan_pass12:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x0a': 
		goto art_scan_pass11;
	case '\x00': 
		goto art_scan_pass116;
	default: goto art_scan_pass12;
	}
art_scan_pass111:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x09': case '\x20': 
		goto art_scan_pass12;
	case '\x0a': 
		goto art_scan_pass115;
	case '\x00': 
		goto art_scan_pass116;
	default: goto art_scan_pass112;
	}
art_scan_pass112:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x09': 
		goto art_scan_pass12;
	case '\x20': 
		goto art_scan_pass111;
	case '\x0a': 
		goto art_scan_pass115;
	case '\x00': 
		goto art_scan_pass116;
	default: goto art_scan_pass112;
	}
art_scan_pass114:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x09': 
		goto art_scan_pass12;
	case '\x20': 
		goto art_scan_pass112;
	case '\x00': 
		goto art_scan_pass116;
	default: goto art_scan_pass115;
	}
art_scan_pass115:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x09': 
		goto art_scan_pass12;
	case '\x20': 
		goto art_scan_pass114;
	case '\x00': 
		goto art_scan_pass116;
	default: goto art_scan_pass115;
	}
art_scan_pass116:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x0a': 
		goto art_scan_pass115;
	default: goto art_scan_pass116;
	}
}

const char* art_scan_pass2(const char* s, const char* e)
{
	const char* last = 0;
	goto art_scan_pass22;
	
art_scan_pass20:
	return last;
art_scan_pass21:
	last = s;
	if (s == e) return last;
	switch (*s++)
	{
	case '\x00': case '\x0a': 
		goto art_scan_pass20;
	case '\x09': 
		goto art_scan_pass23;
	case '\x20': 
		goto art_scan_pass29;
	default: goto art_scan_pass215;
	}
art_scan_pass22:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x0a': 
		goto art_scan_pass215;
	default: goto art_scan_pass20;
	}
art_scan_pass23:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x00': 
		goto art_scan_pass20;
	case '\x0a': 
		goto art_scan_pass21;
	default: goto art_scan_pass23;
	}
art_scan_pass25:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x00': case '\x0a': 
		goto art_scan_pass20;
	case '\x09': case '\x20': 
		goto art_scan_pass23;
	default: goto art_scan_pass27;
	}
art_scan_pass27:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x00': case '\x0a': 
		goto art_scan_pass20;
	case '\x09': 
		goto art_scan_pass23;
	case '\x20': 
		goto art_scan_pass25;
	default: goto art_scan_pass27;
	}
art_scan_pass29:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x00': case '\x0a': 
		goto art_scan_pass20;
	case '\x09': 
		goto art_scan_pass23;
	case '\x20': 
		goto art_scan_pass27;
	default: goto art_scan_pass215;
	}
art_scan_pass215:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x00': case '\x0a': 
		goto art_scan_pass20;
	case '\x09': 
		goto art_scan_pass23;
	case '\x20': 
		goto art_scan_pass29;
	default: goto art_scan_pass215;
	}
}

void art_scan(const char** s, const char** e)
{
	*s = art_scan_pass1(*s, *e);
	if (*s) *e = art_scan_pass2(*s, *e);
}
