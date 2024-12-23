from matplotlib import pyplot as plt
import csv


size_data = []
cycles_data = []

with open("dtlb_size.csv", newline="") as f:
    r = csv.DictReader(f)
    for row in r:
        size_data.append(float(row["pages"]))
        cycles_data.append(float(row["cycles"]))

plt.figure(figsize=(7, 6))
plt.plot(size_data, cycles_data)
plt.ylabel("Cycles")
plt.xlabel("Pages")
plt.grid()
plt.savefig("plot_dtlb_size.png")
plt.cla()
