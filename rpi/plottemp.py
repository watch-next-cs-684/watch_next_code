#!/usr/bin/python

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import csv, sys, os, time
import datetime

dt = datetime.datetime.now()
datenow = str(dt.date())
infile = datenow+'-temp.txt'
outfile = datenow+'-temp.png'

Mcsv = csv.reader(open(infile, 'rb'), delimiter=' ', quotechar='"')

# Matrix of all the rows
M = []

rowNum = 0
for row in Mcsv:
    rowNum += 1
    M.append(row)



########## Plotting section

time_col = map(lambda r: datetime.datetime.strptime(r[0], "%H:%M:%S"), M)
temp_col = map(lambda r: r[1], M)

plt.plot(time_col, temp_col, label='Temperature', ls='-', color='red', marker='o', markersize=9, mew=2, linewidth=2)
plt.gcf().autofmt_xdate()

plt.xlabel('Time')
plt.ylabel('Temperature (F)')
plt.grid(b='on')
plt.legend(loc=0)
plt.savefig(outfile, bbox_inches='tight')
