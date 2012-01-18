Mesh.ElementOrder = 2;
Mesh.SecondOrderLinear = 0; //--> ensures curved boundary instead of multiple points on straight faces

// Uncomment the desired grid
grid = 1;  // 16x4
// grid = 2;  // 32x8
// grid = 3;  // 64x16
// grid = 4;  // 128x32

// constants
rad = 0.5;
far = 20.0;
nCon = 33;
nCir = 128;
nDiv = 2^2;
If(grid==1)
  nDiv = 8;
EndIf
If(grid==2)
  nDiv = 4;
EndIf
If(grid==3)
  nDiv = 2;
EndIf
If(grid==4)
  nDiv = 1;
EndIf

// parameters
nConD = (nCon-1)/nDiv + 1;
nCirD = nCir/nDiv;

nCirDD4 = nCirD/4;
twoPiD128 = 2*Pi/128;
alpha = 1.1648336;

// points
p0 = newp; Point(p0) = {0.0,0.0,0.0,1.0};
pC1 = newp; Point(pC1) = {+rad,0.0,0.0,1.0};
pC2 = newp; Point(pC2) = {0.0,+rad,0.0,1.0};
pC3 = newp; Point(pC3) = {-rad,0.0,0.0,1.0};
pC4 = newp; Point(pC4) = {0.0,-rad,0.0,1.0};

pF1 = newp; Point(pF1) = {+far,0.0,0.0,1.0};
pF2 = newp; Point(pF2) = {0.0,+far,0.0,1.0};
pF3 = newp; Point(pF3) = {-far,0.0,0.0,1.0};
pF4 = newp; Point(pF4) = {0.0,-far,0.0,1.0};

// circles
lC1 = newl; Circle(lC1) = {pC1,p0,pC2};
lC2 = newl; Circle(lC2) = {pC2,p0,pC3};
lC3 = newl; Circle(lC3) = {pC3,p0,pC4};
lC4 = newl; Circle(lC4) = {pC4,p0,pC1};

lF1 = newl; Circle(lF1) = {pF1,p0,pF2};
lF2 = newl; Circle(lF2) = {pF2,p0,pF3};
lF3 = newl; Circle(lF3) = {pF3,p0,pF4};
lF4 = newl; Circle(lF4) = {pF4,p0,pF1};

// transfinite
Transfinite Line{lC1,lC2,lC3,lC4} = nCirDD4+1 Using Progression 1.0;

