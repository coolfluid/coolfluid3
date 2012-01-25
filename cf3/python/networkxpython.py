from cStringIO import StringIO
from copy import deepcopy
import numpy
import Image
import base64
import sys
import math
import matplotlib.pyplot as plt
import networkx as nx
import coolfluid as cf

# color scheme
titlecolor='white'
bgcolor='black'
componentcolor='#aaaaaa'
optioncolor='#2a83bd'
signalcolor='#3a773a'
fieldcolor='#3a3a77'
linkcolor='#aaaaaa'
propertycolor='yellow'

# base64 coded png image
rootpng="""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
OQy9FESRGTLAAAAAElFTkSuQmCC"""

# its just easier to be on top
root = cf.Core.root()
fig=plt.figure(figsize=(10,10),facecolor=bgcolor,edgecolor=bgcolor)
fig.patch.set_facecolor(bgcolor)
ax=plt.axes([0,0,1,1],axisbg=bgcolor)

# normal tree-like layout
def traverse_successors_recursive(G,key,pos,y) :
  for i in G.successors(key):
    if G.node[i]['tag']=='link':
      if not ((G.edge[key][i]['tag']=='link') and (G.node[i]['tag']!='link')) :
        pos[i]=[G.node[i]['depth'],y]
        y+=1
        y=traverse_successors_recursive(G,i,pos,y)
    if G.node[i]['tag']=='option':
      if not ((G.edge[key][i]['tag']=='link') and (G.node[i]['tag']!='link')) :
        pos[i]=[G.node[i]['depth'],y]
        y+=1
        y=traverse_successors_recursive(G,i,pos,y)
  for i in G.successors(key):
    if G.node[i]['tag']=='field':
      if not ((G.edge[key][i]['tag']=='link') and (G.node[i]['tag']!='link')) :
        pos[i]=[G.node[i]['depth'],y]
        y+=1
        y=traverse_successors_recursive(G,i,pos,y)
  for i in G.successors(key):
    if G.node[i]['tag']=='property':
      if not ((G.edge[key][i]['tag']=='link') and (G.node[i]['tag']!='link')) :
        pos[i]=[G.node[i]['depth'],y]
        y+=1
        y=traverse_successors_recursive(G,i,pos,y)
  for i in G.successors(key):
    if G.node[i]['tag']=='signal':
      if not ((G.edge[key][i]['tag']=='link') and (G.node[i]['tag']!='link')) :
        pos[i]=[G.node[i]['depth'],y]
        y+=1
        y=traverse_successors_recursive(G,i,pos,y)
    if G.node[i]['tag']=='component':
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

# in polar coordinates
def traditional_left2right_tree_layout_in_polar(G,key) :
  pos=traditional_left2right_tree_layout(G,key)
  ymax=-1.;
  for i in pos: ymax=max(float(pos[i][1]),float(ymax))
  for i in pos:
    x=float(pos[i][0])
    y=float(pos[i][1])
    r=x
    fi=2*math.pi*y/ymax
    pos[i][0]=r*math.cos(fi)
    pos[i][1]=r*math.sin(fi)
  return pos

# compute angle in degree for each node at which the parent edge arrives
def compute_edge_angles_for_target_nodes(G,pos):
  angle=dict.fromkeys(pos,float(0.))
  for i in G:
    for j in G.successors(i):
      if not ((G.node[j]['tag']!='link') and (G.edge[i][j]['tag']=='link')):
        dx=float(pos[j][0]-pos[i][0])
        dy=float(pos[j][1]-pos[i][1])
        if not (dx==0. and dy==0.):
          angle[j] = -180./math.pi*math.atan2(dy,dx)
  return angle

