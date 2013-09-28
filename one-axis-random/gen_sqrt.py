#!/usr/bin/python

# Generate an approximate square root table.
# The domain is v**2 + 2*a.
# 0 <= v <= 93500; 0 <= a <= 86600.
# So it looks like a 32 bit square root.  (Actually, about 33 bits.)
# What could be easier?

# (a + b)**2 = a**2 + 2ab + b**2
# a**2 + b**2 = (a + b) * (a + b) - 2ab
# sqrt(a**2 + b**2) == (a + b) - sqrt(2ab) ??? No...

# If I can make the magic 1/7 thing work, that should give me
# a sqrt within 20 clocks or so...
# If I get the initial guess with a lookup table, that will help.

