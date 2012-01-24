from cStringIO import StringIO
from copy import deepcopy
import numpy
import Image
import base64
import sys
import matplotlib.pyplot as plt
import networkx as nx
import coolfluid as cf

# its just easier to be on top
root = cf.Core.root()

# color scheme
titlecolor='white'
bgcolor='black'
componentcolor='#aaaaaa'
optioncolor='#2a83bd'
signalcolor='#3a773a'
fieldcolor='#3a3a77'
linkcolor='#aaaaaa'

rootpng="""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
OQy9FESRGTLAAAAAElFTkSuQmCC"""

# normal tree-like layout
def traverse_successors_recursive(G,key,pos,y) :
  for i in G.successors(key):
    if not ((G.edge[key][i]['tag']=='link') and (G.node[i]['tag']!='link')) :
      pos[i]=[G.node[i]['depth'],y]
      y+=1
      y=traverse_successors_recursive(G,i,pos,y)
  return y
def traditional_left2right_tree_layout(G,key) :
  pos=dict(zip(G,numpy.tile(-1,(G.number_of_nodes(),2))))
  pos[key]=[G.node[key]['depth'],0]
  y=traverse_successors_recursive(G,key,pos,1)
  return pos

# need to put things into a class to be reachable by the event callbacks
class nx_event_connector:
  G=nx.DiGraph()
  pos=dict()
  cn=list()
  ce=list()
  on=list()
  oe=list()
  sn=list()
  se=list()
  fn=list()
  fe=list()
  ln=list()
  le=list()
  def on_pick(self, event):
      exec "key_orig= self." + event.artist.get_label() + "[" + str(event.ind[0]) + "]"
      key=deepcopy(key_orig)
      if ((self.G.node[key]['tag']=='signal') or (self.G.node[key]['tag']=='option') or (self.G.node[key]['tag']=='field')):
        key=self.G.pred[key].keys()[0]
      comp=root.access_component(key)
      comp.print_info()
      plt.text(self.pos[key_orig][0],self.pos[key_orig][1],"INFO   " + key,
        size=12, weight='normal', color=titlecolor, ha='left', va='top',zorder=51)
      plt.draw()

