from sklearn.metrics import confusion_matrix
import plotml
import os
resultsDir = "./results/"

os.chdir("..")

y_predicted = []
y_true = []

# for filename in os.listdir(resultsDir):
#     if ".out" in filename:
#         print(filename)
#         falsePositives = 0
#         falseNegatives = 0
#         trueNegatives = 0
#         truePositives = 0

#         tokens = filename.split(".")
#         labelFileName = tokens[0] + ".lbl"

#         labelFile = open(resultsDir + labelFileName)
#         outputFile = open(resultsDir + filename)

#         labelData = labelFile.readline()
#         outputData = outputFile.readline()

#         labelDataLength = len(labelData)
#         outputDataLength = len(outputData)

#         shorterLength = min([labelDataLength, outputDataLength])
        
#         for i in range(0, shorterLength):
#           y_predicted.append(outputData[i])
#           y_true.append(labelData[i])

#         cm = confusion_matrix(y_true, y_predicted)
#         plotml.plot_confusion_matrix(cm, ["fire", "no-fire" ])
#         y_predicted = []
#         y_true = []

# Just use the 4 videos in the test set. 
total_pred = []
total_actual = []
for filename in ["dry-flask-converted.out", "fire-2-orig-converted.out", "solvents-2.out", "video-of-video-converted.out"]:
    if ".out" in filename:
        print(filename)
        falsePositives = 0
        falseNegatives = 0
        trueNegatives = 0
        truePositives = 0

        tokens = filename.split(".")
        labelFileName = tokens[0] + ".lbl"

        labelFile = open(resultsDir + labelFileName)
        outputFile = open(resultsDir + filename)

        labelData = labelFile.readline()
        outputData = outputFile.readline()

        labelDataLength = len(labelData)
        outputDataLength = len(outputData)

        shorterLength = min([labelDataLength, outputDataLength])
        
        
        for i in range(0, shorterLength):
          if outputData[i] == "\n" or labelData[i] == "\n":
            break
          
          y_predicted.append(int(outputData[i]))
          y_true.append(int(labelData[i]))

        cm = confusion_matrix(y_true, y_predicted, [0,1])
        plotml.plot_confusion_matrix(cm, ["No Fire", "Fire" ])
        total_actual.extend(y_true)
        total_pred.extend(y_predicted)
        y_predicted = []
        y_true = []

final_cm = confusion_matrix(total_actual, total_pred,  [0,1])


plotml.plot_confusion_matrix(final_cm, ["No Fire", "Fire"])