# need to put things into a class to be reachable by the event callbacks
class nx_event_connector:
  G=nx.DiGraph()
  pos=dict()
  nodecaption=dict()
  edgecaption=dict()
  cn=list()
  ce=list()
  on=list()
  oe=list()
  pn=list()
  pe=list()
  sn=list()
  se=list()
  fn=list()
  fe=list()
  ln=list()
  le=list()
  printdestination='sc'
  infotxt=plt.text(0.02,0.92,"",
    transform=ax.transAxes, family='monospace',size=10, weight='bold',
    color=titlecolor, ha='left', va='top',zorder=51)
  def on_pick(self, event):
      exec "key_orig= self." + event.artist.get_label() + "[" + str(event.ind[0]) + "]"
      key=deepcopy(key_orig)
      if ((self.G.node[key]['tag']=='signal') or (self.G.node[key]['tag']=='option') or (self.G.node[key]['tag']=='field') or (self.G.node[key]['tag']=='property')):
        key=self.G.pred[key].keys()[0]
      comp=root.access_component(key)
      if (self.G.node[key_orig]['tag']=='component'):
        plt.text(self.pos[key_orig][0],self.pos[key_orig][1], key, family='monspace',
          size=10, weight='bold', color=titlecolor, ha='left', va='top',zorder=51)
      if (self.G.node[key_orig]['tag']=='option'):
        plt.text(self.pos[key_orig][0],self.pos[key_orig][1], "option of " + key, family='monspace',
          size=10, weight='bold', color=titlecolor, ha='left', va='top',zorder=51)
      if (self.G.node[key_orig]['tag']=='property'):
        plt.text(self.pos[key_orig][0],self.pos[key_orig][1], "property of " + key, family='monspace',
          size=10, weight='bold', color=titlecolor, ha='left', va='top',zorder=51)
      if (self.G.node[key_orig]['tag']=='signal'):
        plt.text(self.pos[key_orig][0],self.pos[key_orig][1], "signal of " + key, family='monspace',
          size=10, weight='bold', color=titlecolor, ha='left', va='top',zorder=51)
      if (self.G.node[key_orig]['tag']=='field'):
        plt.text(self.pos[key_orig][0],self.pos[key_orig][1], "field of " + key, family='monspace',
          size=10, weight='bold', color=titlecolor, ha='left', va='top',zorder=51)
      if (self.G.node[key_orig]['tag']=='link'):
        plt.text(self.pos[key_orig][0],self.pos[key_orig][1], "link to " + key, family='monspace',
          size=10, weight='bold', color=titlecolor, ha='left', va='top',zorder=51)
      nxp=root.get_child('NetworkXPython')
      infostr=nxp.get_detailed_info(cf.URI(key))
      if 's' in self.printdestination:
        self.infotxt.set_text(infostr)
      else:
        self.infotxt.set_text("")
      if 'c' in self.printdestination:
        print infostr
      plt.draw()

# draw text
def draw_captions(G,pos,capt,label,color,zord,rotation=None):
  for i in capt:
    if capt[i]!="":
      capt[i]= "  " + capt[i] + "  ."
  txt=nx.draw_networkx_labels(G,pos,labels=capt,font_color=color)
  if label is not None:
    for i in txt:
      if txt[i] is not None:
        txt[i].set_family('sans-serif')
        txt[i].set_size(11)
        txt[i].set_weight('bold')
        txt[i].set_zorder(zord)
        txt[i].set_label(label)
        txt[i].set_rotation_mode('anchor')
        if rotation is not None:
          if (abs(rotation[i])<=90):
            txt[i].set_verticalalignment('center')
            txt[i].set_horizontalalignment('left')
            txt[i].set_rotation(rotation[i])
          else:
            txt[i].set_verticalalignment('center')
            txt[i].set_horizontalalignment('right')
            txt[i].set_rotation(rotation[i]+180.)

# draw edges and nodes
def draw_edges_nodes(G,pos,elist,elabel,nlist,nlabel,color,zord):
    edge=nx.draw_networkx_edges(G,pos,edgelist=elist,edge_color=color,arrows=False)
    node=nx.draw_networkx_nodes(G,pos,nodelist=nlist,node_color=color,node_shape='o',node_size=50)
    if edge is not None:
      edge.set_label(elabel)
      edge.set_zorder(zord)
      edge.set_linewidths(1.5)
      edge.set_linestyle('solid')
    if node is not None:
      node.set_picker(5)
      node.set_edgecolor(bgcolor)
      node.set_label(nlabel)
      node.set_zorder(zord+10)
      node.set_linewidths(1.5)

