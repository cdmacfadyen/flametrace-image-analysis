import os
import subprocess
import time

os.chdir("..")

os.system("rm ./results/*.out")
start = time.time()
number_of_files = len(os.listdir("./videos/"))
for filename in os.listdir("./videos/"):
  print(filename)
  output = subprocess.check_call(["./flametrace", "./videos/" + filename, "-hide"])
  # output = subprocess.check_output(["./flametrace", "./videos/" + filename, "-hide"])
  # print(output)

end = time.time()

total_number_of_frames = -1
for filename in os.listdir("./results/"):
  if ".lbl" not in filename:  #only label files
    continue
  
  with open("./results/" + filename) as f:
    label = f.readline()
    total_number_of_frames += len(label)

avg_time_per_frame = (end - start) / total_number_of_frames
with open("results/avg-time-per-frame.txt", "w") as f:
  f.write(str(avg_time_per_frame))

print("{}s to run on {} files".format(end - start, number_of_files))
print("Avg time per frame = ", avg_time_per_frame)

# start = time.time()
# number_of_files = len(os.listdir("./videos/"))
# processes = []
# for filename in os.listdir("./videos/"):
#   p = subprocess.Popen(["./test", "./videos/" + filename, "-hide"])
#   processes.append(p)

# for process in processes:
#   process.wait()

# end = time.time()


# print("{}s to run on {} files".format(end - start, number_of_files))