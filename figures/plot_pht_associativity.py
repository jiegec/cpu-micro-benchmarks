import subprocess
from matplotlib import pyplot as plt
import numpy as np
import csv

# Test branch toggles
for prefix in [""]:
    x_data = []
    y_data = []
    z_data = []

    with open(f"{prefix}pht_associativity.csv", newline="") as f:
        r = csv.DictReader(f)
        for row in r:
            x_data.append(int(row["branches"]))
            y_data.append(int(row["align"]))
            z_data.append(min(float(row["min"]), 0.5))
        z_data = np.array(z_data)

    x_data = list(sorted(set(x_data)))
    y_data = list(sorted(set(y_data)))
    z_data = z_data.reshape((len(x_data), len(y_data)))

    plt.imshow(z_data.transpose())
    plt.xlabel("# Conditional branches")
    plt.xticks(range(len(x_data)), x_data, rotation=90)
    plt.ylabel("Log2 branch base address")
    plt.yticks(range(len(y_data)), y_data)
    bar = plt.colorbar(shrink=0.5)
    bar.ax.set_ylabel("Misprediction rate", fontsize=8, rotation=270, labelpad=9.0)
    plt.savefig(f"plot_{prefix}pht_associativity.png")
    plt.savefig(f"plot_{prefix}pht_associativity.pdf", bbox_inches="tight")
    plt.cla()
