cl=40;
Point(1) = {-750, -250, 250, cl};
Point(2) = {-750, -250, -250, cl};
Point(3) = {-750, 250, -250, cl};
Point(4) = {-750, 250, 250, cl};
Point(5) = {1500, 250, 250, cl};
Point(6) = {1500, -250, 250, cl};
Point(7) = {1500, -250, -250, cl};
Point(8) = {1500, 250, -250, cl};

Point(24) = {630, 0, 0, cl/50};
Point(25) = {630, 63, 0, cl/50};
Point(26) = {630, 0, 63, cl/50};
Point(27) = {630, 0, -63, cl/50};
Point(28) = {630, -63, 0, cl/50};

Point(29) = {0, 0, 0, cl/50};
Point(30) = {0, 63, 0, cl/50};
Point(31) = {0, 0, 63, cl/50};
Point(32) = {0, 0, -63, cl/50};
Point(33) = {0, -63, 0, cl/50};


Point(14) = {-250, 0, 0, cl};
Point(15) = {1000, 0, 0, cl};
Point(16) = {126, -250, 0, cl};
Point(17) = {126, 250, 0, cl};
Point(18) = {252, -250, 0, cl};
Point(19) = {252, 250, 0, cl};
Point(20) = {504, -250, 0, cl};
Point(21) = {504, 250, 0, cl};
Point(22) = {756, -250, 0, cl};
Point(23) = {756, 250, 0, cl};

Circle(22) = {27, 24, 25};
Circle(23) = {25, 24, 26};
Circle(24) = {26, 24, 28};
Circle(25) = {28, 24, 27};

Circle(26) = {32, 29, 30};
Circle(27) = {30, 29, 31};
Circle(28) = {31, 29, 33};
Circle(29) = {33, 29, 32};

Line(5) = {3, 4};
Line(6) = {4, 1};
Line(7) = {1, 2};
Line(8) = {2, 3};
Line(9) = {3, 8};
Line(10) = {4, 5};
Line(11) = {2, 7};
Line(12) = {1, 6};
Line(13) = {6, 7};
Line(14) = {7, 8};
Line(15) = {8, 5};
Line(16) = {5, 6};

Line(17) = {14, 15};
Line(18) = {16, 17};
Line(19) = {18, 19};
Line(20) = {20, 21};
Line(21) = {22, 23};




Line Loop(17) = {8, 5, 6, 7};
Plane Surface(18) = {17};
Line Loop(19) = {5, 10, -15, -9};
Plane Surface(20) = {19};
Line Loop(21) = {14, 15, 16, 13};
Plane Surface(22) = {21};
Line Loop(23) = {7, 11, -13, -12};
Plane Surface(24) = {23};
Line Loop(25) = {8, 9, -14, -11};
Plane Surface(26) = {25};
Line Loop(27) = {6, 12, -16, -10};
Plane Surface(28) = {27};
Line Loop(31) = {23, 24, 25, 22};
Plane Surface(32) = {31};
Line Loop(33) = {27, 28, 29, 26};
Plane Surface(34) = {33};

Surface Loop(37) = {32};
Surface Loop(39) = {34};
Surface Loop(55) = {26, 18, 20, 28, 24, 22};
Volume(56) = {55, 37, 39};

Physical Surface('inlet') = {18};
Physical Surface('top') = {20};
Physical Surface('outlet') = {22};
Physical Surface('bottom') = {24};
Physical Surface('left') = {26};
Physical Surface('right') = {28};
Physical Surface('actuator_in1') = {34};
Physical Surface('actuator_in2') = {32};
Physical Volume('interior') = {56};
