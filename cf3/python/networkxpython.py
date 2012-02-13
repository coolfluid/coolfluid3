# Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
#
# This software is distributed under the terms of the
# GNU Lesser General Public License version 3 (LGPLv3).
# See doc/lgpl.txt and doc/gpl.txt for the license text.

#########################################################################################
# import needed stuff
#########################################################################################
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

#########################################################################################
# color settings
#########################################################################################
titlecolor='white'
bgcolor='black'
navicolor='yellow'
componentcolor='#aaaaaa'
optioncolor='#2a83bd'
fieldcolor='#3a773a'
signalcolor='#aa6d00'
tagcolor='#5a5ab7'
linkcolor='#aaaaaa'
propertycolor='#cc4444'

#########################################################################################
# base64 coded png image for root component
#########################################################################################
cf3_b64="""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
OQy9FESRGTLAAAAAElFTkSuQmCC"""

#########################################################################################
# setting up the matplotlib layout
#########################################################################################

# coolfluid head component
root = cf.Core.root()

#figures and axes
fig=plt.figure(figsize=(10,10))
mainax=plt.axes([0,0,1,1],axisbg=bgcolor,label='main')
naviax=plt.axes([0.65,0.01,0.34,0.34],axisbg=bgcolor,label='navi')
plt.sca(mainax)

# turning off ticks, labels and frame both for the main plot and the little navigation plot
mainax.xaxis.set_visible(False)
mainax.yaxis.set_visible(False)
for i in mainax.spines: mainax.spines[i].set_visible(False)
naviax.xaxis.set_visible(False)
naviax.yaxis.set_visible(False)
for i in naviax.spines: naviax.spines[i].set_color(titlecolor)
naviax.set_visible(False)

# setting up the text object of the title and decode+load the cf logo image
titletxt=plt.text(0.02,0.98,'',transform=mainax.transAxes, size=16, weight='bold',color=titlecolor, ha='left', va='top',zorder=50)
cf3_png=StringIO(base64.decodestring(cf3_b64))
cf3_img=Image.open(cf3_png)
plt.imshow(cf3_img,aspect='auto',zorder=41)
plt.gca().add_patch(plt.Rectangle([20,5],12,22,facecolor=bgcolor,lw=0,fill=True,zorder=40))


#########################################################################################
# custom-made layouts
#   traditional_left2right_tree_layout(G,key,pos)
#   traditional_left2right_tree_layout_in_polar(G,key)
#########################################################################################
def traverse_successors_recursive(G,key,pos,y) :
  for i in G.successors(key):
    if G.node[i]['tag']=='link':
      if not ((G.edge[key][i]['tag']=='link') and (G.node[i]['tag']!='link')) :
        pos[i]=[G.node[i]['depth'],y]
        y+=1
        y=traverse_successors_recursive(G,i,pos,y)
  for i in G.successors(key):
    if G.node[i]['tag']=='option':
      if not ((G.edge[key][i]['tag']=='link') and (G.node[i]['tag']!='link')) :
        pos[i]=[G.node[i]['depth'],y]
        y+=1
        y=traverse_successors_recursive(G,i,pos,y)
  for i in G.successors(key):
    if G.node[i]['tag']=='tag':
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
  for i in G.successors(key):
    if G.node[i]['tag']=='component':
      if not ((G.edge[key][i]['tag']=='link') and (G.node[i]['tag']!='link')) :
        pos[i]=[G.node[i]['depth'],y]
        y+=1
        y=traverse_successors_recursive(G,i,pos,y)
  return y

def traditional_left2right_tree_layout(G,key) :
  pos=dict(zip(G,numpy.tile(-999,(G.number_of_nodes(),2))))
  pos['c:'+key]=[G.node['c:'+key]['depth'],0]
  y=traverse_successors_recursive(G,'c:'+key,pos,1)
  for i in G.pred['c:'+key]:
    pos[i]=[G.node[i]['depth'],G.node[i]['depth']]
  ctr=0
  for i in pos:
    if pos[i][0]==-999:
      print "traditional_left2right_tree_layout: unplugged key: ", i, G.node[i]
      pos[i][1]=ctr
      ctr+=1
  return pos

