/*------------------------------------------------------------------------
 *
 * Copyright (c) 1997-1998 by Cornell University.
 * 
 * See the file "license.txt" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 *------------------------------------------------------------------------
 */
/*
 * sqr(x) = theSquareTable(x + 255)
 */
unsigned short theSquareTable[511] = {
  65025,   64516,   64009,   63504, 
  63001,   62500,   62001,   61504,   61009,   60516, 
  60025,   59536,   59049,   58564,   58081,   57600, 
  57121,   56644,   56169,   55696,   55225,   54756, 
  54289,   53824,   53361,   52900,   52441,   51984, 
  51529,   51076,   50625,   50176,   49729,   49284, 
  48841,   48400,   47961,   47524,   47089,   46656, 
  46225,   45796,   45369,   44944,   44521,   44100, 
  43681,   43264,   42849,   42436,   42025,   41616, 
  41209,   40804,   40401,   40000,   39601,   39204, 
  38809,   38416,   38025,   37636,   37249,   36864, 
  36481,   36100,   35721,   35344,   34969,   34596, 
  34225,   33856,   33489,   33124,   32761,   32400, 
  32041,   31684,   31329,   30976,   30625,   30276, 
  29929,   29584,   29241,   28900,   28561,   28224, 
  27889,   27556,   27225,   26896,   26569,   26244, 
  25921,   25600,   25281,   24964,   24649,   24336, 
  24025,   23716,   23409,   23104,   22801,   22500, 
  22201,   21904,   21609,   21316,   21025,   20736, 
  20449,   20164,   19881,   19600,   19321,   19044, 
  18769,   18496,   18225,   17956,   17689,   17424, 
  17161,   16900,   16641,   16384,   16129,   15876, 
  15625,   15376,   15129,   14884,   14641,   14400, 
  14161,   13924,   13689,   13456,   13225,   12996, 
  12769,   12544,   12321,   12100,   11881,   11664, 
  11449,   11236,   11025,   10816,   10609,   10404, 
  10201,   10000,    9801,    9604,    9409,    9216, 
   9025,    8836,    8649,    8464,    8281,    8100, 
   7921,    7744,    7569,    7396,    7225,    7056, 
   6889,    6724,    6561,    6400,    6241,    6084, 
   5929,    5776,    5625,    5476,    5329,    5184, 
   5041,    4900,    4761,    4624,    4489,    4356, 
   4225,    4096,    3969,    3844,    3721,    3600, 
   3481,    3364,    3249,    3136,    3025,    2916, 
   2809,    2704,    2601,    2500,    2401,    2304, 
   2209,    2116,    2025,    1936,    1849,    1764, 
   1681,    1600,    1521,    1444,    1369,    1296, 
   1225,    1156,    1089,    1024,     961,     900, 
    841,     784,     729,     676,     625,     576, 
    529,     484,     441,     400,     361,     324, 
    289,     256,     225,     196,     169,     144, 
    121,     100,      81,      64,      49,      36, 
     25,      16,       9,       4,       1,       0, 
      1,       4,       9,      16,      25,      36, 
     49,      64,      81,     100,     121,     144, 
    169,     196,     225,     256,     289,     324, 
    361,     400,     441,     484,     529,     576, 
    625,     676,     729,     784,     841,     900, 
    961,    1024,    1089,    1156,    1225,    1296, 
   1369,    1444,    1521,    1600,    1681,    1764, 
   1849,    1936,    2025,    2116,    2209,    2304, 
   2401,    2500,    2601,    2704,    2809,    2916, 
   3025,    3136,    3249,    3364,    3481,    3600, 
   3721,    3844,    3969,    4096,    4225,    4356, 
   4489,    4624,    4761,    4900,    5041,    5184, 
   5329,    5476,    5625,    5776,    5929,    6084, 
   6241,    6400,    6561,    6724,    6889,    7056, 
   7225,    7396,    7569,    7744,    7921,    8100, 
   8281,    8464,    8649,    8836,    9025,    9216, 
   9409,    9604,    9801,   10000,   10201,   10404, 
  10609,   10816,   11025,   11236,   11449,   11664, 
  11881,   12100,   12321,   12544,   12769,   12996, 
  13225,   13456,   13689,   13924,   14161,   14400, 
  14641,   14884,   15129,   15376,   15625,   15876, 
  16129,   16384,   16641,   16900,   17161,   17424, 
  17689,   17956,   18225,   18496,   18769,   19044, 
  19321,   19600,   19881,   20164,   20449,   20736, 
  21025,   21316,   21609,   21904,   22201,   22500, 
  22801,   23104,   23409,   23716,   24025,   24336, 
  24649,   24964,   25281,   25600,   25921,   26244, 
  26569,   26896,   27225,   27556,   27889,   28224, 
  28561,   28900,   29241,   29584,   29929,   30276, 
  30625,   30976,   31329,   31684,   32041,   32400, 
  32761,   33124,   33489,   33856,   34225,   34596, 
  34969,   35344,   35721,   36100,   36481,   36864, 
  37249,   37636,   38025,   38416,   38809,   39204, 
  39601,   40000,   40401,   40804,   41209,   41616, 
  42025,   42436,   42849,   43264,   43681,   44100, 
  44521,   44944,   45369,   45796,   46225,   46656, 
  47089,   47524,   47961,   48400,   48841,   49284, 
  49729,   50176,   50625,   51076,   51529,   51984, 
  52441,   52900,   53361,   53824,   54289,   54756, 
  55225,   55696,   56169,   56644,   57121,   57600, 
  58081,   58564,   59049,   59536,   60025,   60516, 
  61009,   61504,   62001,   62500,   63001,   63504, 
  64009,   64516,   65025 };

