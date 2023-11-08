#!/usr/bin/python3
import cgi, cgitb 

import os

for param in os.environ.keys():
   print("%s: %s" % (param, os.environ[param]))

form = cgi.FieldStorage()
for field_name in form.keys():
    field_value = form[field_name].value
    print(f"{field_name}: {field_value}")