def traditional_left2right_tree_layout_in_polar(G,key) :
  pos=traditional_left2right_tree_layout(G,key)
  ymax=-1.
  for i in pos: ymax=max(float(pos[i][1]),float(ymax))
  for i in pos:
    x=float(pos[i][0])
    y=float(pos[i][1])
    r=x
    fi=2*math.pi*y/ymax
    pos[i][0]=r*math.cos(fi)
    pos[i][1]=r*math.sin(fi)
  return pos

#########################################################################################
# compute angle in degree for each node at which the edge from parent node arrives
#########################################################################################
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

#########################################################################################
# renormalizing positions in the range of zoomfact & root position in zoomfact
# it gets normalized in a way that cf3_img's pixels are 1m away frm each other
# and size is 1/zoomfact compared t axes dimensions
# also computing angles, based on the angle of the edge leading to parent node
#########################################################################################
def normalize_coordinates(key_,pos_,img_):
  zoomfact=40.
  imx=float(img_.size[0])
  imy=float(img_.size[1])
  rootx,rooty=pos_['c:'+key_]
  xmin=float(min([i[0] for i in pos_.values()]))
  ymin=float(min([i[1] for i in pos_.values()]))
  xmax=float(max([i[0] for i in pos_.values()]))
  ymax=float(max([i[1] for i in pos_.values()]))
  if xmin==xmax:
    xmin-=0.5
    xmax+=0.5
  if ymin==ymax:
    ymin-=0.5
    ymax+=0.5
  for i in pos_:
    pos_[i]=[(pos_[i][0]-rootx)/(xmax-xmin)*zoomfact*imx+imx/2,
             (pos_[i][1]-rooty)/(ymax-ymin)*zoomfact*imy+imy/2]
  return pos_;

