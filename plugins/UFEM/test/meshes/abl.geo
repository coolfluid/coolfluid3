cl = 40;

xmin = 0;
xmax = 5000;
ymin = 0;
ymax = 500;

box_x0 = 1000;
box_x1 = 1200;
box_y0 = 0;
box_y1 = 200;

// Domain corners
Point(1) = {xmin, ymin, 0, cl};
Point(2) = {xmax, ymin, 0, cl};
Point(3) = {xmax, ymax, 0, cl};
Point(4) = {xmin, ymax, 0, cl};

// Box corners
Point(5) = {box_x0, box_y0, 0, cl};
Point(6) = {box_x1, box_y0, 0, cl};
Point(7) = {box_x1, box_y1, 0, cl};
Point(8) = {box_x0, box_y1, 0, cl};

// Domain borders
Line(1) = {1,5};
Line(2) = {5,6};
Line(3) = {6,2};
Line(4) = {2,3};
Line(5) = {3,4};
Line(6) = {4,1};

// Box sides
Line(7) = {6,7};
Line(8) = {7,8};
Line(9) = {8,5};

// loop around the domain
Line Loop(10) = {1,2,3,4,5,6};

// Loop around the box
Line Loop(11) = {2,7,8,9};

Plane Surface(12) = {10,-11};
Plane Surface(13) = {11};

Physical Surface("interior") = {12}; // The fluid domain
Physical Surface("box") = {13}; // The fluid domain
Physical Line("in") = {6};
Physical Line("out") = {4};
Physical Line("top") = {5};
Physical Line("bottom") = {1,2,3};

Field[1] = MathEval;
Field[1].F = "5+1.075^(y/10.)";

Background Field = 1;
