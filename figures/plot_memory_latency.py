from matplotlib import pyplot as plt
import csv

size_data = []
time_data = []
llc_miss_data = []
llc_load_data = []

with open('memory_latency.csv', newline='') as f:
	r = csv.reader(f)
	for row in r:
		if row[0] == "size":
			continue
		if len(row) < 5:
			continue
		size_data.append(float(row[0]))
		time_data.append(float(row[1]))
		llc_miss_data.append(float(row[3]))
		llc_load_data.append(float(row[4]))

fig, ax = plt.subplots()

ax.plot(size_data, time_data)
ax.set_xscale('log')
ax.set_ylabel('Time (ns)')
ax.set_xlabel('Memory Block Size (B)')

ax2 = ax.twinx()

ax2.plot(size_data, llc_load_data, 'r.-', label='LLC Loads')
ax2.plot(size_data, llc_miss_data, 'g.-', label='LLC Misses')
ax2.legend()
ax2.set_xscale('log')
ax2.set_ylabel('LLC Load/Miss per Access')
ax2.set_xlabel('Memory Block Size (B)')

plt.savefig('plot_memory_latency.png')