# starturi: from where the recursive listing starts
# depth: how many levels to list down deep in starturi's subtree
# tree: what to plot in the tree. Combination of chars 'cosfl'
# caption: what to label in the tree. Combination of chars 'cosfl'
#  'c': component
#  'o': option
#  's': signal
#  'f': field
#  'l': link
#
# zorder numbers:
#   14:  component edges
#   12:  option edges
#   10:  signal edges
#   11:  field edges
#   13:  link edges
#   24:  component nodes
#   22:  option nodes
#   20:  signal nodes
#   21:  field nodes
#   23:  link nodes
#   30:  cf logo middle backdrop rectangle
#   31:  cf logo image
#   44:  component captions
#   42:  option captions
#   40:  signal captions
#   41:  field captions
#   43:  link captions
#   50:  title
#   51:  component info
def show_graph(starturi,depth=1000,tree='cofl',caption=''):

  # decode and load root component's image
  rootpng_bin=StringIO(base64.decodestring(rootpng))
  root_img=Image.open(rootpng_bin)
  print root_img

  # create the collector component
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
  #print components
  #print options
  #print signals
  #print fields
  #print links

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
    color=titlecolor, ha='left', va='top',zorder=50)
  plt.imshow(root_img,aspect='auto',zorder=31)
  plt.gca().add_patch(plt.Rectangle([20,5],12,22,facecolor=bgcolor,lw=0,fill=True,zorder=30))
  plt.axis('off')

  # constructing directional graph, because of using successors function, only directed graphs
  nec=nx_event_connector()
  cid=fig.canvas.mpl_connect('pick_event', nec.on_pick)

  # executing the submission into the graph
  if 'c' in tree:
    for line in StringIO(components):
      exec line
  if 'o' in tree:
    for line in StringIO(options):
      exec line
  if 's' in tree:
    for line in StringIO(signals):
      exec line
  if 'f' in tree:
    for line in StringIO(fields):
      exec line
  if 'l' in tree:
    for line in StringIO(links):
      exec line

  # separating lists for fancy drawing
  nec.cn=[(u)   for (u,d)   in nec.G.nodes(data=True) if d['tag']=='component']
  nec.ce=[(u,v) for (u,v,d) in nec.G.edges(data=True) if d['tag']=='component']
  ccapt=dict.fromkeys(nodecaption,'')
  cnote=dict.fromkeys(nodenote,'')
  for key in nec.cn :
    ccapt[key]=nodecaption[key]
    cnote[key]=nodenote[key]
  nec.on=[(u)   for (u,d)   in nec.G.nodes(data=True) if d['tag']=='option']
  nec.oe=[(u,v) for (u,v,d) in nec.G.edges(data=True) if d['tag']=='option']
  ocapt=dict.fromkeys(nodecaption,'')
  onote=dict.fromkeys(nodenote,'')
  for key in nec.on :
    ocapt[key]=nodecaption[key]
    onote[key]=nodenote[key]
  nec.sn=[(u)   for (u,d)   in nec.G.nodes(data=True) if d['tag']=='signal']
  nec.se=[(u,v) for (u,v,d) in nec.G.edges(data=True) if d['tag']=='signal']
  scapt=dict.fromkeys(nodecaption,'')
  snote=dict.fromkeys(nodenote,'')
  for key in nec.sn :
    scapt[key]=nodecaption[key]
    snote[key]=nodenote[key]
  nec.fn=[(u)   for (u,d)   in nec.G.nodes(data=True) if d['tag']=='field']
  nec.fe=[(u,v) for (u,v,d) in nec.G.edges(data=True) if d['tag']=='field']
  fcapt=dict.fromkeys(nodecaption,'')
  fnote=dict.fromkeys(nodenote,'')
  for key in nec.fn :
    fcapt[key]=nodecaption[key]
    fnote[key]=nodenote[key]
  nec.ln=[(u)   for (u,d)   in nec.G.nodes(data=True) if d['tag']=='link']
  nec.le=[(u,v) for (u,v,d) in nec.G.edges(data=True) if d['tag']=='link']
  lcapt=dict.fromkeys(nodecaption,'')
  lnote=dict.fromkeys(nodenote,'')
  for key in nec.ln :
    lcapt[key]=nodecaption[key]
    lnote[key]=nodenote[key]

  # computing the positions of the nodes via layouts
  print "---------- LAYOUT ----------"
  #nec.pos=nx.spring_layout(nec.G,iterations=50)
  #nec.pos=nx.graphviz_layout(nec.G,prog='twopi')
  #nec.pos=nx.graphviz_layout(nec.G,prog='fdp',root=start_key)
  #nec.pos=nx.graphviz_layout(nec.G,prog='circo',root=start_key ,args='mindist=2')
  #nec.pos=nx.graphviz_layout(nec.G,prog='circo',root=start_key,mindist=10,aspect=2)
  #   pos2=nx.graphviz_layout(nec.G,prog='circo',root=start_key)
  #nec.pos=nx.spring_layout(nec.G,nec.pos=pos2,iterations=500)
  #nec.pos=nx.graphviz_layout(nec.G,prog='circo',root=start_key ,args='mindist=1e8')
  #nec.pos=nx.graphviz_layout(nec.G,prog='twopi',root=start_key ,args='mindist=10')
  #   pos2=nx.graphviz_layout(nec.G,prog='fdp')
  #nec.pos=nx.spring_layout(nec.G,nec.pos=pos2,iterations=50)
  nec.pos=traditional_left2right_tree_layout(nec.G,start_key)

  # renormalizing positions in the range of zoomfact & root position in zoomfact
  zoomfact=float(40.)
  imx=float(root_img.size[0])
  imy=float(root_img.size[1])
  rootx,rooty=nec.pos[start_key]
  xmin=float(min([i[0] for i in nec.pos.values()]))
  ymin=float(min([i[1] for i in nec.pos.values()]))
  xmax=float(max([i[0] for i in nec.pos.values()]))
  ymax=float(max([i[1] for i in nec.pos.values()]))
  if xmin==xmax:
    xmin-=0.5
    xmax+=0.5
  if ymin==ymax:
    ymin-=0.5
    ymax+=0.5
  for i in nec.pos:
    nec.pos[i]=[(nec.pos[i][0]-rootx)/(xmax-xmin)*zoomfact*imx+imx/2,
            (nec.pos[i][1]-rooty)/(ymax-ymin)*zoomfact*imy+imy/2]

  # draw the contents, node_shape: so^>v<dph8
  # the order the commands are executed is important, todo: move out zorder via .set_zorder and will become proper
  print "---------- MATPLOTLIB ----------"
  if 's' in tree:
    nx.draw_networkx_edges(nec.G,nec.pos,edgelist=nec.se,edge_color=signalcolor,width=2,arrows=False,style='solid',zorder=10)
    ssnn=nx.draw_networkx_nodes(nec.G,nec.pos,nodelist=nec.sn,node_color=signalcolor,node_size=50,node_shape='o',linewidths=2,zorder=20)
    if ssnn is not None:
      ssnn.set_picker(5)
      ssnn.set_edgecolor(bgcolor)
      ssnn.set_label('sn')
  if 'f' in tree:
    nx.draw_networkx_edges(nec.G,nec.pos,edgelist=nec.fe,edge_color=fieldcolor,width=2,arrows=False,style='solid',zorder=11)
    ffnn=nx.draw_networkx_nodes(nec.G,nec.pos,nodelist=nec.fn,node_color=fieldcolor,node_size=50,node_shape='o',linewidths=2,zorder=21)
    if ffnn is not None:
      ffnn.set_picker(5)
      ffnn.set_edgecolor(bgcolor)
      ffnn.set_label('fn')
  if 'o' in tree:
    nx.draw_networkx_edges(nec.G,nec.pos,edgelist=nec.oe,edge_color=optioncolor,width=2,arrows=False,style='solid',zorder=12)
    oonn=nx.draw_networkx_nodes(nec.G,nec.pos,nodelist=nec.on,node_color=optioncolor,node_size=50,node_shape='o',linewidths=2,zorder=22)
    if oonn is not None:
      oonn.set_picker(5)
      oonn.set_edgecolor(bgcolor)
      oonn.set_label('on')
  if 'l' in tree:
    nx.draw_networkx_edges(nec.G,nec.pos,edgelist=nec.le,edge_color=linkcolor,width=2,arrows=False,style='dashed',zorder=13)
    llnn=nx.draw_networkx_nodes(nec.G,nec.pos,nodelist=nec.ln,node_color=linkcolor,node_size=50,node_shape='o',linewidths=2,zorder=23)
    if llnn is not None:
      llnn.set_picker(5)
      llnn.set_edgecolor(bgcolor)
      llnn.set_label('ln')
  if 'c' in tree:
    nx.draw_networkx_edges(nec.G,nec.pos,edgelist=nec.ce,edge_color=componentcolor,width=2,arrows=False,style='solid',zorder=14)
    ccnn=nx.draw_networkx_nodes(nec.G,nec.pos,nodelist=nec.cn,node_color=componentcolor,node_size=50,node_shape='o',linewidths=2,zorder=24)
    if ccnn is not None:
      ccnn.set_picker(5)
      ccnn.set_edgecolor(bgcolor)
      ccnn.set_label('cn')

  if 's' in caption:
    nx.draw_networkx_labels(nec.G,nec.pos,labels=scapt,font_size=11,font_color=signalcolor,font_weight='bold',font_family='sans-serif',verticalalignment='center',horizontalalignment='left',zorder=40)
    #nx.draw_networkx_labels(nec.G,nec.pos,labels=scapt,font_size=11,font_color=signalcolor,font_weight='bold',font_family='sans-serif',verticalalignment='bottom',horizontalalignment='left',zorder=40)
    #nx.draw_networkx_labels(nec.G,nec.pos,labels=snote,font_size=8,font_color=signalcolor,font_family='sans-serif',verticalalignment='top',horizontalalignment='left',zorder=40)
  if 'f' in caption:
    nx.draw_networkx_labels(nec.G,nec.pos,labels=fcapt,font_size=11,font_color=fieldcolor,font_weight='bold',font_family='sans-serif',verticalalignment='center',horizontalalignment='left',zorder=41)
    #nx.draw_networkx_labels(nec.G,nec.pos,labels=fcapt,font_size=11,font_color=fieldcolor,font_weight='bold',font_family='sans-serif',verticalalignment='bottom',horizontalalignment='left',zorder=41)
    #nx.draw_networkx_labels(nec.G,nec.pos,labels=fnote,font_size=8,font_color=fieldcolor,font_family='sans-serif',verticalalignment='top',horizontalalignment='left',zorder=41)
  if 'o' in caption:
    nx.draw_networkx_labels(nec.G,nec.pos,labels=ocapt,font_size=11,font_color=optioncolor,font_weight='bold',font_family='sans-serif',verticalalignment='center',horizontalalignment='left',zorder=42)
    #nx.draw_networkx_labels(nec.G,nec.pos,labels=ocapt,font_size=11,font_color=optioncolor,font_weight='bold',font_family='sans-serif',verticalalignment='bottom',horizontalalignment='left',zorder=42)
    #nx.draw_networkx_labels(nec.G,nec.pos,labels=onote,font_size=8,font_color=optioncolor,font_family='sans-serif',verticalalignment='top',horizontalalignment='left',zorder=42)
  if 'l' in caption:
    nx.draw_networkx_labels(nec.G,nec.pos,labels=lcapt,font_size=11,font_color=linkcolor,font_weight='bold',font_family='sans-serif',verticalalignment='center',horizontalalignment='left',zorder=43)
    #nx.draw_networkx_labels(nec.G,nec.pos,labels=lcapt,font_size=11,font_color=linkcolor,font_weight='bold',font_family='sans-serif',verticalalignment='bottom',horizontalalignment='left',zorder=43)
    #nx.draw_networkx_labels(nec.G,nec.pos,labels=lnote,font_size=8,font_color=linkcolor,font_family='sans-serif',verticalalignment='top',horizontalalignment='left',zorder=43)
  if 'c' in caption:
    nx.draw_networkx_labels(nec.G,nec.pos,labels=ccapt,font_size=11,font_color=componentcolor,font_weight='bold',font_family='sans-serif',verticalalignment='center',horizontalalignment='left',zorder=44)
    #nx.draw_networkx_labels(nec.G,nec.pos,labels=ccapt,font_size=11,font_color=componentcolor,font_weight='bold',font_family='sans-serif',verticalalignment='bottom',horizontalalignment='left',zorder=44)
    #nx.draw_networkx_labels(nec.G,nec.pos,labels=cnote,font_size=8,font_color=componentcolor,font_family='sans-serif',verticalalignment='top',horizontalalignment='left',zorder=44)

  # showing the plot
  print "---------- SHOW ----------"
  plt.show()

  # delete the collector component

