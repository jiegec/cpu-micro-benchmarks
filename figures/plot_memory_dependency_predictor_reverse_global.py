from matplotlib import pyplot as plt
import csv

size_data = []
avg_data = []

with open("memory_dependency_predictor_reverse_global.csv", newline="") as f:
    r = csv.DictReader(f)
    for row in r:
        size_data.append(float(row["size"]))
        avg_data.append(float(row["avg"]))

plt.plot(size_data, avg_data)
plt.ylabel("Indepedent Loads")
plt.xlabel("Machine Clears")
plt.savefig("plot_memory_dependency_predictor_reverse_global.png")
