import subprocess
from matplotlib import pyplot as plt
import numpy as np
import csv

# collect phr bits
phr_bits = []
with open("pht_index_bits_xor_phr.csv", newline="") as f:
    r = csv.DictReader(f)
    for row in r:
        phr_bits.append(int(row["phr"]))

phr_bits = list(sorted(list(set(phr_bits))))
fig, axes = plt.subplots(len(phr_bits), figsize=(16, len(phr_bits) * 2.5))

for i, phr_bit in enumerate(phr_bits):
    x_data = []
    y_data = []
    z_data = []

    with open("pht_index_bits_xor_phr.csv", newline="") as f:
        r = csv.DictReader(f)
        for row in r:
            if int(row["phr"]) == phr_bit:
                x_data.append(int(row["branches"]))
                y_data.append(int(row["dummy"]))
                z_data.append(min(float(row["min"]), 0.5))
        z_data = np.array(z_data)

    x_data = list(sorted(set(x_data)))
    y_data = list(sorted(set(y_data)))
    z_data = z_data.reshape((len(x_data), len(y_data)))

    axes[i].imshow(z_data)
    axes[i].set_ylabel("Predict branches")
    axes[i].set_yticks(range(len(x_data)), x_data)
    axes[i].set_xlabel(f"PHR bit position injecting PHR[{phr_bit}]")
    axes[i].set_xticks(range(len(y_data)), y_data, rotation=90)
plt.subplots_adjust(hspace=1.0)
plt.savefig("plot_pht_index_bits_xor_phr.png")
plt.cla()
