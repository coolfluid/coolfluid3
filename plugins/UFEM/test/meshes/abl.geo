cl = 40;

xmin = 0;
xmax = 5000;
ymin = 0;
ymax = 500;

// Domain corners
Point(1) = {xmin, ymin, 0, cl};
Point(2) = {xmax, ymin, 0, cl} ;
Point(3) = {xmax, ymax, 0, cl} ;
Point(4) = {xmin, ymax, 0, cl} ;

// Domain borders
Line(1) = {1,2} ;
Line(2) = {3,2} ;
Line(3) = {3,4} ;
Line(4) = {4,1} ;

// loop around the domain
Line Loop(5) = {4,1,-2,3};

Plane Surface(6) = {5};

Physical Surface("interior") = {6}; // The fluid domain
Physical Line("in") = {4};
Physical Line("out") = {2};
Physical Line("top") = {3};
Physical Line("bottom") = {1};

Field[1] = MathEval;
Field[1].F = "5+1.075^(y/10.)";

Background Field = 1;
