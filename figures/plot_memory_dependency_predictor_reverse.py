from matplotlib import pyplot as plt
import csv

pattern_data = []
allow_data = []
index_data = []
cycles_data = []
perf_data = []

patterns = ["100N-20Y-10N", "100N-20Y-1N-19Y", "100N-20Y-1N-9Y-1N-9Y","1000Y-10N using 10 loads", "100Y-20N-20Y"]

with open("memory_dependency_predictor_reverse.csv", newline="") as f:
    r = csv.DictReader(f)
    for row in r:
        pattern_data.append(int(row["pattern"]))
        allow_data.append(row["allow"])
        index_data.append(int(row["index"]))
        cycles_data.append(float(row["avg"]))
        perf_data.append(row["perf"])

for i, pattern in enumerate(patterns):
    for perf in ["cycles", "machine_clear"]:
        fig, ax = plt.subplots(ncols=1, figsize=(14, 5))

        x_data = []
        y_data = []
        c_data = []
        for j in range(len(pattern_data)):
            if pattern_data[j] == i and perf_data[j] == perf:
                if (i != 3 and index_data[j] >= 90) or (i == 3 and index_data[j] >= 980):
                    x_data.append(index_data[j])
                    y_data.append(cycles_data[j])
                    c_data.append(allow_data[j])

        ax.vlines(
            x_data,
            0,
            y_data,
            colors=["red" if x == "Y" else "green" for x in c_data],
        )
        ax.scatter(
            x_data,
            y_data,
            c=["red" if x == "Y" else "green" for x in c_data],
            label=pattern,
        )

        ax.set_ylabel(perf.title())
        ax.set_xlabel("Access index")
        if perf == "cycles":
            ax.set_ylim(0, max(y_data) + 10)
        ax.legend()

        dst = f"plot_memory_dependency_predictor_reverse_{perf}_{i}.png"
        print(f"Saved to {dst}")
        plt.savefig(
            dst,
            bbox_inches="tight",
        )
        plt.cla()
