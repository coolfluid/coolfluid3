
// Configuration
// -------------

// dimensions
outer_radius  = 0.5;
square_length = outer_radius/2 * Cos(Pi/4);

// divisions
nb_circ_div   = 32;
nb_radial_div = nb_circ_div/4;

// order
Mesh.ElementOrder      = 1; // P1 elements
Mesh.SecondOrderLinear = 0; // curved elements in case P2


// Square
// ------

square_pt=square_length;
pq1 = newp; Point(pq1) = { square_pt,  square_pt, 0};
pq2 = newp; Point(pq2) = {-square_pt,  square_pt, 0};
pq3 = newp; Point(pq3) = {-square_pt, -square_pt, 0};
pq4 = newp; Point(pq4) = { square_pt, -square_pt, 0};

lqt = newl; Line(lqt) = {pq1, pq2};
lql = newl; Line(lql) = {pq2, pq3};
lqb = newl; Line(lqb) = {pq3, pq4};
lqr = newl; Line(lqr) = {pq4, pq1};


// Circle
// ------

circle_pt=outer_radius*Cos(Pi/4);
pc1 = newp; Point(pc1) = { circle_pt,  circle_pt, 0};
pc2 = newp; Point(pc2) = {-circle_pt,  circle_pt, 0};
pc3 = newp; Point(pc3) = {-circle_pt, -circle_pt, 0};
pc4 = newp; Point(pc4) = { circle_pt, -circle_pt, 0};

pcentre = newp; Point(pcentre) = {0, 0, 0};
lct = newl; Circle(lct) = {pc1,pcentre,pc2};
lcl = newl; Circle(lcl) = {pc2,pcentre,pc3};
lcb = newl; Circle(lcb) = {pc3,pcentre,pc4};
lcr = newl; Circle(lcr) = {pc4,pcentre,pc1};


// Surfaces
// --------

// Connection square-circle
l11 = newl; Line(l11) = {pq1,pc1};
l22 = newl; Line(l22) = {pq2,pc2};
l33 = newl; Line(l33) = {pq3,pc3};
l44 = newl; Line(l44) = {pq4,pc4};

// Set divisions
Transfinite Line{lct,lqt,lqb,lcb} = nb_circ_div/4+1 Using Progression 1;
Transfinite Line{lcl,lql,lqr,lcr} = nb_circ_div/4+1 Using Progression 1;
Transfinite Line{l11,l22,l33,l44} = nb_radial_div+1 Using Progression 1;

// square surface
llq = newll; Line Loop(llq) = {lqt,lql,lqb,lqr};
sq  = news;  Plane Surface(sq) = {llq}; 
Transfinite Surface{sq} = {pq1,pq2,pq3,pq4}; Recombine Surface{sq};

// top surface
llt = newll; Line Loop(llt) = {l11,lct,-l22,-lqt};
st  = news;  Plane Surface(st) = {llt}; 
Transfinite Surface{st} = {pq1,pc1,pc2,pq2}; Recombine Surface{st};

// left surface
lll = newll; Line Loop(lll) = {l22,lcl,-l33,-lql};
sl  = news;  Plane Surface(sl) = {lll}; 
Transfinite Surface{sl} = {pq2,pc2,pc3,pq3}; Recombine Surface{sl};

// bottom surface
llb = newll; Line Loop(llb) = {l33,lcb,-l44,-lqb};
sb  = news;  Plane Surface(sb) = {llb}; 
Transfinite Surface{sb} = {pq3,pc3,pc4,pq4}; Recombine Surface{sb};

// right surface
llr = newll; Line Loop(llr) = {l44,lcr,-l11,-lqr};
sr  = news;  Plane Surface(sr) = {llr}; 
Transfinite Surface{sr} = {pq4,pc4,pc1,pq1}; Recombine Surface{sr};


// Physical groups
// ---------------

Physical Line("boundary")    = {lct, lcl, lcb, lcr};
Physical Surface("interior") = {sq, st, sl, sb, sr};
