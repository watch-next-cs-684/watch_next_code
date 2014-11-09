#!/usr/bin/python

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import csv, sys, os, time
import datetime

dt = datetime.datetime.now()
datenow = str(dt.date())
inspo2file = datenow+'-spo2.txt'
inpulsefile = datenow+'-pulse.txt'
outfile = datenow+'-pulse-spo2.png'

Mcsv = csv.reader(open(inspo2file, 'rb'), delimiter=' ', quotechar='"')

# Matrix of all the rows
Mspo2 = []

rowNum = 0
for row in Mcsv:
    rowNum += 1
    Mspo2.append(row)

Mcsv = csv.reader(open(inpulsefile, 'rb'), delimiter=' ', quotechar='"')

# Matrix of all the rows
Mpulse = []

rowNum = 0
for row in Mcsv:
    rowNum += 1
    Mpulse.append(row)



########## Plotting section

times_col = map(lambda r: datetime.datetime.strptime(r[0], "%H:%M:%S"), Mspo2)
spo2_col = map(lambda r: r[1], Mspo2)
timep_col = map(lambda r: datetime.datetime.strptime(r[0], "%H:%M:%S"), Mpulse)
pulse_col = map(lambda r: r[1], Mpulse)

plt.plot(timep_col, pulse_col, label='Pulse', ls='-', color='red', marker='o', markersize=9, mew=2, linewidth=2)
plt.plot(times_col, spo2_col, label='Spo2', ls='--', color='green', marker='^', markersize=9, mew=2, linewidth=2)
plt.gcf().autofmt_xdate()

plt.xlabel('Time')
plt.ylabel('pulse/spo2')
plt.grid(b='on')
plt.legend(loc=0)
plt.savefig(outfile, bbox_inches='tight')
