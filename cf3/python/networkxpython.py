from cStringIO import StringIO
import sys
import copy
import matplotlib.pyplot as plt
import networkx as nx
import coolfluid as cf

# some settings
fgcolor='#444444'
bgcolor='black'
componentcolor='#444444'

def show_graph(starturi):

  # create the collector component
  root = cf.Core.root()
  nxp = root.create_component('NetworkXPython','cf3.python.NetworkXPython')

  # getting the string which holds the script to setup the graph of the components
  components=nxp.get_component_graph( uri = starturi )
  #print components

  # getting the string which holds the script to setup the graph of the components
  options=nxp.get_option_graph( uri = starturi )
  print options

  # dictionary to the fancy name and a second smaller line for types
  nodecaption = dict()
  nodenote = dict()

  # setup and show in a mathplotlib window
  fig=plt.figure(figsize=(100,100))
  fig.patch.set_facecolor(bgcolor)
  plt.axes([0.,0.,1.,1.],axisbg=bgcolor)
  plt.title("COOLFluiD3 component graph starting from '" + str(starturi) + "'",
    position=(0.02,0.02), size=20, weight='bold',
    color=fgcolor, ha='left')
  plt.axis('off')

  # constructing graph or directional graph
  G=nx.DiGraph()
  #G=nx.Graph()

  # executing the submission into the graph
  for line in StringIO(components):
    exec line

  # setting the layout
  #pos=nx.spring_layout(G)
  pos=nx.graphviz_layout(G,prog='twopi')
  #pos=nx.graphviz_layout(G,prog='circo')

  # draw the contents, node_shape: so^>v<dph8
  nx.draw_networkx_nodes(G,pos,node_color=componentcolor,node_size=70,node_shape='8',linewidths=2)
  nx.draw_networkx_edges(G,pos,edge_color=fgcolor,width=2,arrows=False,style='solid')
  for i,j in nodecaption.iteritems():
    nodecaption[i] += '\n\n\n'
    nodenote[i] = '( ' + nodenote[i] + ' )\n\n'
  #nx.draw_networkx_labels(G,pos,labels=nodenote,
    #font_size=8,font_color=fgcolor,font_family='sans-serif',
    #verticalalignment='center')
  #nx.draw_networkx_labels(G,pos,labels=nodecaption,
    #font_size=11,font_color=fgcolor,font_weight='bold',font_family='sans-serif',
    #verticalalignment='center')

  # showing the plot
  plt.show()

  # delete the collector component

