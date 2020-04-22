import matplotlib.pyplot as plt


predicted = []
actual = []

with open("../results/fire-2-orig-converted.lbl") as f:
  labels = f.readline()
  actual = [int(char) for char in labels if char != "\n"]

with open("../results/fire-2-orig-converted.out") as f:
  labels = f.readline()
  predicted = [int(char) for char in labels if char != "\n"]

actual.pop()    # labels is missing one for some reason

fig, ax = plt.subplots()

y = range(0, len(actual))
ax.plot(y, predicted, label="Predicted label")
ax.plot(y, actual, label="Actual label")
ax.legend()

ax.set_yticks([0,1])
ax.set_yticklabels(["No fire", "Fire"])
ax.set_xlabel("Frame number")
ax.set_title("Predicted and actual label for every frame in video:\n Heuristic implementation")

plt.savefig("./figures/predicted-v-actual-heuristic.png")
