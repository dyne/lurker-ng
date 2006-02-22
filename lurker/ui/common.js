var oldClass = "";

function rollOut(thisItem)
{
	thisItem.className = oldClass;
}

function rollIn(thisItem)
{
	oldClass = thisItem.className;
	thisItem.className = "rowover";
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

function titledate(thisItem, utc)
{
	var d = new Date(utc*1000);
	var s = thisItem.getAttribute('title');
	s =	s.substring(0, s.length-16)
		+ d.getFullYear().toString()
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

function form_timezone(form)
{
	var d = new Date(0);
	d.setSeconds(form.sec.value);
	d.setMinutes(form.min.value);
	d.setHours(form.hour.value);
	d.setDate(form.mday.value);
	d.setMonth(form.mon.value-1);
	d.setFullYear(form.year.value);
	form.utc.value = d.valueOf() / 1000;
}

function readCookie(name)
{
	var nameEQ = name + "=";
	var ca = document.cookie.split(';');
	for(var i=0;i < ca.length;i++)
	{
		var c = ca[i];
		while (c.charAt(0)==' ') c = c.substring(1,c.length);
		if (c.indexOf(nameEQ) == 0) return c.substring(nameEQ.length,c.length);
	}
	return null;
}

function trash(kill)
{
	if (readCookie('lurker-pass') == null)
	{
		var pass = prompt('Enter the admin password to confirm deletion');
		if (pass != null)
		{
			document.cookie = "lurker-pass="+pass+"; path=/";
			self.location = kill;
		}
	}
	else
	{
		if (confirm('Really delete this message?'))
		{
			self.location = kill;
		}
	}
}
