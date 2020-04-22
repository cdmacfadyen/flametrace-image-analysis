import os
resultsDir = "./results/"

os.chdir("..")

for filename in os.listdir(resultsDir):
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
        
        first_fire_frame_truth = 0 
        first_fire_frame_predicted = 0

        for i in range(0, shorterLength):
            labelDataValue = labelData[i]
            outputDataValue = outputData[i]

            if labelDataValue == "1" and first_fire_frame_truth == 0:
                first_fire_frame_truth = i
            
            if outputDataValue == "1" and first_fire_frame_predicted == 0 and first_fire_frame_truth != 0:
                first_fire_frame_predicted = i
            
            if labelDataValue == "0" and outputDataValue == "0":
                trueNegatives += 1
            elif labelDataValue == "1" and outputDataValue == "0":
                falseNegatives += 1
            elif labelDataValue == "0" and outputDataValue == "1":
                falsePositives += 1
            elif labelDataValue == "1" and outputDataValue == "1":
                truePositives += 1
        analFile = open(resultsDir + tokens[0] + ".anl", "w")
        analFile.write("tpos: {}\ntneg: {}\nfpos: {}\nfneg: {}\n"
            .format(truePositives, trueNegatives, falsePositives, falseNegatives))
        analFile.write("frames_to_detect: {}".format(first_fire_frame_predicted - first_fire_frame_truth))

        print("tpos: {} tneg: {} fpos: {} fneg: {}"
            .format(truePositives, trueNegatives, falsePositives, falseNegatives))