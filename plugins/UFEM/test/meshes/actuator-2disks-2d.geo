cl = 40;
x_min = -1500;
x_max = 2500;
y_min = -1500;
y_max = 1500;
d=126;
th=10;
refine=10;
refine_box = 5;
disc1_x=0;
disc1_y=0;

disc2_x=630;
disc2_y=0;


Point(1) = {x_min, y_min, 0, cl};
Point(2) = {x_max-refine_box, y_min,  0, cl} ;
Point(3) = {x_max-refine_box, y_max, 0, cl} ;
Point(4) = {x_min,  y_max, 0, cl} ;

Line(1) = {1,2} ;
Line(2) = {3,2} ;
Line(3) = {3,4} ;
Line(4) = {4,1} ;

Line Loop(5) = {4,1,-2,3};


// Refine box at the oultet
Point(9) = {x_max, y_min, 0, cl};
Point(10) = {x_max, y_max, 0, cl};
Line(11) = {9, 10};
Line(12) = {10, 3};
Line(13) = {2, 9};
Line Loop(12) = {13, 11, 12, 2}; 

// Disc 1
Point(5) = {disc1_x-th/2, disc1_y+d/2, 0, cl/refine};
Point(6) = {disc1_x-th/2, disc1_y-d/2, 0, cl/refine};
Point(7) = {disc1_x+th/2, disc1_y+d/2, 0, cl/refine};
Point(8) = {disc1_x+th/2, disc1_y-d/2, 0, cl/refine};

Line(7) = {5,6};
Line(8) = {8,7};
Line(9) = {6,8};
Line(10) = {7,5};
//Line{7,8,9,10} In Surface{6};
Line Loop(11) = {7,8,9,10};


// Disc 2
Point(11) = {disc2_x-th/2, disc2_y+d/2, 0, cl/refine};
Point(12) = {disc2_x-th/2, disc2_y-d/2, 0, cl/refine};
Point(13) = {disc2_x+th/2, disc2_y+d/2, 0, cl/refine};
Point(14) = {disc2_x+th/2, disc2_y-d/2, 0, cl/refine};

Line(14) = {11,12};
Line(15) = {14,13};
Line(16) = {12,14};
Line(17) = {13,11};
Line Loop(13) = {14,15,16,17};

Plane Surface(6) = {5, -11, -13}; // Volume minus discs 1 and 2
Plane Surface(7) = {11}; // disc1 only
Plane Surface(9) = {13}; // disc2 only
Plane Surface(8) = {12};

// Structured mesh at the outlet
Transfinite Line{11, 2} = (y_max-y_min)/cl;
Transfinite Line{12, 13} = 6;
Transfinite Surface{8};
Recombine Surface{8};

Physical Surface("interior") = {6, 8} ;
Physical Line("inlet") = {4};
Physical Line("outlet") = {11};
Physical Line("top") = {3, 12};
Physical Line("bottom") = {1, 13};
Physical Surface("actuator1") = {7};
Physical Surface("actuator2") = {9};