#########################################################################################
# need to put things into a class to be reachable by the event callbacks
#########################################################################################
class nx_event_connector:

  # main members
  #######################################################################################
  name=None
  G=None
  pos=None
  nodecaption=None
  cn=None
  ce=None
  ccapt=None
  on=None
  oe=None
  ocapt=None
  pn=None
  pe=None
  pcapt=None
  sn=None
  se=None
  scapt=None
  fn=None
  fe=None
  fcapt=None
  ln=None
  le=None
  lcapt=None
  tn=None
  te=None
  tcapt=None
  printdestination=None
  hidden=None
  navi=None

  # constructor
  #######################################################################################
  def __init__(self,name_):
    self.name=name_                      # name for identifying between main and navi
    self.G=nx.DiGraph()                  # the graph itself
    self.pos=dict()                      # positions of the nodes
    #self.navi=nx_event_connector("navi") # secondary graph for navigating, only initialized on pick
    self.nodecaption=dict()              # node labels to show
    self.cn=list()                       # list of nodes to display for components
    self.ce=list()                       # list of edges to display for components
    self.ccapt=dict(  )                  # text to display for nodes of components
    self.on=list()                       # list of nodes to display for options
    self.oe=list()                       # list of edges to display for options
    self.ocapt=dict()                    # text to display for nodes of options
    self.pn=list()                       # list of nodes to display for properties
    self.pe=list()                       # list of edges to display for properties
    self.pcapt=dict()                    # text to display for nodes of properties
    self.sn=list()                       # list of nodes to display for signal
    self.se=list()                       # list of edges to display for signal
    self.scapt=dict()                    # text to display for nodes of signal
    self.fn=list()                       # list of nodes to display for field
    self.fe=list()                       # list of edges to display for field
    self.fcapt=dict()                    # text to display for nodes of field
    self.ln=list()                       # list of nodes to display for links
    self.le=list()                       # list of edges to display for links
    self.lcapt=dict()                    # text to display for nodes of links
    self.tn=list()                       # list of nodes to display for tags
    self.te=list()                       # list of edges to display for tags
    self.tcapt=dict()                    # text to display for nodes of tags
    self.printdestination='sc'           # where to print component info: 's' screen and/or 'c' console terminal
    self.hidden='cfospl'                 # mask for which nodes marked as hidden not to display

  # helper members
  #######################################################################################
  infotxt=plt.text(0.02,0.92,"",
    transform=mainax.transAxes, family='monospace',size=10, weight='bold',
    color=navicolor, ha='left', va='top',zorder=51)
  seltxt=plt.text(0.,0.,"",
    family='monospace',size=10, weight='bold',
    color=navicolor, ha='left', va='center',zorder=51)

  # helper func for plotting the secondary navigator
  #######################################################################################
  def plot_secondary_navigation(self,key):
    plt.sca(naviax)
    nxp=root.get_child('NetworkXPython')
    naviax.clear()
    self.navi=nx_event_connector("navi")
    self.navi=build_graph_with_lists(cf.URI(key[2:]),self.navi,nxp,1,'cspoflt',depthlimit_sofplt=False,add_parent=True,hidden=self.hidden)
    self.navi.pos=traditional_left2right_tree_layout_in_polar(self.navi.G,key[2:])
    self.navi.pos=normalize_coordinates(key[2:],self.navi.pos,cf3_img)
    self.navi.rot=compute_edge_angles_for_target_nodes(self.navi.G,self.navi.pos)
    for i in self.navi.G:
      if self.navi.G.node[i]['depth']==-1:
        self.navi.pos[i]=(-1.3,1.3)
      else:
        if self.navi.G.node[i]['depth']==0:
          self.navi.pos[i]=(0.,0.)
        else:
          coord=self.navi.pos[i]
          magnitude=math.sqrt(coord[0]*coord[0]+coord[1]*coord[1])
          if magnitude!=0:
            coord[0]/=magnitude
            coord[1]/=magnitude
            self.navi.pos[i]=coord
    draw_edges_nodes(self.navi.G,self.navi.pos,self.navi.se,'se',self.navi.sn,'sn','solid',signalcolor,10)
    draw_edges_nodes(self.navi.G,self.navi.pos,self.navi.pe,'pe',self.navi.pn,'pn','solid',propertycolor,11)
    draw_edges_nodes(self.navi.G,self.navi.pos,self.navi.fe,'fe',self.navi.fn,'fn','solid',fieldcolor,12)
    draw_edges_nodes(self.navi.G,self.navi.pos,self.navi.te,'te',self.navi.tn,'tn','solid',tagcolor,13)
    draw_edges_nodes(self.navi.G,self.navi.pos,self.navi.oe,'oe',self.navi.on,'on','solid',optioncolor,14)
    draw_edges_nodes(self.navi.G,self.navi.pos,self.navi.le,'le',self.navi.ln,'ln','dashed',linkcolor,15)
    draw_edges_nodes(self.navi.G,self.navi.pos,self.navi.ce,'ce',self.navi.cn,'cn','solid',componentcolor,16)
    draw_captions(self.navi.G,self.navi.pos,self.navi.scapt,'sc',signalcolor,40,rotation=self.navi.rot,onlyalign=True)
    draw_captions(self.navi.G,self.navi.pos,self.navi.pcapt,'pc',propertycolor,41,rotation=self.navi.rot,onlyalign=True)
    draw_captions(self.navi.G,self.navi.pos,self.navi.fcapt,'fc',fieldcolor,42,rotation=self.navi.rot,onlyalign=True)
    draw_captions(self.navi.G,self.navi.pos,self.navi.tcapt,'tc',tagcolor,43,rotation=self.navi.rot,onlyalign=True)
    draw_captions(self.navi.G,self.navi.pos,self.navi.ocapt,'oc',optioncolor,44,rotation=self.navi.rot,onlyalign=True)
    draw_captions(self.navi.G,self.navi.pos,self.navi.lcapt,'lc',linkcolor,45,rotation=self.navi.rot,onlyalign=True)
    draw_captions(self.navi.G,self.navi.pos,self.navi.ccapt,'cc',componentcolor,46,rotation=self.navi.rot,onlyalign=True)
    naviax.set_xbound((-3,3))
    naviax.set_ybound((-1.2,1.5))
    naviax.set_visible(True)
    plt.sca(mainax)

  # event on picking a node
  #######################################################################################
  def on_pick(self, event):

      # get the key from the label of the artist node_info=(main|navi,label_passed_to_draw_functions)
      exec event.artist.get_label() # this gives node_info
      if node_info[0]=="main": # this gives the key was picked
        exec "key_orig=self." + node_info[1] + "[" + str(event.ind[0]) + "]"
        tG=self.G
      else:
        exec "key_orig=self.navi." + node_info[1] + "[" + str(event.ind[0]) + "]"
        tG=self.navi.G
      key=deepcopy(key_orig)

      # marionetting out the owner if its not a component, first trying main and then navi
      if ((tG.node[key]['tag']=='tag') or (tG.node[key]['tag']=='signal') or (tG.node[key]['tag']=='option') or (tG.node[key]['tag']=='field') or (tG.node[key]['tag']=='property')):
        key=tG.pred[key].keys()[0]
      comp=root.access_component(key[2:])

      if node_info[0]=="navi":
        key_orig=key

      # set string to display
      self.seltxt.set_text("")
      self.seltxt.set_position(self.pos[key_orig])
      if (tG.node[key_orig]['tag']=='component'):
        self.seltxt.set_text(key[2:])
      if (tG.node[key_orig]['tag']=='option'):
        self.seltxt.set_text("option of " + key[2:])
      if (tG.node[key_orig]['tag']=='tag'):
        self.seltxt.set_text("tag of " + key[2:])
      if (tG.node[key_orig]['tag']=='property'):
        self.seltxt.set_text("property of " + key[2:])
      if (tG.node[key_orig]['tag']=='signal'):
        self.seltxt.set_text("signal of " + key[2:])
      if (tG.node[key_orig]['tag']=='field'):
        self.seltxt.set_text("field of " + key[2:])
      if (tG.node[key_orig]['tag']=='link'):
        self.seltxt.set_text("link to " + key[2:])

      # and finally get and display detailed info of the actual component
      nxp=root.get_child('NetworkXPython')
      infostr=nxp.get_detailed_info(cf.URI(key[2:]))
      self.infotxt.set_text("")
      if 's' in self.printdestination: self.infotxt.set_text(infostr)
      if 'c' in self.printdestination: print infostr
      self.plot_secondary_navigation(key)

      plt.draw()

