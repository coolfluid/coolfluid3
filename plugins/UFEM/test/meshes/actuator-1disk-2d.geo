cl = 40;
x_min = -2000;
x_max = 2000;
y_min = -1500;
y_max = 1500;
d=126;
th=10;
refine=10;
refine_box = 5;

Point(1) = {x_min, y_min, 0, cl};
Point(2) = {x_max-refine_box, y_min,  0, cl} ;
Point(3) = {x_max-refine_box, y_max, 0, cl} ;
Point(4) = {x_min,  y_max, 0, cl} ;

Line(1) = {1,2} ;
Line(2) = {3,2} ;
Line(3) = {3,4} ;
Line(4) = {4,1} ;

Line Loop(4) = {4,1,-2,3};

// The disk
Point(5) = {-th/2, d/2, 0, cl/refine};
Point(6) = {-th/2, -d/2, 0, cl/refine};
Point(7) = {th/2, d/2, 0, cl/refine};
Point(8) = {th/2, -d/2, 0, cl/refine};

// Outside of the disk
Point(15) = {-d/4,d/2,0,cl/refine};
Point(16) = {-d/4, -d/2, 0, cl/refine};
Point(17) = {d/4, d/2, 0, cl/refine};
Point(18) = {d/4, -d/2, 0, cl/refine};

Line(5) = {15,16};
Line(6) = {16,6};
Line(7) = {6,5};
Line(8) = {5,15};

Line(16) = {6,8};
Line(17) = {8,7};
Line(18) = {7,5};

Line(26) = {8,18};
Line(27) = {18,17};
Line(28) = {17, 7};

Line Loop(5) = {5,6,7,8};
Line Loop(6) = {16,17,18,-7};
Line Loop(7) = {26,27,28,-17};

// Refine box at the oultet
Point(9) = {x_max, y_min, 0, cl};
Point(10) = {x_max, y_max, 0, cl};
Line(11) = {9, 10};
Line(12) = {10, 3};
Line(13) = {2, 9};
Line Loop(12) = {13, 11, 12, 2}; 
Plane Surface(12) = {12};

// Structured mesh at the outlet
Transfinite Line{11, 2} = (y_max-y_min)/cl;
Transfinite Line{12, 13} = 6;
Transfinite Surface{12};
Recombine Surface{12};

Plane Surface(20) = {4,-5,-6,-7};
Plane Surface(21) = {5};
Plane Surface(22) = {6};
Plane Surface(23) = {7};

Physical Surface("interior") = {20, 12} ;
Physical Line("inlet") = {4};
Physical Line("outlet") = {11};
Physical Line("top") = {3, 12};
Physical Line("bottom") = {1, 13};
Physical Surface("actuator") = {22};
Physical Surface("out_actuator") = {21,22,23};

