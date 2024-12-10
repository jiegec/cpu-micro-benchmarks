import subprocess
from matplotlib import pyplot as plt
import numpy as np
import csv

# Test branch toggles
x_data = [[], []]
y_data = [[], []]
z_data = [[], []]

with open("pht_tag_bits_xor_phr.csv", newline="") as f:
    r = csv.DictReader(f)
    for row in r:
        target = int(row["target"])
        x_data[target].append(int(row["first_phr_bit"]))
        y_data[target].append(int(row["dummy_branches"]))
        z_data[target].append(float(row["min"]))
    for i in range(2):
        z_data[i] = np.array(z_data[i])

fig, axes = plt.subplots(2, figsize=(15, 10))
for i in range(2):
    x_data[i] = list(sorted(set(x_data[i])))
    y_data[i] = list(sorted(set(y_data[i])))
    z_data[i].resize(len(x_data[i]) * len(y_data[i]))
    z_data[i] = z_data[i].reshape((len(x_data[i]), len(y_data[i])))

    axes[i].imshow(z_data[i])
    if i == 0:
        axes[i].set_ylabel("PHRB bits")
    else:
        axes[i].set_ylabel("PHRT bits")
    axes[i].set_yticks(range(len(x_data[i])), x_data[i])
    axes[i].set_xlabel("PHRT bits")
    axes[i].set_xticks(range(len(y_data[i])), y_data[i], rotation=90)
    axes[i].grid()

plt.savefig("plot_pht_tag_bits_xor_phr.png")
plt.cla()
