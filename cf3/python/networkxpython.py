from cStringIO import StringIO
from copy import deepcopy
import numpy
import Image
import base64
import sys
import matplotlib.pyplot as plt
import networkx as nx
import coolfluid as cf

# some settings
titlecolor='white'
bgcolor='black'

componentcolor='#444444'
optioncolor='#0e3815'
signalcolor='#401144'
fieldcolor='#112544'
linkcolor='#444444'

rootpng="""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
AAAAAElFTkSuQmCC"""


def traditional_left2right_tree_layout() :
  pos=dict()
  return pos

def show_graph(starturi,depth=1000):

  # decode and load root component's image
  rootpng_bin=StringIO(base64.decodestring(rootpng))
  root_img=Image.open(rootpng_bin)
  print root_img

  # create the collector component
  root = cf.Core.root()
  nxp = root.create_component('NetworkXPython','cf3.python.NetworkXPython')

  # get a string with a nice name of the component
  start_key=root.access_component(str(starturi)).uri().path()

  # getting the strings which holds the script to setup the graph
  print "---------- PREPROCESS - C++ ----------"
  components=nxp.get_component_graph( uri = starturi, depth = depth )
  options=nxp.get_option_graph( uri = starturi, depth = depth )
  signals=nxp.get_signal_graph( uri = starturi, depth = depth )
  fields=nxp.get_field_graph( uri = starturi, depth = depth )
  links=nxp.get_link_graph( uri = starturi, depth = depth )

  # dictionary to the fancy name and a second smaller line for extra info
  print "---------- PREPROCESS - PYTHON ----------"
  nodecaption = dict()
  nodenote = dict()

  # setup and show in a mathplotlib window
  fig=plt.figure(figsize=(10,10))
  fig.patch.set_facecolor(bgcolor)
  ax=plt.axes([0,0,1,1])
  plt.text(0.02,0.98,"COOLFluiD3 component graph starting from '" + start_key + "'",
    transform=ax.transAxes, size=20, weight='bold',
    color=titlecolor, ha='left', va='top')
  trans=ax.transData.inverted()
  plt.imshow(root_img,aspect='auto',zorder=10)
  #rect=plt.Rectangle([0,0],50,32,facecolor=componentcolor,fill=True,zorder=11)#,lw=0)
  #plt.gca().add_patch(rect)
  plt.gca().add_patch(plt.Rectangle([20,5],12,22,facecolor=bgcolor,lw=0,fill=True,zorder=9))
  plt.axis('off')

  # constructing directional graph, because of using successors function, only directed graphs
  G=nx.DiGraph()

  # executing the submission into the graph
  for line in StringIO(components):
    exec line
  for line in StringIO(options):
    exec line
  for line in StringIO(signals):
    exec line
  for line in StringIO(fields):
    exec line
  for line in StringIO(links):
    exec line

  # separating lists for fancy drawing
  cn=[(u)   for (u,d)   in G.nodes(data=True) if d['tag']=='component']
  ce=[(u,v) for (u,v,d) in G.edges(data=True) if d['tag']=='component']
  ccapt=dict.fromkeys(nodecaption,'')
  cnote=dict.fromkeys(nodenote,'')
  for key in cn :
    ccapt[key]=nodecaption[key]
    cnote[key]=nodenote[key]
  on=[(u)   for (u,d)   in G.nodes(data=True) if d['tag']=='option']
  oe=[(u,v) for (u,v,d) in G.edges(data=True) if d['tag']=='option']
  ocapt=dict.fromkeys(nodecaption,'')
  onote=dict.fromkeys(nodenote,'')
  for key in on :
    ocapt[key]=nodecaption[key]
    onote[key]=nodenote[key]
  sn=[(u)   for (u,d)   in G.nodes(data=True) if d['tag']=='signal']
  se=[(u,v) for (u,v,d) in G.edges(data=True) if d['tag']=='signal']
  scapt=dict.fromkeys(nodecaption,'')
  snote=dict.fromkeys(nodenote,'')
  for key in sn :
    scapt[key]=nodecaption[key]
    snote[key]=nodenote[key]
  fn=[(u)   for (u,d)   in G.nodes(data=True) if d['tag']=='field']
  fe=[(u,v) for (u,v,d) in G.edges(data=True) if d['tag']=='field']
  fcapt=dict.fromkeys(nodecaption,'')
  fnote=dict.fromkeys(nodenote,'')
  for key in fn :
    fcapt[key]=nodecaption[key]
    fnote[key]=nodenote[key]
  ln=[(u)   for (u,d)   in G.nodes(data=True) if d['tag']=='link']
  le=[(u,v) for (u,v,d) in G.edges(data=True) if d['tag']=='link']
  lcapt=dict.fromkeys(nodecaption,'')
  lnote=dict.fromkeys(nodenote,'')
  for key in ln :
    lcapt[key]=nodecaption[key]
    lnote[key]=nodenote[key]

  # computing the positions of the nodes via layouts
  print "---------- LAYOUT ----------"
  #pos=nx.spring_layout(G,iterations=50)
  pos=nx.graphviz_layout(G,prog='twopi')
  #pos=nx.graphviz_layout(G,prog='circo')
  #pos2=nx.graphviz_layout(G,prog='twopi')
  #pos=nx.spring_layout(G,pos=pos2,iterations=500)
  #pos=nx.graphviz_layout(G,prog='circo',root=start_key ,args='mindist=1e8')
  #pos=nx.graphviz_layout(G,prog='twopi',root=start_key ,args='mindist=10')

  # renormalizing positions in the range of zoomfact & root position in zoomfact
  zoomfact=40.
  imx,imy=root_img.size
  rootx,rooty=pos[start_key]
  xmin=min([i[0] for i in pos.values()])
  ymin=min([i[1] for i in pos.values()])
  xmax=max([i[0] for i in pos.values()])
  ymax=max([i[1] for i in pos.values()])
  for i in pos:
    pos[i]=[(pos[i][0]-rootx)/(xmax-xmin)*zoomfact*imx+imx/2,
            (pos[i][1]-rooty)/(ymax-ymin)*zoomfact*imy+imy/2]

  # draw the contents, node_shape: so^>v<dph8
  print "---------- MATPLOTLIB ----------"
  nx.draw_networkx_edges(G,pos,edgelist=ce,edge_color=componentcolor,width=2,arrows=False,style='solid')
  nx.draw_networkx_edges(G,pos,edgelist=oe,edge_color=optioncolor,width=2,arrows=False,style='solid')
  nx.draw_networkx_edges(G,pos,edgelist=se,edge_color=signalcolor,width=2,arrows=False,style='solid')
  nx.draw_networkx_edges(G,pos,edgelist=fe,edge_color=fieldcolor,width=2,arrows=False,style='solid')
  nx.draw_networkx_edges(G,pos,edgelist=le,edge_color=linkcolor,width=2,arrows=False,style='dashed')

  nx.draw_networkx_nodes(G,pos,nodelist=cn,node_color=componentcolor,node_size=50,node_shape='o',linewidths=2)
  nx.draw_networkx_nodes(G,pos,nodelist=on,node_color=optioncolor,node_size=50,node_shape='o',linewidths=2)
  nx.draw_networkx_nodes(G,pos,nodelist=sn,node_color=signalcolor,node_size=50,node_shape='o',linewidths=2)
  nx.draw_networkx_nodes(G,pos,nodelist=fn,node_color=fieldcolor,node_size=50,node_shape='o',linewidths=2)
  nx.draw_networkx_nodes(G,pos,nodelist=ln,node_color=linkcolor,node_size=50,node_shape='o',linewidths=2)

  #nx.draw_networkx_labels(G,pos,labels=ccapt,font_size=11,font_color=componentcolor,font_weight='bold',font_family='sans-serif',verticalalignment='bottom',horizontalalignment='left')
  #nx.draw_networkx_labels(G,pos,labels=cnote,font_size=8,font_color=componentcolor,font_family='sans-serif',verticalalignment='top',horizontalalignment='left')
  #nx.draw_networkx_labels(G,pos,labels=ocapt,font_size=11,font_color=optioncolor,font_weight='bold',font_family='sans-serif',verticalalignment='bottom',horizontalalignment='left')
  #nx.draw_networkx_labels(G,pos,labels=onote,font_size=8,font_color=optioncolor,font_family='sans-serif',verticalalignment='top',horizontalalignment='left')
  #nx.draw_networkx_labels(G,pos,labels=scapt,font_size=11,font_color=signalcolor,font_weight='bold',font_family='sans-serif',verticalalignment='bottom',horizontalalignment='left')
  #nx.draw_networkx_labels(G,pos,labels=snote,font_size=8,font_color=signalcolor,font_family='sans-serif',verticalalignment='top',horizontalalignment='left')
  #nx.draw_networkx_labels(G,pos,labels=fcapt,font_size=11,font_color=fieldcolor,font_weight='bold',font_family='sans-serif',verticalalignment='bottom',horizontalalignment='left')
  #nx.draw_networkx_labels(G,pos,labels=fnote,font_size=8,font_color=fieldcolor,font_family='sans-serif',verticalalignment='top',horizontalalignment='left')
  #nx.draw_networkx_labels(G,pos,labels=lcapt,font_size=11,font_color=linkcolor,font_weight='bold',font_family='sans-serif',verticalalignment='bottom',horizontalalignment='left')
  #nx.draw_networkx_labels(G,pos,labels=lnote,font_size=8,font_color=linkcolor,font_family='sans-serif',verticalalignment='top',horizontalalignment='left')

  # showing the plot
  print "---------- SHOW ----------"
  plt.show()

  # delete the collector component

