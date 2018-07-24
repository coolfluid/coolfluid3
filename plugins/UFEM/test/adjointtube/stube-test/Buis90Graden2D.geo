cl =2;
 
Point(1) = {0,0,0,cl};
Point(2) = {0,-50,0,cl};
Point(3) = {150,-200,0,cl};
Point(4) = {200,-200,0,cl};

Point(5) = {140,-50,0,cl/3};
Point(6) = {150,-60,0,cl/3};
Point(7) = {140,-60,0,cl/3};

Point(8) = {200,-20,0,cl/3};
Point(9) = {180,0,0,cl/3};
Point(10) = {180,-20,0,cl/3};


Line(1) = {1, 9};
Line(2) = {8, 4};
Line(3) = {4, 3};
Line(4) = {3, 6};
Line(5) = {5, 2};
Line(6) = {2, 1};
Circle(7) = {9, 10, 8};
Circle(8) = {6, 7, 5};

// Line Loop om omkadering te maken
Line Loop(9) = {6,1,7,2,3,4,8,5};
// Definieren van dit domein als oppervlak
Plane Surface(10) = {9};
Physical Surface("interior") = {10};

// 
Physical Line("inlet") = {6};
Physical Line("outlet") = {3};
Physical Line("top") = {1,7,2}; // 7 van Spline
Physical Line("bottom") = {4,8,5}; // 8 van Spline







