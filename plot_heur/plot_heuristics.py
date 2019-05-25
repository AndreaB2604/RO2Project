import sys
import numpy as np
import matplotlib.pyplot as plt
from scipy import interpolate 
import math
import fileinput

if __name__ == '__main__':
	problem = []
	for line in sys.argv[1:]:
		f = open(line,"r")
		name = f.readline().strip("\n")
		#problem.append(name)
		value = []
		time = []
		for i, line in enumerate(f):
			chuncks = line.split(" ")
			value.append(float(chuncks[0]))
			time.append(float(chuncks[1].strip("\n")))
		#print(value)
		#print(time)
		plt.plot(time, value, label=name)
		
	for i, prob in enumerate(problem):
		problem[i] = prob.strip("\n")

	plt.xlabel('Time (s)')
	plt.ylabel('Value of the objective functions')
	y = np.array(value)
	#plt.yticks(np.arange(y.min(), y.max(), (y.max()-y.min())/10))
	plt.yticks(np.arange(11900, 12900, 200))
	plt.legend()
	plt.grid(True, linewidth=0.25)
	#plt.title(problem)
	plt.show()

	plt.savefig("plot_heur/plot_heur.pdf", format='pdf')
	