int table33023[256] = {
      0,  33023,  66046,  99069, 132092, 165115, 198138, 231161,
 264184, 297207, 330230, 363253, 396276, 429299, 462322, 495345,
 528368, 561391, 594414, 627437, 660460, 693483, 726506, 759529,
 792552, 825575, 858598, 891621, 924644, 957667, 990690,1023713,
1056736,1089759,1122782,1155805,1188828,1221851,1254874,1287897,
1320920,1353943,1386966,1419989,1453012,1486035,1519058,1552081,
1585104,1618127,1651150,1684173,1717196,1750219,1783242,1816265,
1849288,1882311,1915334,1948357,1981380,2014403,2047426,2080449,
2113472,2146495,2179518,2212541,2245564,2278587,2311610,2344633,
2377656,2410679,2443702,2476725,2509748,2542771,2575794,2608817,
2641840,2674863,2707886,2740909,2773932,2806955,2839978,2873001,
2906024,2939047,2972070,3005093,3038116,3071139,3104162,3137185,
3170208,3203231,3236254,3269277,3302300,3335323,3368346,3401369,
3434392,3467415,3500438,3533461,3566484,3599507,3632530,3665553,
3698576,3731599,3764622,3797645,3830668,3863691,3896714,3929737,
3962760,3995783,4028806,4061829,4094852,4127875,4160898,4193921,
4226944,4259967,4292990,4326013,4359036,4392059,4425082,4458105,
4491128,4524151,4557174,4590197,4623220,4656243,4689266,4722289,
4755312,4788335,4821358,4854381,4887404,4920427,4953450,4986473,
5019496,5052519,5085542,5118565,5151588,5184611,5217634,5250657,
5283680,5316703,5349726,5382749,5415772,5448795,5481818,5514841,
5547864,5580887,5613910,5646933,5679956,5712979,5746002,5779025,
5812048,5845071,5878094,5911117,5944140,5977163,6010186,6043209,
6076232,6109255,6142278,6175301,6208324,6241347,6274370,6307393,
6340416,6373439,6406462,6439485,6472508,6505531,6538554,6571577,
6604600,6637623,6670646,6703669,6736692,6769715,6802738,6835761,
6868784,6901807,6934830,6967853,7000876,7033899,7066922,7099945,
7132968,7165991,7199014,7232037,7265060,7298083,7331106,7364129,
7397152,7430175,7463198,7496221,7529244,7562267,7595290,7628313,
7661336,7694359,7727382,7760405,7793428,7826451,7859474,7892497,
7925520,7958543,7991566,8024589,8057612,8090635,8123658,8156681,
8189704,8222727,8255750,8288773,8321796,8354819,8387842,};

