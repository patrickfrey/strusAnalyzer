#!/usr/bin/python
import unicodedata

def convDiacriticalCharacter( input_str):
    nfkd_form = unicodedata.normalize('NFKD', input_str)
    only_ascii = nfkd_form.encode('ASCII', 'ignore')
    return only_ascii

def printConvDia():
    for x in range(33, 800000):
        u = unichr(x)
        l = convDiacriticalCharacter( u)
        if l != u and len(l) > 0:
           print "set( 0x%x, \"%s\"); /* '%s' -> '%s' */" % (ord(u),l,u,l) 

def printUppercase():
    for x in range(33, 800000):
        u = unichr(x)
        l = u.upper()
        if l != u:
           print "set( 0x%x, 0x%x); /* '%s' -> '%s' */" % (ord(u),ord(l),u,l) 

def printLowercase():
    for x in range(33, 800000):
        u = unichr(x)
        l = u.lower()
        if l != u:
           print "set( 0x%x, 0x%x); /* '%s' -> '%s' */" % (ord(u),ord(l),u,l) 

printConvDia();


