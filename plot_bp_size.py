from matplotlib import pyplot as plt
import csv
import numpy as np

size_data = []
history_data = []
min_data = []
avg_data = []

with open('bp_size.csv', newline='') as f:
	r = csv.DictReader(f)
	for row in r:
		size_data.append(float(row["size"]))
		history_data.append(float(row["history"]))
		min_data.append(float(row["min"]))
		avg_data.append(float(row["avg"]))

plt.imshow(np.array(avg_data).reshape((11, 17)))
plt.colorbar()

xticks = [1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768,65536]
plt.xticks(range(len(xticks)), xticks, rotation='vertical')

yticks = [1,2,4,8,16,32,64,128,256,512,1024]
plt.yticks(range(len(yticks)), yticks)

plt.xlabel('Pattern Length')
plt.ylabel('Branch Num')
plt.savefig('plot_bp_size.png')
