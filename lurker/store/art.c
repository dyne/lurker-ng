/* Meticulously hand-crafted regex scanner for: art_scan */
const char* art_scan_pass1(const char* s, const char* e)
{
	const char* last = 0;
	goto art_scan_pass130;
	
art_scan_pass11:
	last = e;
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x0a': 
		goto art_scan_pass129;
	case '\x00': 
		goto art_scan_pass130;
	default: goto art_scan_pass12;
	}
art_scan_pass12:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x0a': 
		goto art_scan_pass11;
	case '\x00': 
		goto art_scan_pass130;
	default: goto art_scan_pass12;
	}
art_scan_pass114:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x20': 
		goto art_scan_pass114;
	case '\x2e': case '\x3e': 
		goto art_scan_pass116;
	case '\x0a': 
		goto art_scan_pass120;
	case '\x00': 
		goto art_scan_pass130;
	default: goto art_scan_pass12;
	}
art_scan_pass115:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x20': 
		goto art_scan_pass114;
	case '\x0a': 
		goto art_scan_pass120;
	case '\x00': 
		goto art_scan_pass130;
	default: goto art_scan_pass116;
	}
art_scan_pass116:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x20': 
		goto art_scan_pass115;
	case '\x0a': 
		goto art_scan_pass120;
	case '\x00': 
		goto art_scan_pass130;
	default: goto art_scan_pass116;
	}
art_scan_pass117:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x20': 
		goto art_scan_pass117;
	case '\x2e': case '\x3e': 
		goto art_scan_pass119;
	case '\x0a': 
		goto art_scan_pass120;
	case '\x00': 
		goto art_scan_pass130;
	default: goto art_scan_pass116;
	}
art_scan_pass118:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x20': 
		goto art_scan_pass117;
	case '\x0a': 
		goto art_scan_pass120;
	case '\x00': 
		goto art_scan_pass130;
	default: goto art_scan_pass119;
	}
art_scan_pass119:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x20': 
		goto art_scan_pass118;
	case '\x0a': 
		goto art_scan_pass120;
	case '\x00': 
		goto art_scan_pass130;
	default: goto art_scan_pass119;
	}
art_scan_pass120:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x20': 
		goto art_scan_pass118;
	case '\x0a': 
		goto art_scan_pass129;
	case '\x00': 
		goto art_scan_pass130;
	default: goto art_scan_pass119;
	}
art_scan_pass121:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x0a': 
		goto art_scan_pass120;
	case '\x00': 
		goto art_scan_pass130;
	default: goto art_scan_pass121;
	}
art_scan_pass124:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x20': 
		goto art_scan_pass124;
	case '\x2e': case '\x3e': 
		goto art_scan_pass126;
	case '\x0a': 
		goto art_scan_pass129;
	case '\x00': 
		goto art_scan_pass130;
	default: goto art_scan_pass121;
	}
art_scan_pass125:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x20': 
		goto art_scan_pass124;
	case '\x0a': 
		goto art_scan_pass129;
	case '\x00': 
		goto art_scan_pass130;
	default: goto art_scan_pass126;
	}
art_scan_pass126:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x20': 
		goto art_scan_pass125;
	case '\x0a': 
		goto art_scan_pass129;
	case '\x00': 
		goto art_scan_pass130;
	default: goto art_scan_pass126;
	}
art_scan_pass127:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x20': 
		goto art_scan_pass127;
	case '\x0a': case '\x2e': case '\x3e': 
		goto art_scan_pass129;
	case '\x00': 
		goto art_scan_pass130;
	default: goto art_scan_pass126;
	}
art_scan_pass128:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x20': 
		goto art_scan_pass127;
	case '\x00': 
		goto art_scan_pass130;
	default: goto art_scan_pass129;
	}
art_scan_pass129:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x20': 
		goto art_scan_pass128;
	case '\x00': 
		goto art_scan_pass130;
	default: goto art_scan_pass129;
	}
art_scan_pass130:
	if (s == e) return last;
	switch (*(--e))
	{
	case '\x0a': 
		goto art_scan_pass129;
	default: goto art_scan_pass130;
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
	default: goto art_scan_pass29;
	}
art_scan_pass22:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x0a': 
		goto art_scan_pass23;
	default: goto art_scan_pass20;
	}
art_scan_pass23:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x00': case '\x0a': 
		goto art_scan_pass20;
	case '\x20': case '\x2e': case '\x3e': 
		goto art_scan_pass25;
	default: goto art_scan_pass26;
	}
