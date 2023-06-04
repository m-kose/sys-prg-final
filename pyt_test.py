# set the range of file IDs
start_id = 0
end_id = 999

# loop over file IDs in the range
for file_id in range(start_id, end_id+1):

    # set the file names using the chosen ID
    input_file = "./text/"+f"numbers_{file_id}.txt"
    output_file = "./encryption/"+f"encrypted_{file_id}.txt"
    #print("input file: ", input_file)
    #print("output file: ", output_file)
    
    # open the files for reading
    with open(input_file, "r") as input_f, open(output_file, "r") as output_f:

        # loop over the first 5 lines of each file
        for i in range(5):
            input_line = input_f.readline().strip()
            output_line = output_f.readline().strip()
            #print("i:",i ,"input: ",input_line, "output: ", output_line)

            # check if the output line is equal to input line multiplied by 0.5
            if float(output_line) != float(input_line) * 0.500:
                #print(f"Rule not satisfied for line {i+1} in file {file_id}")
                break

        else:
            print(f"Rule satisfied for the first 5 lines in file {file_id}")