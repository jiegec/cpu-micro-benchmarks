from matplotlib import pyplot as plt
import csv

size_data = []
min_data = []
avg_data = []

with open('rob_size.csv', newline='') as f:
	r = csv.DictReader(f)
	for row in r:
		size_data.append(float(row["size"]))
		min_data.append(float(row["min"]))
		avg_data.append(float(row["avg"]))

plt.plot(size_data, min_data, label="min")
plt.plot(size_data, avg_data, label="avg")
plt.ylabel('Time')
plt.xlabel('Instruction Block Size')
plt.legend()
plt.savefig('plot_rob_size.png')
