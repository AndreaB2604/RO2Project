import sys
import numpy as np
import matplotlib.pyplot as plt
# from adjustText import adjust_text #extern library to install it conda install -c phlya adjusttext 
from scipy import interpolate 
import toyplot # pip install toyploy

# import ghostscript
import toyplot.png
import toyplot.pdf


def plot_vertexes(x,y,n):
    '''
    fig, ax = plt.subplots(figsize=(20,15))
    #p1 = plt.plot(figsize=(20,15))
    '''
    '''
    area = np.array([60 for i in range(len(x))])
    plt.scatter(x, y, s=area, marker= 'o', facecolors='none', edgecolors='b')
    plt.plot([x[0],x[1]],[y[0],y[1]], 'r--', alpha=0.7)
    texts = [plt.text((x[i] * (1 + 0.01) - adjust(x[i])), y[i] * (1 + 0.005) , str(i+1), fontsize=12) for i in range(len(x))]
    '''
    edges=np.array([[i,i+1] for i in range(1,48)]) #little trick to plot "null edges"
    coordinates=np.transpose(np.vstack((x,y)))
    extra_vertices=[n]
    # print str(coordinates.shape)
    # print str(coordinates[1])
    # layout = toyplot.layout.FruchtermanReingold()
    layout = toyplot.layout.FruchtermanReingold()
    vstyle = {"stroke":toyplot.color.black}
    vlstyle = {"fill":"white"}
    colormap = toyplot.color.LinearMap(toyplot.color.Palette(["white"]))
    canvas, axes, mark = toyplot.graph(edges, extra_vertices ,
                                       vcoordinates=coordinates, layout=layout,
                                       vcolor=colormap, vsize=18, vstyle=vstyle, width=1000)
    axes.show = True
    axes.aspect = None
    axes.y.show = False
    axes.x.show=False
    # adjust_text(texts, x=x, y=y, autoalign='y',
    #        only_move={'points':'y', 'text':'y'}, force_points=0.5)
    # adjust_text(texts)
    # for i in range(len(x)):
    #    ax.annotate(str(i), (x[i], y[i]), xytext=(x[i],y[i]-14))
    
    toyplot.pdf.render(canvas, "nodes.pdf")
    # plt.savefig('nodes', format='eps', dpi=1000)

def init_nodes(f, n, x, y):
    # print n
    # i = 0
    for i in range(n):
        line = f.readline()
        x[i] = float(line.split(" ")[0])
        y[i] = float(line.split(" ")[1])
        #i = i+1
            
def main(toplot):
    # print "hello"
    # print toplot
    f = open(toplot,"r")
    nnodes = int(f.readline())
    xcoord = np.zeros(nnodes)
    ycoord = np.zeros(nnodes)
    init_nodes(f,nnodes, xcoord, ycoord)
    #init_edges(args...)
    #for i in range(0,nnodes):
    #    print str(xcoord[i]) + " " + str(ycoord[i])
    plot_vertexes(xcoord, ycoord, nnodes)
            
    
if __name__ == '__main__':
    # if(len(sys.argv) == 1):
    # main("48cap")
	main(sys.argv[1])
