from matplotlib import pyplot as plt
import csv

size_data = []
time_data = []

with open('memory_latency.csv', newline='') as f:
	r = csv.DictReader(f)
	for row in r:
		size_data.append(float(row["size"]))
		time_data.append(float(row["time(ns)"]))

fig, ax = plt.subplots()

ax.plot(size_data, time_data)
ax.set_xscale('log')
ax.set_ylabel('Time')
ax.set_xlabel('Memory Block Size')

plt.savefig('plot_memory_latency.png')
