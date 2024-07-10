import subprocess
from matplotlib import pyplot as plt
import numpy as np

# Reproduce Figure 4 of Half&Half
x_data = range(0, 16)
y_data = range(0, 6)
z_data = []
for branch_toggle in x_data:
    temp = []
    for target_toggle in y_data:
        output = subprocess.check_output(
            ["./my_branch2.sh2", "16", "16", str(branch_toggle), str(target_toggle)],
            encoding="utf-8",
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
        avg = np.average(np.array(data)) / 2000  # 2 branches, 1000 loops
        print(branch_toggle, target_toggle, f"{avg:.2f}")
        temp.append(avg)
    z_data.append(temp)

plt.imshow(z_data)
plt.xlabel("Target toggle bit")
plt.xticks(y_data)
plt.ylabel("Branch toggle bit")
plt.yticks(x_data)
plt.savefig("my_branch2.png")