# starturi: from where the recursive listing starts
# depth: how many levels to list down deep in starturi's subtree
# tree: what to plot in the tree. Combination of chars 'cosfl'
# caption: what to label in the tree. Combination of chars 'cosfl'
#  'c': component
#  'o': option
#  'p': property
#  's': signal
#  'f': field
#  'l': link
# printdestination: 'sc' if user clicks on a node, the component information is printed
#  's': to screen (matplotlib window)
#  'c': console (terminal)
#
# zorder numbers:
#   1x:  edges
#   2x:  nodes, always edges +10
#   3x:  captions
#   40:  cf logo middle backdrop rectangle
#   41:  cf logo image
#   50:  title
#   51:  component info
def show_graph(starturi,depth=1000,tree='copfl',caption='',printdestination='c'):

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
  properties=nxp.get_property_graph( uri = starturi, depth = depth )
  #print components
  #print options
  #print signals
  #print fields
  #print links
  #print properties

  # dictionary to the fancy name and a second smaller line for extra info
  print "---------- PREPROCESS - PYTHON ----------"

  # setup title and root node logo
  plt.text(0.02,0.98,"COOLFluiD3 component graph starting from '" + start_key + "'",
    transform=ax.transAxes, size=16, weight='bold',
    color=titlecolor, ha='left', va='top',zorder=50)
  plt.imshow(root_img,aspect='auto',zorder=41)
  plt.gca().add_patch(plt.Rectangle([20,5],12,22,facecolor=bgcolor,lw=0,fill=True,zorder=40))
  plt.axis('off')

  # constructing directional graph, because of using successors function, only directed graphs
  nec=nx_event_connector()
  nec.printdestination=printdestination
  cid=fig.canvas.mpl_connect('pick_event', nec.on_pick)

  # executing the submission into the graph
  if 'c' in tree:
    for line in StringIO(components): exec 'nec.' + line
  if 'o' in tree:
    for line in StringIO(options   ): exec 'nec.' + line
  if 'p' in tree:
    for line in StringIO(properties): exec 'nec.' + line
  if 's' in tree:
    for line in StringIO(signals   ): exec 'nec.' + line
  if 'f' in tree:
    for line in StringIO(fields    ): exec 'nec.' + line
  if 'l' in tree:
    for line in StringIO(links     ): exec 'nec.' + line

  # separating lists for fancy drawing
  nec.cn=[(u)   for (u,d)   in nec.G.nodes(data=True) if d['tag']=='component']
  nec.ce=[(u,v) for (u,v,d) in nec.G.edges(data=True) if d['tag']=='component']
  ccapt=dict.fromkeys(nec.nodecaption,'')
  for key in nec.cn : ccapt[key]=nec.nodecaption[key]
  nec.on=[(u)   for (u,d)   in nec.G.nodes(data=True) if d['tag']=='option']
  nec.oe=[(u,v) for (u,v,d) in nec.G.edges(data=True) if d['tag']=='option']
  ocapt=dict.fromkeys(nec.nodecaption,'')
  for key in nec.on : ocapt[key]=nec.nodecaption[key]
  nec.pn=[(u)   for (u,d)   in nec.G.nodes(data=True) if d['tag']=='property']
  nec.pe=[(u,v) for (u,v,d) in nec.G.edges(data=True) if d['tag']=='property']
  pcapt=dict.fromkeys(nec.nodecaption,'')
  for key in nec.pn : pcapt[key]=nec.nodecaption[key]
  nec.sn=[(u)   for (u,d)   in nec.G.nodes(data=True) if d['tag']=='signal']
  nec.se=[(u,v) for (u,v,d) in nec.G.edges(data=True) if d['tag']=='signal']
  scapt=dict.fromkeys(nec.nodecaption,'')
  for key in nec.sn : scapt[key]=nec.nodecaption[key]
  nec.fn=[(u)   for (u,d)   in nec.G.nodes(data=True) if d['tag']=='field']
  nec.fe=[(u,v) for (u,v,d) in nec.G.edges(data=True) if d['tag']=='field']
  fcapt=dict.fromkeys(nec.nodecaption,'')
  for key in nec.fn : fcapt[key]=nec.nodecaption[key]
  nec.ln=[(u)   for (u,d)   in nec.G.nodes(data=True) if d['tag']=='link']
  nec.le=[(u,v) for (u,v,d) in nec.G.edges(data=True) if d['tag']=='link']
  lcapt=dict.fromkeys(nec.nodecaption,'')
  for key in nec.ln : lcapt[key]=nec.nodecaption[key]

  # computing the positions of the nodes via layouts
  print "---------- LAYOUT ----------"
  #nec.pos=nx.spring_layout(nec.G,iterations=50)
  #nec.pos=nx.graphviz_layout(nec.G,prog='twopi')
  #nec.pos=nx.graphviz_layout(nec.G,prog='fdp',root=start_key)
  #nec.pos=nx.graphviz_layout(nec.G,prog='circo',root=start_key)
  #nec.pos=nx.graphviz_layout(nec.G,prog='circo',root=start_key,mindist=10,aspect=2)
  #   pos2=nx.graphviz_layout(nec.G,prog='circo',root=start_key)
  #nec.pos=nx.spring_layout(nec.G,nec.pos=pos2,iterations=500)
  #nec.pos=nx.graphviz_layout(nec.G,prog='circo',root=start_key ,args='mindist=1e8')
  #nec.pos=nx.graphviz_layout(nec.G,prog='twopi',root=start_key ,args='mindist=10')
  #   pos2=nx.graphviz_layout(nec.G,prog='fdp')
  #nec.pos=nx.spring_layout(nec.G,pos=pos2,iterations=50)
  #nec.pos=traditional_left2right_tree_layout(nec.G,start_key)
  nec.pos=traditional_left2right_tree_layout_in_polar(nec.G,start_key)
  #pos2=traditional_left2right_tree_layout_in_polar(nec.G,start_key)
  #nec.pos=nx.spring_layout(nec.G,pos=pos2,iterations=50)

  # renormalizing positions in the range of zoomfact & root position in zoomfact
  # also computing angles, just in case
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
  rot=compute_edge_angles_for_target_nodes(nec.G,nec.pos)

  # print statistics
  print "Statistics for: ", start_key
  print "Number of        nodes  &  edges"
  print "--------------------------------"
  print "Components: %10d%10d" % ( len(nec.cn), len(nec.ce) )
  print "Options:    %10d%10d" % ( len(nec.on), len(nec.oe) )
  print "Properties: %10d%10d" % ( len(nec.pn), len(nec.pe) )
  print "Signals:    %10d%10d" % ( len(nec.sn), len(nec.se) )
  print "Fields:     %10d%10d" % ( len(nec.fn), len(nec.fe) )
  print "Links:      %10d%10d" % ( len(nec.ln), len(nec.le) )
  print "TOTAL:      %10d%10d" % ( len(nec.G.nodes()), len(nec.G.edges()) )

  # draw the contents, node_shape: so^>v<dph8
  # the order the commands are executed is important, todo: move out zorder via .set_zorder and will become proper
  print "---------- MATPLOTLIB ----------"
  if 's' in tree: draw_edges_nodes(nec.G,nec.pos,nec.se,'se',nec.sn,'sn',signalcolor,10)
  if 'p' in tree: draw_edges_nodes(nec.G,nec.pos,nec.pe,'pe',nec.pn,'pn',propertycolor,11)
  if 'f' in tree: draw_edges_nodes(nec.G,nec.pos,nec.fe,'fe',nec.fn,'fn',fieldcolor,12)
  if 'o' in tree: draw_edges_nodes(nec.G,nec.pos,nec.oe,'oe',nec.on,'on',optioncolor,13)
  if 'l' in tree: draw_edges_nodes(nec.G,nec.pos,nec.le,'le',nec.ln,'ln',linkcolor,14)
  if 'c' in tree: draw_edges_nodes(nec.G,nec.pos,nec.ce,'ce',nec.cn,'cn',componentcolor,15)

  if 's' in caption: draw_captions(nec.G,nec.pos,scapt,'sc',signalcolor,40,rotation=rot)
  if 'p' in caption: draw_captions(nec.G,nec.pos,pcapt,'pc',propertycolor,41,rotation=rot)
  if 'f' in caption: draw_captions(nec.G,nec.pos,fcapt,'fc',fieldcolor,42,rotation=rot)
  if 'o' in caption: draw_captions(nec.G,nec.pos,ocapt,'oc',optioncolor,43,rotation=rot)
  if 'l' in caption: draw_captions(nec.G,nec.pos,lcapt,'lc',linkcolor,44,rotation=rot)
  if 'c' in caption: draw_captions(nec.G,nec.pos,ccapt,'cc',componentcolor,45,rotation=rot)

  # showing the plot
  print "---------- SHOW ----------"
  plt.show()

  # delete the collector component

