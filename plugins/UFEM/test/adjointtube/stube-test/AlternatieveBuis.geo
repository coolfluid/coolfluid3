cl =0.5; 
kl = cl/2;
sl = cl/10;
lc = 0.8;

// ruwheid mesh

// hoekpunten SBuis 
Point(1) = {-50,0,0,lc};
Point(2) = {-50,-70,0,lc};
Point(3) = {300,-25,0,lc};
Point(4) = {300,-95,0,lc};

// Uitersten geplooide deel
Point(6) = {29,0,0,lc};
Point(11) = {231,-95,0,lc};
Point(8) = {231,-25,0,lc};
Point(9) = {29,-70,0,lc};

// Punten geplooide deel
//Point(12) = {30,-0.0011349467,0,lc};
Point(13) = {40,-0.0030848644,0,lc};
Point(14) = {50,-0.0083837533,0,lc};
Point(15) = {60,-0.0227762799,0,lc};
Point(16) = {70,-0.0618155789,0,lc};
Point(17) = {80,-0.1673212731,0,lc};
Point(18) = {90,-0.4496552491,0,lc};
Point(19) = {100,-1.1856468294,0,lc};
Point(20) = {110,-2.9800730506,0,lc};
Point(21) = {120,-6.7235355342,0,lc};
Point(22) = {130,-12.5,0,lc};
Point(23) = {140,-18.2764644658,0,lc};
Point(24) = {150,-22.0199269494,0,lc};
Point(25) = {160,-23.8143531706,0,lc};
Point(26) = {170,-24.5503447509,0,lc};
Point(27) = {180,-24.8326787269,0,lc};
Point(28) = {190,-24.9381844211,0,lc};
Point(29) = {200,-24.9772237201,0,lc};
Point(30) = {210,-24.9916162467,0,lc};
Point(31) = {220,-24.9969151356,0,lc};
//Point(32) = {230,-24.9988650533,0,lc};
//Point(33) = {30,-70.0011349467,0,lc};
Point(34) = {40,-70.0030848644,0,lc};
Point(35) = {50,-70.0083837533,0,lc};
Point(36) = {60,-70.0227762799,0,lc};
Point(37) = {70,-70.0618155789,0,lc};
Point(38) = {80,-70.1673212731,0,lc};
Point(39) = {90,-70.4496552491,0,lc};
Point(40) = {100,-71.1856468294,0,lc};
Point(41) = {110,-72.9800730506,0,lc};
Point(42) = {120,-76.7235355342,0,lc};
Point(43) = {130,-82.5,0,lc};
Point(44) = {140,-88.2764644658,0,lc};
Point(45) = {150,-92.0199269494,0,lc};
Point(46) = {160,-93.8143531706,0,lc};
Point(47) = {170,-94.5503447509,0,lc};
Point(48) = {180,-94.8326787269,0,lc};
Point(49) = {190,-94.9381844211,0,lc};
Point(50) = {200,-94.9772237201,0,lc};
Point(51) = {210,-94.9916162467,0,lc};
Point(52) = {220,-94.9969151356,0,lc};
//Point(53) = {230,-94.9988650533,0,lc};

// Omkadering van domein
Line(1) = {1,6};
Line(2) = {9,2};
Line(3) = {8,3};
Line(4) = {4,11};
Line(5)= {2,1};
Line(6)= {3,4};
// Splines voor geplooide deel
Spline(7) = {6,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,8};

Spline(8) = {11,52,51,50,49,48,47,46,45,44,43,42,41,40,39,38,37,36,35,34,9};

// Line Loop om omkadering te maken
Line Loop(9) = {1,7,3,6,4,8,2,5};
// Definieren van dit domein als oppervlak
Plane Surface(10) = {9};
Physical Surface("interior") = {10};

// 
Physical Line("inlet") = {5};
Physical Line("outlet") = {6};
Physical Line("top") = {1,7,3}; // 7 van Spline
Physical Line("bottom") = {4,8,2}; // 8 van Spline



lc2 = lc*2;
// Punten Middelste deel

Point(58) = {30,-35.0011349467,0,lc2};
Point{58} In Surface{10}; 
Point(59) = {40,-35.0030848644,0,lc2};
Point{59} In Surface{10}; 
Point(60) = {50,-35.0083837533,0,lc2};
Point{60} In Surface{10}; 
Point(61) = {60,-35.0227762799,0,lc2};
Point{61} In Surface{10}; 
Point(62) = {70,-35.0618155789,0,lc2};
Point{62} In Surface{10}; 
Point(63) = {80,-35.1673212731,0,lc2};
Point{63} In Surface{10}; 
Point(64) = {90,-35.4496552491,0,lc2};
Point{64} In Surface{10}; 
Point(65) = {100,-36.1856468294,0,lc2};
Point{65} In Surface{10}; 
Point(66) = {110,-37.9800730506,0,lc2};
Point{66} In Surface{10}; 
Point(67) = {120,-41.7235355342,0,lc2};
Point{67} In Surface{10}; 
Point(68) = {130,-47.5,0,lc2};
Point{68} In Surface{10}; 
Point(69) = {140,-53.2764644658,0,lc2};
Point{69} In Surface{10}; 
Point(70) = {150,-57.0199269494,0,lc2};
Point{70} In Surface{10}; 
Point(71) = {160,-58.8143531706,0,lc2};
Point{71} In Surface{10}; 
Point(72) = {170,-59.5503447509,0,lc2};
Point{72} In Surface{10}; 
Point(73) = {180,-59.8326787269,0,lc2};
Point{73} In Surface{10}; 
Point(74) = {190,-59.9381844211,0,lc2};
Point{74} In Surface{10}; 
Point(75) = {200,-59.9772237201,0,lc2};
Point{75} In Surface{10}; 
Point(76) = {210,-59.9916162467,0,lc2};
Point{76} In Surface{10}; 
Point(77) = {220,-59.9969151356,0,lc2};
Point{77} In Surface{10}; 
Point(78) = {230,-59.9988650533,0,lc2};
Point{78} In Surface{10}; 

// Rechte stukken links en rechts centraal gelegen
Point(79) = {20,-35,0,lc2};
Point{79} In Surface{10}; 
Point(80) = {10,-35,0,lc2};
Point{80} In Surface{10}; 
Point(81) = {0,-35,0,lc2};
Point{81} In Surface{10}; 
Point(82) = {-10,-35,0,lc2};
Point{82} In Surface{10}; 
Point(83) = {-20,-35,0,lc2};
Point{83} In Surface{10}; 
Point(95) = {-30,-35,0,lc2};
Point{95} In Surface{10}; 
Point(96) = {-40,-35,0,lc2};
Point{96} In Surface{10}; 

Point(84) = {240,-60,0,lc2};
Point{84} In Surface{10}; 
Point(85) = {250,-60,0,lc2};
Point{85} In Surface{10}; 
Point(86) = {260,-60,0,lc2};
Point{86} In Surface{10}; 
Point(87) = {270,-60,0,lc2};
Point{87} In Surface{10}; 
Point(88) = {280,-60,0,lc2};
Point{88} In Surface{10}; 
Point(90) = {290,-60,0,lc2};
Point{90} In Surface{10}; 

