art_scan_pass24:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x00': case '\x0a': 
		goto art_scan_pass20;
	case '\x20': case '\x2e': case '\x3e': 
		goto art_scan_pass230;
	default: goto art_scan_pass213;
	}
art_scan_pass25:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x00': 
		goto art_scan_pass20;
	case '\x0a': 
		goto art_scan_pass23;
	case '\x20': case '\x2e': case '\x3e': 
		goto art_scan_pass25;
	default: goto art_scan_pass26;
	}
art_scan_pass26:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x00': 
		goto art_scan_pass20;
	case '\x0a': 
		goto art_scan_pass23;
	case '\x2e': case '\x3e': 
		goto art_scan_pass25;
	case '\x20': 
		goto art_scan_pass215;
	default: goto art_scan_pass26;
	}
art_scan_pass27:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x00': 
		goto art_scan_pass20;
	case '\x0a': 
		goto art_scan_pass23;
	case '\x20': 
		goto art_scan_pass216;
	case '\x2e': case '\x3e': 
		goto art_scan_pass224;
	default: goto art_scan_pass27;
	}
art_scan_pass28:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x00': 
		goto art_scan_pass20;
	case '\x0a': 
		goto art_scan_pass24;
	default: goto art_scan_pass28;
	}
art_scan_pass29:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x00': 
		goto art_scan_pass20;
	case '\x0a': 
		goto art_scan_pass21;
	default: goto art_scan_pass29;
	}
art_scan_pass211:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x00': 
		goto art_scan_pass20;
	case '\x0a': 
		goto art_scan_pass24;
	case '\x20': 
		goto art_scan_pass220;
	case '\x2e': case '\x3e': 
		goto art_scan_pass228;
	default: goto art_scan_pass211;
	}
art_scan_pass213:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x00': 
		goto art_scan_pass20;
	case '\x0a': 
		goto art_scan_pass24;
	case '\x20': 
		goto art_scan_pass222;
	case '\x2e': case '\x3e': 
		goto art_scan_pass230;
	default: goto art_scan_pass213;
	}
art_scan_pass215:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x00': 
		goto art_scan_pass20;
	case '\x0a': 
		goto art_scan_pass23;
	case '\x2e': case '\x3e': 
		goto art_scan_pass25;
	case '\x20': 
		goto art_scan_pass224;
	default: goto art_scan_pass26;
	}
art_scan_pass216:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x00': 
		goto art_scan_pass20;
	case '\x0a': 
		goto art_scan_pass23;
	case '\x20': 
		goto art_scan_pass28;
	case '\x2e': case '\x3e': 
		goto art_scan_pass224;
	default: goto art_scan_pass27;
	}
art_scan_pass220:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x00': 
		goto art_scan_pass20;
	case '\x0a': 
		goto art_scan_pass24;
	case '\x20': 
		goto art_scan_pass29;
	case '\x2e': case '\x3e': 
		goto art_scan_pass228;
	default: goto art_scan_pass211;
	}
art_scan_pass222:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x00': 
		goto art_scan_pass20;
	case '\x0a': 
		goto art_scan_pass24;
	case '\x20': 
		goto art_scan_pass228;
	case '\x2e': case '\x3e': 
		goto art_scan_pass230;
	default: goto art_scan_pass213;
	}
art_scan_pass224:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x00': 
		goto art_scan_pass20;
	case '\x0a': 
		goto art_scan_pass23;
	case '\x20': case '\x2e': case '\x3e': 
		goto art_scan_pass224;
	default: goto art_scan_pass27;
	}
art_scan_pass228:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x00': 
		goto art_scan_pass20;
	case '\x0a': 
		goto art_scan_pass24;
	case '\x20': case '\x2e': case '\x3e': 
		goto art_scan_pass228;
	default: goto art_scan_pass211;
	}
art_scan_pass230:
	if (s == e) return last;
	switch (*s++)
	{
	case '\x00': 
		goto art_scan_pass20;
	case '\x0a': 
		goto art_scan_pass24;
	case '\x20': case '\x2e': case '\x3e': 
		goto art_scan_pass230;
	default: goto art_scan_pass213;
	}
}

void art_scan(const char** s, const char** e)
{
	*s = art_scan_pass1(*s, *e);
	if (*s) *e = art_scan_pass2(*s, *e);
}
