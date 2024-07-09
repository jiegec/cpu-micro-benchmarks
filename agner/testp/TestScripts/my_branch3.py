import subprocess
from matplotlib import pyplot as plt
import numpy as np

# Reproduce Table 2 of Half&Half
# Test branch toggles
x_data = range(0, 16)
y_data = range(185, 195)
z_data = []
for branch_toggle in x_data:
    temp = []
    for dummy_branches in y_data:
        output = subprocess.check_output(
            ["./my_branch3.sh2", "16", "16", str(branch_toggle), "-1", str(dummy_branches)], encoding="utf-8"
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
        # skip misprediction from dummy branches
        avg = (np.average(np.array(data)) - 1000) / 2000  # 2 branches, 1000 loops
        print(branch_toggle, dummy_branches, f"{avg:.2f}")
        temp.append(avg)
    z_data.append(temp)

plt.imshow(z_data)
plt.xlabel("Dummy branches")
plt.xticks(range(len(y_data)), y_data, rotation=90)
plt.ylabel("Branch toggle bit")
plt.yticks(x_data)
plt.savefig('my_branch3_1.png')
plt.cla()

# Test target toggles
x_data = range(0, 6)
y_data = range(185, 195)
z_data = []
for target_toggle in x_data:
    temp = []
    for dummy_branches in y_data:
        output = subprocess.check_output(
            ["./my_branch3.sh2", "16", "16", "-1", str(target_toggle), str(dummy_branches)], encoding="utf-8"
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
        # skip misprediction from dummy branches
        avg = (np.average(np.array(data)) - 1000) / 2000  # 2 branches, 1000 loops
        print(target_toggle, dummy_branches, f"{avg:.2f}")
        temp.append(avg)
    z_data.append(temp)

plt.imshow(z_data)
plt.xlabel("Dummy branches")
plt.xticks(range(len(y_data)), y_data, rotation=90)
plt.ylabel("Target toggle bit")
plt.yticks(x_data)
plt.savefig('my_branch3_2.png')
plt.cla()