#########################################################################################
# draw text
#########################################################################################
def draw_captions(G,pos,capt,label,color,zord,rotation=None,onlyalign=False):
  curraxes=fig.gca()
  for i in capt:
    capt[i]= "  " + capt[i] + "  "
  txt=nx.draw_networkx_labels(G,pos,labels=capt,font_color=color)
  if txt is not None:
    for i in txt:
      if txt[i] is not None:
        txt[i].set_family('sans-serif')
        txt[i].set_size(11)
        txt[i].set_weight('bold')
        txt[i].set_zorder(zord)
        txt[i].set_label( "caption_info=('" + curraxes.get_label() + "','" + label + "')" )
        txt[i].set_rotation_mode('anchor')
        if rotation is not None:
          if (abs(rotation[i])<=90):
            txt[i].set_verticalalignment('center')
            txt[i].set_horizontalalignment('left')
            if not onlyalign: txt[i].set_rotation(rotation[i])
          else:
            txt[i].set_verticalalignment('center')
            txt[i].set_horizontalalignment('right')
            if not onlyalign: txt[i].set_rotation(rotation[i]+180.)

#########################################################################################
# draw edges and nodes
#########################################################################################
def draw_edges_nodes(G,pos,elist,elabel,nlist,nlabel,edgestyle,color,zord):
    curraxes=fig.gca()
    edge=nx.draw_networkx_edges(G,pos,edgelist=elist,edge_color=color,arrows=False)
    node=nx.draw_networkx_nodes(G,pos,nodelist=nlist,node_color=color,node_shape='o',node_size=50)
    if edge is not None:
      edge.set_label("edge_info=('" + curraxes.get_label() + "','" + elabel + "')")
      edge.set_zorder(zord)
      edge.set_linewidths(1.5)
      edge.set_linestyle(edgestyle)
    if node is not None:
      node.set_picker(5)
      node.set_edgecolor(bgcolor)
      node.set_label("node_info=('" + curraxes.get_label() + "','" + nlabel + "')")
      node.set_zorder(zord+10)
      node.set_linewidths(1.5)