// structured mesh
pjprev1 = pC1;
pjprev2 = pC2;
pjprev3 = pC3;
pjprev4 = pC4;
ljprev1 = lC1;
ljprev2 = lC2;
ljprev3 = lC3;
ljprev4 = lC4;
For j In {1:nConD-2}
  // radius
  radj = 0;
  For k In {0:nDiv*j-1}
    radj = radj + alpha^k;
  EndFor
  radj = radj*twoPiD128;
  radj = rad*(1+radj);
  
  // points
  pj1 = newp; Point(pj1) = {+radj,0.0,0.0,1.0};
  pj2 = newp; Point(pj2) = {0.0,+radj,0.0,1.0};
  pj3 = newp; Point(pj3) = {-radj,0.0,0.0,1.0};
  pj4 = newp; Point(pj4) = {0.0,-radj,0.0,1.0};
  
  // circles
  lj1 = newl; Circle(lj1) = {pj1,p0,pj2};
  lj2 = newl; Circle(lj2) = {pj2,p0,pj3};
  lj3 = newl; Circle(lj3) = {pj3,p0,pj4};
  lj4 = newl; Circle(lj4) = {pj4,p0,pj1};

  // lines
  lR1 = newl; Line(lR1) = {pjprev1,pj1};
  lR2 = newl; Line(lR2) = {pjprev2,pj2};
  lR3 = newl; Line(lR3) = {pjprev3,pj3};
  lR4 = newl; Line(lR4) = {pjprev4,pj4};

  // transfinite lines
  Transfinite Line{lj1,lj2,lj3,lj4} = nCirDD4+1 Using Progression 1.0;
  Transfinite Line{lR1,lR2,lR3,lR4} = 2 Using Progression 1.0;
  
  // line loops
  ll1 = newl; Line Loop(ll1) = {lj1,-lR2,-ljprev1,lR1};
  ll2 = newl; Line Loop(ll2) = {lj2,-lR3,-ljprev2,lR2};
  ll3 = newl; Line Loop(ll3) = {lj3,-lR4,-ljprev3,lR3};
  ll4 = newl; Line Loop(ll4) = {lj4,-lR1,-ljprev4,lR4};
  
  // surfaces
  s1 = news; Plane Surface(s1) = {ll1};
  s2 = news; Plane Surface(s2) = {ll2};
  s3 = news; Plane Surface(s3) = {ll3};
  s4 = news; Plane Surface(s4) = {ll4};
  
  // store surfaces
  surfaces[(j-1)*4+0] = s1;
  surfaces[(j-1)*4+1] = s2;
  surfaces[(j-1)*4+2] = s3;
  surfaces[(j-1)*4+3] = s4;
  
  // transfinite surfaces
  Transfinite Surface{s1} = {pj1,pj2,pjprev2,pjprev1};
  Transfinite Surface{s2} = {pj2,pj3,pjprev3,pjprev2};
  Transfinite Surface{s3} = {pj3,pj4,pjprev4,pjprev3};
  Transfinite Surface{s4} = {pj4,pj1,pjprev1,pjprev4};
  
  // recombine surfaces
  Recombine Surface(s1);
  Recombine Surface(s2);
  Recombine Surface(s3);
  Recombine Surface(s4);

  // store prev points and lines
  pjprev1 = pj1;
  pjprev2 = pj2;
  pjprev3 = pj3;
  pjprev4 = pj4;
  ljprev1 = lj1;
  ljprev2 = lj2;
  ljprev3 = lj3;
  ljprev4 = lj4;
EndFor


// lines
lR1 = newl; Line(lR1) = {pjprev1,pF1};
lR2 = newl; Line(lR2) = {pjprev2,pF2};
lR3 = newl; Line(lR3) = {pjprev3,pF3};
lR4 = newl; Line(lR4) = {pjprev4,pF4};

// transfinite lines
Transfinite Line{lF1,lF2,lF3,lF4} = nCirDD4+1 Using Progression 1.0;
Transfinite Line{lR1,lR2,lR3,lR4} = 2 Using Progression 1.0;
  
// line loops
ll1 = newl; Line Loop(ll1) = {lF1,-lR2,-ljprev1,lR1};
ll2 = newl; Line Loop(ll2) = {lF2,-lR3,-ljprev2,lR2};
ll3 = newl; Line Loop(ll3) = {lF3,-lR4,-ljprev3,lR3};
ll4 = newl; Line Loop(ll4) = {lF4,-lR1,-ljprev4,lR4};
  
// surfaces
s1 = news; Plane Surface(s1) = {ll1};
s2 = news; Plane Surface(s2) = {ll2};
s3 = news; Plane Surface(s3) = {ll3};
s4 = news; Plane Surface(s4) = {ll4};

// store surfaces
surfaces[(nConD-2)*4+0] = s1;
surfaces[(nConD-2)*4+1] = s2;
surfaces[(nConD-2)*4+2] = s3;
surfaces[(nConD-2)*4+3] = s4;

// transfinite surfaces
Transfinite Surface{s1} = {pF1,pF2,pjprev2,pjprev1};
Transfinite Surface{s2} = {pF2,pF3,pjprev3,pjprev2};
Transfinite Surface{s3} = {pF3,pF4,pjprev4,pjprev3};
Transfinite Surface{s4} = {pF4,pF1,pjprev1,pjprev4};

// recombine surfaces
Recombine Surface(s1);
Recombine Surface(s2);
Recombine Surface(s3);
Recombine Surface(s4);

// physical lines and surfaces
Physical Line("cylinder") = {lC1,lC2,lC3,lC4};// Cyl {1, 2, 3, 4};
Physical Line("boundary") = {lF1,lF2,lF3,lF4};// Far {5, 6, 7, 8};
Physical Surface("interior") = {surfaces[]};
