# Some hard regexps
# abab|ba  in ...abab...  <-- first match is not leftmost
# aba|bbab in ...bbaba... <--- last match is not longest
# 

all:		test grep install

install:	url.c mailto.c quote.c art.c
		cp $^ ..

url.c:		reg2c host
		./reg2c scan url_scan "`./host url`" > $@

mailto.c:	reg2c host
		./reg2c scan mailto_scan "`./host mailto`" > $@

quote.c:	reg2c host
		./reg2c scan quote_scan "`./host quote`" > $@

art.c:		reg2c host
		./reg2c scan art_scan "`./host art`" > $@

test:		demoo.c test.c
		gcc -Wall -O2 -g $^ -o $@

grep:		demom.c grep.c
		gcc -Wall -O2 -g $^ -o $@

host:		host.c
		gcc -Wall -O2 -g $^ -o $@

demom.c:	reg2c host
		./reg2c match demo "`./host url`" > $@

demoo.c:	reg2c host
		./reg2c scan demo "`./host url`" > $@

reg2c:		reg2c.cc
		g++ -Wall -O2 -g $^ -o $@

reg2c.cc:	reg2c.yy
		bison $^ -o $@

clean:
		rm -f reg2c reg2c.cc demom.c demoo.c test grep host \
			url.c mailto.c quote.c art.c
