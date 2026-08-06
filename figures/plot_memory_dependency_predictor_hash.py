from matplotlib import pyplot as plt
import csv

size_data = []
min_data = []
avg_data = []

with open("memory_dependency_predictor_hash.csv", newline="") as f:
    r = csv.DictReader(f)
    for row in r:
        size_data.append(float(row["bit"]))
        min_data.append(float(row["min"]))
        avg_data.append(float(row["avg"]))

plt.plot(size_data, min_data)
plt.xlabel("Bit")
plt.ylabel("Cycle")
plt.savefig("plot_memory_dependency_predictor_hash.png")
