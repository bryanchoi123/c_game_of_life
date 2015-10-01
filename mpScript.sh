# add MPI to path
export PATH=/home/dapa/mpich2-install/bin:$PATH
#
# start script
inputFile=initialState
outputFile=output
iterations=20
printIterations=10
threads=2
maxThreads=40
while [ $threads -le $maxThreads ]
do
	mpirun -np $threads ./mp6 $inputFile $outputFile $iterations $printIterations
done

echo "Done"
