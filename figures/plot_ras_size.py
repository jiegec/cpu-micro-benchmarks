from matplotlib import pyplot as plt
import csv

size_data = []
min_data = []
avg_data = []

with open('ras_size.csv', newline='') as f:
	r = csv.DictReader(f)
	for row in r:
		size_data.append(float(row["size"]))
		min_data.append(float(row["min"]))
		avg_data.append(float(row["avg"]))

plt.plot(size_data, min_data, label="min")
plt.plot(size_data, avg_data, label="avg")
plt.ylabel('Time')
plt.xlabel('Call Depth')
plt.legend()
plt.grid()
plt.savefig('plot_ras_size.png')
