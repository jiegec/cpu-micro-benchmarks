from matplotlib import pyplot as plt
import csv

x_data = []
y_data = []

with open('rob_size.csv', newline='') as f:
	r = csv.DictReader(f)
	for row in r:
		x_data.append(float(row["size"]))
		y_data.append(float(row["min"]))

plt.plot(x_data, y_data)
plt.savefig('plot_rob_size.png')
