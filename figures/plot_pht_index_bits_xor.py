import subprocess
from matplotlib import pyplot as plt
import numpy as np
import csv

# Test branch toggles
x_data = []
y_data = []
z_data = []

with open("pht_index_bits_xor.csv", newline="") as f:
    r = csv.DictReader(f)
    for row in r:
        x_data.append(int(row["branches"]))
        y_data.append(int(row["dummy"]))
        z_data.append(min(float(row["min"]), 0.5))
    z_data = np.array(z_data)

x_data = list(sorted(set(x_data)))
y_data = list(sorted(set(y_data)))
z_data = z_data.reshape((len(x_data), len(y_data)))

plt.figure(figsize=(20, 6))
plt.imshow(z_data)
plt.ylabel("Predict branches")
plt.yticks(range(len(x_data)), x_data)
plt.xlabel("PHR bit position")
plt.xticks(range(len(y_data)), y_data, rotation=90)
plt.savefig("plot_pht_index_bits_xor.png")
plt.cla()
