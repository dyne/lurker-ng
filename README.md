# Lurker-NG

Lurker is a great mailman archiver, but is not updated since 2010

Since we use it at Dyne.org we'll try maintain it and fix some bugs

The first urgent one is about clash of unique message ids

So we need a proper random generation using hashing in this file:

https://github.com/dyne/lurker-ng/blob/master/lurker/common/MessageId.cpp

Any contribution welcome.

