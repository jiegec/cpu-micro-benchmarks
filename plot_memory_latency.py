from matplotlib import pyplot as plt
import csv

size_data = []
time_data = []

with open('memory_latency.csv', newline='') as f:
	r = csv.reader(f)
	for row in r:
		if row[0] == "size":
			continue
		if len(row) != 2:
			continue
		size_data.append(float(row[0]))
		time_data.append(float(row[1]))

fig, ax = plt.subplots()

ax.plot(size_data, time_data)
ax.set_xscale('log')
ax.set_ylabel('Time (ns)')
ax.set_xlabel('Memory Block Size (B)')

plt.savefig('plot_memory_latency.png')