int table30013[256] = {
      0,  30013,  60026,  90039, 120052, 150065, 180078, 210091,
 240104, 270117, 300130, 330143, 360156, 390169, 420182, 450195,
 480208, 510221, 540234, 570247, 600260, 630273, 660286, 690299,
 720312, 750325, 780338, 810351, 840364, 870377, 900390, 930403,
 960416, 990429,1020442,1050455,1080468,1110481,1140494,1170507,
1200520,1230533,1260546,1290559,1320572,1350585,1380598,1410611,
1440624,1470637,1500650,1530663,1560676,1590689,1620702,1650715,
1680728,1710741,1740754,1770767,1800780,1830793,1860806,1890819,
1920832,1950845,1980858,2010871,2040884,2070897,2100910,2130923,
2160936,2190949,2220962,2250975,2280988,2311001,2341014,2371027,
2401040,2431053,2461066,2491079,2521092,2551105,2581118,2611131,
2641144,2671157,2701170,2731183,2761196,2791209,2821222,2851235,
2881248,2911261,2941274,2971287,3001300,3031313,3061326,3091339,
3121352,3151365,3181378,3211391,3241404,3271417,3301430,3331443,
3361456,3391469,3421482,3451495,3481508,3511521,3541534,3571547,
3601560,3631573,3661586,3691599,3721612,3751625,3781638,3811651,
3841664,3871677,3901690,3931703,3961716,3991729,4021742,4051755,
4081768,4111781,4141794,4171807,4201820,4231833,4261846,4291859,
4321872,4351885,4381898,4411911,4441924,4471937,4501950,4531963,
4561976,4591989,4622002,4652015,4682028,4712041,4742054,4772067,
4802080,4832093,4862106,4892119,4922132,4952145,4982158,5012171,
5042184,5072197,5102210,5132223,5162236,5192249,5222262,5252275,
5282288,5312301,5342314,5372327,5402340,5432353,5462366,5492379,
5522392,5552405,5582418,5612431,5642444,5672457,5702470,5732483,
5762496,5792509,5822522,5852535,5882548,5912561,5942574,5972587,
6002600,6032613,6062626,6092639,6122652,6152665,6182678,6212691,
6242704,6272717,6302730,6332743,6362756,6392769,6422782,6452795,
6482808,6512821,6542834,6572847,6602860,6632873,6662886,6692899,
6722912,6752925,6782938,6812951,6842964,6872977,6902990,6933003,
6963016,6993029,7023042,7053055,7083068,7113081,7143094,7173107,
7203120,7233133,7263146,7293159,7323172,7353185,7383198,7413211,
7443224,7473237,7503250,7533263,7563276,7593289,7623302,};

int table27011[256] = {
      0,  27011,  54022,  81033, 108044, 135055, 162066, 189077,
 216088, 243099, 270110, 297121, 324132, 351143, 378154, 405165,
 432176, 459187, 486198, 513209, 540220, 567231, 594242, 621253,
 648264, 675275, 702286, 729297, 756308, 783319, 810330, 837341,
 864352, 891363, 918374, 945385, 972396, 999407,1026418,1053429,
1080440,1107451,1134462,1161473,1188484,1215495,1242506,1269517,
1296528,1323539,1350550,1377561,1404572,1431583,1458594,1485605,
1512616,1539627,1566638,1593649,1620660,1647671,1674682,1701693,
1728704,1755715,1782726,1809737,1836748,1863759,1890770,1917781,
1944792,1971803,1998814,2025825,2052836,2079847,2106858,2133869,
2160880,2187891,2214902,2241913,2268924,2295935,2322946,2349957,
2376968,2403979,2430990,2458001,2485012,2512023,2539034,2566045,
2593056,2620067,2647078,2674089,2701100,2728111,2755122,2782133,
2809144,2836155,2863166,2890177,2917188,2944199,2971210,2998221,
3025232,3052243,3079254,3106265,3133276,3160287,3187298,3214309,
3241320,3268331,3295342,3322353,3349364,3376375,3403386,3430397,
3457408,3484419,3511430,3538441,3565452,3592463,3619474,3646485,
3673496,3700507,3727518,3754529,3781540,3808551,3835562,3862573,
3889584,3916595,3943606,3970617,3997628,4024639,4051650,4078661,
4105672,4132683,4159694,4186705,4213716,4240727,4267738,4294749,
4321760,4348771,4375782,4402793,4429804,4456815,4483826,4510837,
4537848,4564859,4591870,4618881,4645892,4672903,4699914,4726925,
4753936,4780947,4807958,4834969,4861980,4888991,4916002,4943013,
4970024,4997035,5024046,5051057,5078068,5105079,5132090,5159101,
5186112,5213123,5240134,5267145,5294156,5321167,5348178,5375189,
5402200,5429211,5456222,5483233,5510244,5537255,5564266,5591277,
5618288,5645299,5672310,5699321,5726332,5753343,5780354,5807365,
5834376,5861387,5888398,5915409,5942420,5969431,5996442,6023453,
6050464,6077475,6104486,6131497,6158508,6185519,6212530,6239541,
6266552,6293563,6320574,6347585,6374596,6401607,6428618,6455629,
6482640,6509651,6536662,6563673,6590684,6617695,6644706,6671717,
6698728,6725739,6752750,6779761,6806772,6833783,6860794,};
