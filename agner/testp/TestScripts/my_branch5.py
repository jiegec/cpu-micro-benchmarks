import subprocess
from matplotlib import pyplot as plt
import numpy as np

# Reproduce Figure 5 of Half&Half
x_data = range(1, 20)
y_data = []
for branch_align in x_data:
    output = subprocess.check_output(
        ["./my_branch5.sh2", str(branch_align)], encoding="utf-8"
    )
    heading = False
    data = []
    for line in output.splitlines():
        parts = list(filter(lambda s: len(s) > 0, line.strip().split(" ")))
        if len(parts) > 0:
            if not heading:
                assert parts[5] == "BrMisCond"
                heading = True
            else:
                data.append(int(parts[4]))
    avg = np.average(np.array(data)) / 1000  # 1 branches, 1000 loops
    print(branch_align, f"{avg:.2f}")
    y_data.append(avg)

plt.plot(x_data, y_data)
plt.xlabel("Branch alignment bits")
plt.ylabel("Miss Rate")
plt.savefig("my_branch5.png")