#########################################################################################
# query data from coolfluid and build graph and lists
#########################################################################################
def append_with_successors_recursive(G,key,to_append) :
  to_append[key]=1
  for i in G.successors(key):
    append_with_successors_recursive(G,i,to_append)

def build_graph_with_lists(starturi_,nec_,nxp_,depth_,tree_,depthlimit_sofplt=True,add_parent=False,hidden='cofpslt'):

  # getting the strings which holds the script to setup the graph
  components=nxp_.get_component_graph( uri = starturi_, depth = depth_ )
  if depthlimit_sofplt==True:
    options=nxp_.get_option_graph( uri = starturi_, depth = depth_ )
    tags=nxp_.get_tag_graph( uri = starturi_, depth = depth_ )
    signals=nxp_.get_signal_graph( uri = starturi_, depth = depth_ )
    fields=nxp_.get_field_graph( uri = starturi_, depth = depth_ )
    links=nxp_.get_link_graph( uri = starturi_, depth = depth_ )
    properties=nxp_.get_property_graph( uri = starturi_, depth = depth_ )
  else:
    options=nxp_.get_option_graph( uri = starturi_, depth = depth_-1 )
    tags=nxp_.get_tag_graph( uri = starturi_, depth = depth_-1 )
    signals=nxp_.get_signal_graph( uri = starturi_, depth = depth_-1 )
    fields=nxp_.get_field_graph( uri = starturi_, depth = depth_-1 )
    links=nxp_.get_link_graph( uri = starturi_, depth = depth_-1 )
    properties=nxp_.get_property_graph( uri = starturi_, depth = depth_-1 )

  #print components
  #print options
  #print tags
  #print signals
  #print fields
  #print links
  #print properties

  # executing the submission into the graph
  if 'c' in tree_:
    for line in StringIO(components): exec 'nec_.' + line
  if 'o' in tree_:
    for line in StringIO(options   ): exec 'nec_.' + line
  if 'p' in tree_:
    for line in StringIO(properties): exec 'nec_.' + line
  if 's' in tree_:
    for line in StringIO(signals   ): exec 'nec_.' + line
  if 'f' in tree_:
    for line in StringIO(fields    ): exec 'nec_.' + line
  if 'l' in tree_:
    for line in StringIO(links     ): exec 'nec_.' + line
  if 't' in tree_:
    for line in StringIO(tags      ): exec 'nec_.' + line

  # if there is a parent node and add_parent is true, the make it mimic a regular component
  if add_parent==True:
    parn=[(u)   for (u,d)   in nec_.G.nodes(data=True) if d['tag']=='parent']
    for i in parn:
      nec_.G.node[i]['tag']='component'
    pare=[(u,v) for (u,v,d) in nec_.G.edges(data=True) if d['tag']=='parent']
    for i in pare:
      nec_.G.edge[i[0]][i[1]]['tag']='component'

  # getting out hidden from the graph if required, using dict becuase some elements could end up in a list multiple times
  dellist=dict()
  for i in nec_.G:
    if nec_.G.node[i]['hidden']==True:
      if nec_.G.node[i]['tag'][0] in hidden:
        append_with_successors_recursive(nec_.G,i,dellist)
  nec_.G.remove_nodes_from(dellist.keys())
  for i in dellist.keys():
    nec_.nodecaption.pop(i)

  # separating lists
  nec_.cn=[(u)   for (u,d)   in nec_.G.nodes(data=True) if d['tag']=='component']
  nec_.ce=[(u,v) for (u,v,d) in nec_.G.edges(data=True) if d['tag']=='component']
  nec_.ccapt=dict.fromkeys(nec_.nodecaption,'')
  for key in nec_.cn : nec_.ccapt[key]=nec_.nodecaption[key]
  nec_.on=[(u)   for (u,d)   in nec_.G.nodes(data=True) if d['tag']=='option']
  nec_.oe=[(u,v) for (u,v,d) in nec_.G.edges(data=True) if d['tag']=='option']
  nec_.ocapt=dict.fromkeys(nec_.nodecaption,'')
  for key in nec_.on : nec_.ocapt[key]=nec_.nodecaption[key]
  nec_.pn=[(u)   for (u,d)   in nec_.G.nodes(data=True) if d['tag']=='property']
  nec_.pe=[(u,v) for (u,v,d) in nec_.G.edges(data=True) if d['tag']=='property']
  nec_.pcapt=dict.fromkeys(nec_.nodecaption,'')
  for key in nec_.pn : nec_.pcapt[key]=nec_.nodecaption[key]
  nec_.sn=[(u)   for (u,d)   in nec_.G.nodes(data=True) if d['tag']=='signal']
  nec_.se=[(u,v) for (u,v,d) in nec_.G.edges(data=True) if d['tag']=='signal']
  nec_.scapt=dict.fromkeys(nec_.nodecaption,'')
  for key in nec_.sn : nec_.scapt[key]=nec_.nodecaption[key]
  nec_.fn=[(u)   for (u,d)   in nec_.G.nodes(data=True) if d['tag']=='field']
  nec_.fe=[(u,v) for (u,v,d) in nec_.G.edges(data=True) if d['tag']=='field']
  nec_.fcapt=dict.fromkeys(nec_.nodecaption,'')
  for key in nec_.fn : nec_.fcapt[key]=nec_.nodecaption[key]
  nec_.ln=[(u)   for (u,d)   in nec_.G.nodes(data=True) if d['tag']=='link']
  nec_.le=[(u,v) for (u,v,d) in nec_.G.edges(data=True) if d['tag']=='link']
  nec_.lcapt=dict.fromkeys(nec_.nodecaption,'')
  for key in nec_.ln : nec_.lcapt[key]=nec_.nodecaption[key]
  nec_.tn=[(u)   for (u,d)   in nec_.G.nodes(data=True) if d['tag']=='tag']
  nec_.te=[(u,v) for (u,v,d) in nec_.G.edges(data=True) if d['tag']=='tag']
  nec_.tcapt=dict.fromkeys(nec_.nodecaption,'')
  for key in nec_.tn : nec_.tcapt[key]=nec_.nodecaption[key]

  return nec_


