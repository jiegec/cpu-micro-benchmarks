from matplotlib import pyplot as plt
import csv

patterns = [
    "data dependency 1 st + 1 ld",
    "data dependency 1 st + 2 ld",
    "addr dependency 1 st + 1 ld",
    "addr dependency 1 st + 2 ld",
    "no dependency 1 st + 1 ld",
]
pattern_data = []
size_data = []
min_data = []
avg_data = []

with open("memory_dependency_predictor_size.csv", newline="") as f:
    r = csv.DictReader(f)
    for row in r:
        pattern_data.append(int(row["pattern"]))
        size_data.append(float(row["size"]))
        min_data.append(float(row["min"]))
        avg_data.append(float(row["avg"]))


def find_spike(x, y, info):
    res = []
    for i in range(len(y) - 1):
        if (float(y[i + 1]) - y[i]) / y[i] > 0.2 and abs(y[i + 1] - y[i]) > 1.0:
            print(f"Spike detected at {int(x[i])} for {info}: {y[i]} -> {y[i+1]}")
            res.append(x[i])
    return res


for i in range(len(patterns)):
    x_data = []
    y_data = []
    for j in range(len(pattern_data)):
        if pattern_data[j] == i:
            x_data.append(size_data[j])
            y_data.append(min_data[j])
    find_spike(x_data, y_data, patterns[i])
    plt.plot(x_data, y_data, label=patterns[i])

plt.ylabel("Cycles")
plt.xlabel("Number of groups")
plt.grid()
plt.legend()
plt.savefig("plot_memory_dependency_predictor_size.png")
plt.cla()
