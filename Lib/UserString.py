#!/usr/bin/env python
## vim:ts=4:et:nowrap
"""A user-defined wrapper around string objects

Note: string objects have grown methods in Python 1.6 
This module requires Python 1.6 or later.
"""
from types import StringType, UnicodeType
import sys

class UserString:
    def __init__(self, seq):
        if isinstance(seq, StringType) or isinstance(seq, UnicodeType):
            self.data = seq
        elif isinstance(seq, UserString):
            self.data = seq.data[:]
        else: 
            self.data = str(seq)
    def __str__(self): return str(self.data)
    def __repr__(self): return repr(self.data)
    def __int__(self): return int(self.data)
    def __long__(self): return long(self.data)
    def __float__(self): return float(self.data)
    def __complex__(self): return complex(self.data)
    def __hash__(self): return hash(self.data)

    def __cmp__(self, string):
        if isinstance(string, UserString):
            return cmp(self.data, string.data)
        else:
            return cmp(self.data, string)
    def __contains__(self, char):
        return char in self.data

    def __len__(self): return len(self.data)
    def __getitem__(self, index): return self.__class__(self.data[index])
    def __getslice__(self, start, end):
        start = max(start, 0); end = max(end, 0)
        return self.__class__(self.data[start:end])

    def __add__(self, other):
        if isinstance(other, UserString):
            return self.__class__(self.data + other.data)
        elif isinstance(other, StringType) or isinstance(other, UnicodeType):
            return self.__class__(self.data + other)
        else:
            return self.__class__(self.data + str(other))
    def __radd__(self, other):
        if isinstance(other, StringType) or isinstance(other, UnicodeType):
            return self.__class__(other + self.data)
        else:
            return self.__class__(str(other) + self.data)
    def __mul__(self, n):
        return self.__class__(self.data*n)
    __rmul__ = __mul__

    # the following methods are defined in alphabetical order:
    def capitalize(self): return self.__class__(self.data.capitalize())
    def center(self, width): return self.__class__(self.data.center(width))
    def count(self, sub, start=0, end=sys.maxint):
        return self.data.count(sub, start, end)
    def encode(self, encoding=None, errors=None): # XXX improve this?
        if encoding:
            if errors:
                return self.__class__(self.data.encode(encoding, errors))
            else:
                return self.__class__(self.data.encode(encoding))
        else: 
            return self.__class__(self.data.encode())
    def endswith(self, suffix, start=0, end=sys.maxint):
        return self.data.endswith(suffix, start, end)
    def expandtabs(self, tabsize=8): 
        return self.__class__(self.data.expandtabs(tabsize))
    def find(self, sub, start=0, end=sys.maxint): 
        return self.data.find(sub, start, end)
    def index(self, sub, start=0, end=sys.maxint): 
        return self.data.index(sub, start, end)
    def isdecimal(self): return self.data.isdecimal()
    def isdigit(self): return self.data.isdigit()
    def islower(self): return self.data.islower()
    def isnumeric(self): return self.data.isnumeric()
    def isspace(self): return self.data.isspace()
    def istitle(self): return self.data.istitle()
    def isupper(self): return self.data.isupper()
    def join(self, seq): return self.data.join(seq)
    def ljust(self, width): return self.__class__(self.data.ljust(width))
    def lower(self): return self.__class__(self.data.lower())
    def lstrip(self): return self.__class__(self.data.lstrip())
    def replace(self, old, new, maxsplit=-1): 
        return self.__class__(self.data.replace(old, new, maxsplit))
    def rfind(self, sub, start=0, end=sys.maxint): 
        return self.data.rfind(sub, start, end)
    def rindex(self, sub, start=0, end=sys.maxint): 
        return self.data.rindex(sub, start, end)
    def rjust(self, width): return self.__class__(self.data.rjust(width))
    def rstrip(self): return self.__class__(self.data.rstrip())
    def split(self, sep=None, maxsplit=-1): 
        return self.data.split(sep, maxsplit)
    def splitlines(self, maxsplit=-1): return self.data.splitlines(maxsplit)
    def startswith(self, prefix, start=0, end=sys.maxint): 
        return self.data.startswith(prefix, start, end)
    def strip(self): return self.__class__(self.data.strip())
    def swapcase(self): return self.__class__(self.data.swapcase())
    def title(self): return self.__class__(self.data.title())
    def translate(self, table, deletechars=""): 
        return self.__class__(self.data.translate(table, deletechars))
    def upper(self): return self.__class__(self.data.upper())

class MutableString(UserString):
    """mutable string objects

    Python strings are immutable objects.  This has the advantage, that
    strings may be used as dictionary keys.  If this property isn't needed
    and you insist on changing string values in place instead, you may cheat
    and use MutableString.

    But the purpose of this class is an educational one: to prevent
    people from inventing their own mutable string class derived
    from UserString and than forget thereby to remove (override) the
    __hash__ method inherited from ^UserString.  This would lead to
    errors that would be very hard to track down.

    A faster and better solution is to rewrite your program using lists."""
    def __init__(self, string=""):
        self.data = string
    def __hash__(self): 
        raise TypeError, "unhashable type (it is mutable)"
    def __setitem__(self, index, sub):
        if index < 0 or index >= len(self.data): raise IndexError
        self.data = self.data[:index] + sub + self.data[index+1:]
    def __delitem__(self, index):
        if index < 0 or index >= len(self.data): raise IndexError
        self.data = self.data[:index] + self.data[index+1:]
    def __setslice__(self, start, end, sub):
        start = max(start, 0); end = max(end, 0)
        if isinstance(sub, UserString):
            self.data = self.data[:start]+sub.data+self.data[end:]
        elif isinstance(sub, StringType) or isinstance(sub, UnicodeType):
            self.data = self.data[:start]+sub+self.data[end:]
        else:
            self.data =  self.data[:start]+str(sub)+self.data[end:]
    def __delslice__(self, start, end):
        start = max(start, 0); end = max(end, 0)
        self.data = self.data[:start] + self.data[end:]
    def immutable(self):
        return UserString(self.data)
    
if __name__ == "__main__":
    # execute the regression test to stdout, if called as a script:
    import os
    called_in_dir, called_as = os.path.split(sys.argv[0])
    called_in_dir = os.path.abspath(called_in_dir)
    called_as, py = os.path.splitext(called_as)
    sys.path.append(os.path.join(called_in_dir, 'test'))
    if '-q' in sys.argv:
        import test_support
        test_support.verbose = 0
    __import__('test_' + called_as.lower())
