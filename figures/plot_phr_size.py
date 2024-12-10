from matplotlib import pyplot as plt
import csv

size_data = []
min_data = []

with open("phr_size.csv", newline="") as f:
    r = csv.DictReader(f)
    for row in r:
        size_data.append(float(row["size"]))
        min_data.append(float(row["avg"]) * 100)

plt.figure(figsize=(5, 2))
plt.plot(size_data, min_data)
plt.yticks([0, 50])
plt.ylabel("Misprediction rate (%)")
plt.xlabel("# Branches before the last conditional branch")
plt.savefig("plot_phr_size.png")
plt.savefig("plot_phr_size.pdf", bbox_inches="tight")
