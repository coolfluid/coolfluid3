cl =2; 
kl = cl/2;
sl = cl/10;
lc = 0.3;
// ruwheid mesh

// hoekpunten SBuis 
Point(1) = {-50,0,0,lc};
Point(2) = {-50,-50,0,lc};
Point(3) = {250,-25,0,lc};
Point(4) = {250,-75,0,lc};

// Uitersten geplooide deel
Point(6) = {29,0,0,lc};
Point(11) = {151,-75,0,lc};
Point(8) = {151,-25,0,lc};
Point(9) = {29,-50,0,lc};



// Punten geplooide deel
Point(12) = {30,-0.0011349467,0,lc};
Point(13) = {36,-0.0030848644,0,lc};
Point(14) = {42,-0.0083837533,0,lc};
Point(15) = {48,-0.0227762799,0,lc};
Point(16) = {54,-0.0618155789,0,lc};
Point(17) = {60,-0.1673212731,0,lc};
Point(18) = {66,-0.4496552491,0,lc};
Point(19) = {72,-1.1856468294,0,lc};
Point(20) = {78,-2.9800730506,0,lc};
Point(21) = {84,-6.7235355342,0,lc};
Point(22) = {90,-12.5,0,lc};
Point(23) = {96,-18.2764644658,0,lc};
Point(24) = {102,-22.0199269494,0,lc};
Point(25) = {108,-23.8143531706,0,lc};
Point(26) = {114,-24.5503447509,0,lc};
Point(27) = {120,-24.8326787269,0,lc};
Point(28) = {126,-24.9381844211,0,lc};
Point(29) = {132,-24.9772237201,0,lc};
Point(30) = {138,-24.9916162467,0,lc};
Point(31) = {144,-24.9969151356,0,lc};
Point(32) = {150,-24.9988650533,0,lc};

Point(33) = {30,-50.0011349467,0,lc};
Point(34) = {36,-50.0030848644,0,lc};
Point(35) = {42,-50.0083837533,0,lc};
Point(36) = {48,-50.0227762799,0,lc};
Point(37) = {54,-50.0618155789,0,lc};
Point(38) = {60,-50.1673212731,0,lc};
Point(39) = {66,-50.4496552491,0,lc};
Point(40) = {72,-51.1856468294,0,lc};
Point(41) = {78,-52.9800730506,0,lc};
Point(42) = {84,-56.7235355342,0,lc};
Point(43) = {90,-62.5,0,lc};
Point(44) = {96,-68.2764644658,0,lc};
Point(45) = {102,-72.0199269494,0,lc};
Point(46) = {108,-73.8143531706,0,lc};
Point(47) = {114,-74.5503447509,0,lc};
Point(48) = {120,-74.8326787269,0,lc};
Point(49) = {126,-74.9381844211,0,lc};
Point(50) = {132,-74.9772237201,0,lc};
Point(51) = {138,-74.9916162467,0,lc};
Point(52) = {144,-74.9969151356,0,lc};
Point(53) = {150,-74.9988650533,0,lc};



// Omkadering van domein
Line(1) = {1,6};
Line(2) = {9,2};
Line(3) = {8,3};
Line(4) = {4,11};
Line(5)= {2,1};
Line(6)= {3,4};




// Splines voor geplooide deel
Spline(7) = {6,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,8};

Spline(8) = {11,53,52,51,50,49,48,47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,9};

// Line Loop om omkadering te maken
Line Loop(9) = {1,7,3,6,4,8,2,5};
// Definieren van dit domein als oppervlak
Plane Surface(10) = {9};
Physical Surface("interior") = {10};
//Transfinite Surface{10} = {1,2,3,4};
// 
Physical Line("inlet") = {5};
Physical Line("outlet") = {6};
Physical Line("top") = {1,7,3}; // 7 van Spline
Physical Line("bottom") = {4,8,2}; // 8 van Spline

lc2 = lc*3;
// Punten Middelste deel
Point(54) = {-50,-25,0,lc2};

Point(55) = {29,-25,0,lc};
Point(56) = {151,-50,0,lc};
Point(57) = {250,-50,0,lc};

Point(58) = {30,-25.0011349467,0,lc2};
Point{58} In Surface{10}; 
Point(59) = {36,-25.0030848644,0,lc};
Point(60) = {42,-25.0083837533,0,lc};
Point(61) = {48,-25.0227762799,0,lc};
Point(62) = {54,-25.0618155789,0,lc};
Point(63) = {60,-25.1673212731,0,lc2};
Point{63} In Surface{10}; 
Point(64) = {66,-25.4496552491,0,lc};
Point(65) = {72,-26.1856468294,0,lc};
Point(66) = {78,-27.9800730506,0,lc};
Point(67) = {84,-31.7235355342,0,lc};
Point(68) = {90,-37.5,0,lc2};
Point{68} In Surface{10}; 
Point(69) = {96,-43.2764644658,0,lc};
Point(70) = {102,-47.0199269494,0,lc};
Point(71) = {108,-48.8143531706,0,lc};
Point(72) = {114,-49.5503447509,0,lc};
Point(73) = {120,-49.8326787269,0,lc2};
Point{73} In Surface{10}; 
Point(74) = {126,-49.9381844211,0,lc};
Point(75) = {132,-49.9772237201,0,lc};
Point(76) = {138,-49.9916162467,0,lc};
Point(77) = {144,-49.9969151356,0,lc};
Point(78) = {150,-49.9988650533,0,lc2};
Point{78} In Surface{10}; 

// Rechte stukken links en rechts centraal gelegen
Point(79) = {20,-25,0,lc2};
Point{79} In Surface{10}; 
Point(80) = {10,-25,0,lc2};
Point{80} In Surface{10}; 
Point(81) = {0,-25,0,lc2};
Point{81} In Surface{10}; 
Point(82) = {-10,-25,0,lc2};
Point{82} In Surface{10}; 
Point(83) = {-20,-25,0,lc2};
Point{83} In Surface{10}; 
Point(95) = {-30,-25,0,lc2};
Point{95} In Surface{10}; 
Point(96) = {-40,-25,0,lc2};
Point{96} In Surface{10}; 

Point(84) = {160,-50,0,lc2};
Point{84} In Surface{10}; 
Point(85) = {170,-50,0,lc2};
Point{85} In Surface{10}; 
Point(86) = {180,-50,0,lc2};
Point{86} In Surface{10}; 
Point(87) = {190,-50,0,lc2};
Point{87} In Surface{10}; 
Point(88) = {200,-50,0,lc2};
Point{88} In Surface{10}; 
Point(90) = {210,-50,0,lc2};
Point{90} In Surface{10}; 
Point(91) = {220,-50,0,lc2};
Point{91} In Surface{10}; 
Point(92) = {230,-50,0,lc2};
Point{92} In Surface{10}; 












