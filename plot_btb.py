from matplotlib import pyplot as plt
import csv

# Mimic https://chipsandcheese.com/2023/10/27/cortex-x2-arm-aims-high/

size_data = []
stride_data = []
min_data = []
avg_data = []

with open('btb.csv', newline='') as f:
	r = csv.DictReader(f)
	for row in r:
		size_data.append(float(row["size"]))
		stride_data.append(float(row["stride"]))
		min_data.append(float(row["min"]))
		avg_data.append(float(row["avg"]))

for stride in [4, 8, 16, 32, 64]:
	x_data = []
	y_data = []
	for i in range(len(stride_data)):
		if stride_data[i] == stride:
			x_data.append(size_data[i])
			y_data.append(min_data[i])
	plt.plot(x_data, y_data, label=f"Branch Per {stride}B")
plt.xscale('log')
ticks = [4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192]
plt.xticks(ticks, ticks)
plt.yticks(range(10))
plt.ylim((0, 10))
plt.xlabel('Branches in loop')
plt.ylabel('Cycles Per Branch')
plt.legend()
plt.savefig('plot_btb.png')