#########################################################################################
# starturi: from where the recursive listing starts
# depth: how many levels to list down deep in starturi's subtree
# tree: what to plot in the tree. Combination of chars 'cosfl'
# caption: what to label in the tree. Combination of chars 'cosfl'
#  'c': component
#  'o': option
#  't': tag
#  'p': property
#  's': signal
#  'f': field
#  'l': link
# printdestination: 'sc' if user clicks on a node, the component information is printed
#  's': to screen (matplotlib window)
#  'c': console (terminal)
# depthlimit_sofpl: bool option to include or exclude printing signals, options, fields,
#   properties and links at depthlimit level
# hidden: wether to display or not cospfl element which has tag 'hidden' set to true
#
# zorder numbers:
#   1x:  edges
#   2x:  nodes, hardcoded to edges +10
#   3x:  captions
#   40:  cf logo middle backdrop rectangle
#   41:  cf logo image
#   50:  title
#   51:  component info
#########################################################################################
def show_graph(starturi,depth=1000,tree='cofl',caption='',printdestination='c',hidden='cospflt'):

  # create the collector component on coolfluid side
  nxp = root.create_component('NetworkXPython','cf3.python.NetworkXPython')

  # create and configure handler class in python side
  nec=nx_event_connector("main")
  nec.printdestination=printdestination
  nec.hidden=hidden
  cid=fig.canvas.mpl_connect('pick_event', nec.on_pick)

  # get a string with a nice name of the component and show it in title
  start_key=root.access_component(str(starturi)).uri().path()
  titletxt.set_text("COOLFluiD3 component graph starting from '" + start_key + "'")
  fig.canvas.set_window_title(start_key)

  # build the graph
  nec=build_graph_with_lists(starturi,nec,nxp,depth,tree,hidden=hidden)

  # computing the positions of the nodes via layouts
  #nec.pos=nx.graphviz_layout(nec.G,prog='twopi')
  #nec.pos=nx.graphviz_layout(nec.G,prog='circo',root=start_key)
  #nec.pos=nx.spring_layout(nec.G,iterations=50)
  #nec.pos=traditional_left2right_tree_layout(nec.G,start_key)
  nec.pos=traditional_left2right_tree_layout_in_polar(nec.G,start_key)
  #pos2=traditional_left2right_tree_layout_in_polar(nec.G,start_key)
  #nec.pos=nx.spring_layout(nec.G,pos=pos2,iterations=50)

  # renormalizing positions in the range of zoomfact & root position in zoomfact
  # also computing angles, just in case
  nec.pos=normalize_coordinates(start_key,nec.pos,cf3_img)
  rot=compute_edge_angles_for_target_nodes(nec.G,nec.pos)

  # print statistics
  print "Statistics for: ", start_key
  print "Number of        nodes  &  edges"
  print "--------------------------------"
  print "Components: %10d%10d" % ( len(nec.cn), len(nec.ce) )
  print "Options:    %10d%10d" % ( len(nec.on), len(nec.oe) )
  print "Tags:       %10d%10d" % ( len(nec.tn), len(nec.te) )
  print "Properties: %10d%10d" % ( len(nec.pn), len(nec.pe) )
  print "Signals:    %10d%10d" % ( len(nec.sn), len(nec.se) )
  print "Fields:     %10d%10d" % ( len(nec.fn), len(nec.fe) )
  print "Links:      %10d%10d" % ( len(nec.ln), len(nec.le) )
  print "TOTAL:      %10d%10d" % ( len(nec.G.nodes()), len(nec.G.edges()) )

  # draw the nodes and edges
  if 's' in tree: draw_edges_nodes(nec.G,nec.pos,nec.se,'se',nec.sn,'sn','solid',signalcolor,10)
  if 'p' in tree: draw_edges_nodes(nec.G,nec.pos,nec.pe,'pe',nec.pn,'pn','solid',propertycolor,11)
  if 'f' in tree: draw_edges_nodes(nec.G,nec.pos,nec.fe,'fe',nec.fn,'fn','solid',fieldcolor,12)
  if 't' in tree: draw_edges_nodes(nec.G,nec.pos,nec.te,'te',nec.tn,'tn','solid',tagcolor,13)
  if 'o' in tree: draw_edges_nodes(nec.G,nec.pos,nec.oe,'oe',nec.on,'on','solid',optioncolor,14)
  if 'l' in tree: draw_edges_nodes(nec.G,nec.pos,nec.le,'le',nec.ln,'ln','dashed',linkcolor,15)
  if 'c' in tree: draw_edges_nodes(nec.G,nec.pos,nec.ce,'ce',nec.cn,'cn','solid',componentcolor,16)

  # draw the
  if 's' in caption: draw_captions(nec.G,nec.pos,nec.scapt,'sc',signalcolor,40,rotation=rot)
  if 'p' in caption: draw_captions(nec.G,nec.pos,nec.pcapt,'pc',propertycolor,41,rotation=rot)
  if 'f' in caption: draw_captions(nec.G,nec.pos,nec.fcapt,'fc',fieldcolor,42,rotation=rot)
  if 't' in caption: draw_captions(nec.G,nec.pos,nec.tcapt,'tc',tagcolor,43,rotation=rot)
  if 'o' in caption: draw_captions(nec.G,nec.pos,nec.ocapt,'oc',optioncolor,44,rotation=rot)
  if 'l' in caption: draw_captions(nec.G,nec.pos,nec.lcapt,'lc',linkcolor,45,rotation=rot)
  if 'c' in caption: draw_captions(nec.G,nec.pos,nec.ccapt,'cc',componentcolor,46,rotation=rot)

  # showing the plot
  print "---------- SHOW ----------"
  plt.show()

  # delete the collector component

