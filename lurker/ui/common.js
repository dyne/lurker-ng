var oldColor = "ffffff";

function rollOut(thisItem)
{
	thisItem.style.background      = oldColor;
	thisItem.style.backgroundColor = oldColor;
}

function rollIn(thisItem)
{
	if (thisItem.style.background)
	{
		oldColor = thisItem.style.background; 
		thisItem.style.background = "CCAACC"; 
	}
	else
	{
		oldColor = thisItem.style.backgroundColor;
		thisItem.style.backgroundColor = "CCAACC";
	}
}

function twostr(num)
{
	var out = '';
	if (num < 10) out += '0';
	out += num.toString();
	return out;
}

function textdate(utc)
{
	var d = new Date(utc*1000);
	var s = d.getFullYear().toString()
		+ '-' + twostr(d.getMonth()+1)
		+ '-' + twostr(d.getDate())
		+ '&#160;' + twostr(d.getHours()) // don't wrap text
		+ ':' + twostr(d.getMinutes());
	document.write(s);
}

function titledate(thisItem, prefix, utc)
{
	var d = new Date(utc*1000);
	var s = prefix + d.getFullYear().toString()
		+ '-' + twostr(d.getMonth()+1)
		+ '-' + twostr(d.getDate())
		+ ' ' + twostr(d.getHours()) // nbsp in titles is bad
		+ ':' + twostr(d.getMinutes());
	
	thisItem.setAttribute('title', s);
}

function timezone(utc)
{
	var d = new Date(utc*1000); // sadly the offset depends on the date
	var o = d.getTimezoneOffset(); // minutes, + = west

	var s = '';
	if (o < 0)
	{
		s += '+'; 
		o = 0 - o;
	}
	else	s += '-';
	
	var h = o / 60;
	var m = o % 60;
	s += h.toString() + twostr(m);
	document.write(s);
}