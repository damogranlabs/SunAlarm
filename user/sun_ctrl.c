
#include <stdbool.h>

#include "sun_ctrl.h"
#include "logic.h"

extern configuration_t cfg_data;

uint16_t _scale_intensity_to_lut(uint32_t intensity);

uint8_t _last_user_sun_intensity;

// clang-format off
// Gamma brightness lookup table <https://victornpb.github.io/gamma-table-generator>
// gamma = 3.30 steps = 1500 range = 0-65535
#define LUT_SIZE 1500
const uint16_t gamma_lut[LUT_SIZE] = {
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1,   1,   1,   1,
     1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   2,   2,   2,   2,   2,
     2,   2,   2,   2,   2,   3,   3,   3,   3,   3,   3,   3,   3,   4,   4,   4,
     4,   4,   4,   5,   5,   5,   5,   5,   6,   6,   6,   6,   7,   7,   7,   7,
     8,   8,   8,   8,   9,   9,   9,  10,  10,  10,  10,  11,  11,  11,  12,  12,
    13,  13,  13,  14,  14,  14,  15,  15,  16,  16,  17,  17,  18,  18,  19,  19,
    20,  20,  21,  21,  22,  22,  23,  23,  24,  24,  25,  26,  26,  27,  27,  28,
    29,  29,  30,  31,  31,  32,  33,  34,  34,  35,  36,  37,  37,  38,  39,  40,
    41,  42,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,
    56,  57,  58,  59,  60,  61,  62,  63,  65,  66,  67,  68,  69,  71,  72,  73,
    74,  76,  77,  78,  80,  81,  82,  84,  85,  86,  88,  89,  91,  92,  94,  95,
    97,  98, 100, 101, 103, 105, 106, 108, 110, 111, 113, 115, 116, 118, 120, 122,
   124, 125, 127, 129, 131, 133, 135, 137, 139, 141, 143, 145, 147, 149, 151, 153,
   155, 157, 160, 162, 164, 166, 168, 171, 173, 175, 178, 180, 182, 185, 187, 190,
   192, 195, 197, 200, 202, 205, 207, 210, 213, 215, 218, 221, 223, 226, 229, 232,
   235, 238, 240, 243, 246, 249, 252, 255, 258, 261, 264, 267, 271, 274, 277, 280,
   283, 287, 290, 293, 297, 300, 303, 307, 310, 314, 317, 321, 324, 328, 331, 335,
   339, 342, 346, 350, 354, 357, 361, 365, 369, 373, 377, 381, 385, 389, 393, 397,
   401, 405, 409, 414, 418, 422, 427, 431, 435, 440, 444, 448, 453, 458, 462, 467,
   471, 476, 481, 485, 490, 495, 500, 504, 509, 514, 519, 524, 529, 534, 539, 544,
   549, 555, 560, 565, 570, 576, 581, 586, 592, 597, 603, 608, 614, 619, 625, 631,
   636, 642, 648, 654, 659, 665, 671, 677, 683, 689, 695, 701, 707, 713, 720, 726,
   732, 738, 745, 751, 758, 764, 771, 777, 784, 790, 797, 804, 810, 817, 824, 831,
   838, 845, 852, 859, 866, 873, 880, 887, 894, 902, 909, 916, 924, 931, 938, 946,
   954, 961, 969, 976, 984, 992,1000,1008,1015,1023,1031,1039,1047,1055,1064,1072,
  1080,1088,1097,1105,1113,1122,1130,1139,1147,1156,1165,1173,1182,1191,1200,1209,
  1218,1227,1236,1245,1254,1263,1272,1282,1291,1300,1310,1319,1329,1338,1348,1358,
  1367,1377,1387,1397,1406,1416,1426,1436,1447,1457,1467,1477,1487,1498,1508,1519,
  1529,1540,1550,1561,1572,1582,1593,1604,1615,1626,1637,1648,1659,1670,1681,1692,
  1704,1715,1727,1738,1750,1761,1773,1784,1796,1808,1820,1832,1844,1856,1868,1880,
  1892,1904,1916,1929,1941,1954,1966,1979,1991,2004,2017,2029,2042,2055,2068,2081,
  2094,2107,2121,2134,2147,2160,2174,2187,2201,2214,2228,2242,2255,2269,2283,2297,
  2311,2325,2339,2353,2368,2382,2396,2411,2425,2440,2454,2469,2484,2498,2513,2528,
  2543,2558,2573,2588,2603,2619,2634,2649,2665,2680,2696,2712,2727,2743,2759,2775,
  2791,2807,2823,2839,2855,2872,2888,2904,2921,2937,2954,2971,2987,3004,3021,3038,
  3055,3072,3089,3106,3123,3141,3158,3176,3193,3211,3228,3246,3264,3282,3300,3318,
  3336,3354,3372,3391,3409,3427,3446,3464,3483,3502,3520,3539,3558,3577,3596,3615,
  3634,3654,3673,3692,3712,3731,3751,3771,3790,3810,3830,3850,3870,3890,3911,3931,
  3951,3972,3992,4013,4033,4054,4075,4096,4116,4137,4159,4180,4201,4222,4244,4265,
  4287,4308,4330,4352,4373,4395,4417,4439,4462,4484,4506,4528,4551,4573,4596,4619,
  4641,4664,4687,4710,4733,4756,4780,4803,4826,4850,4873,4897,4921,4944,4968,4992,
  5016,5040,5064,5089,5113,5137,5162,5187,5211,5236,5261,5286,5311,5336,5361,5386,
  5412,5437,5462,5488,5514,5539,5565,5591,5617,5643,5669,5696,5722,5748,5775,5801,
  5828,5855,5882,5909,5936,5963,5990,6017,6045,6072,6099,6127,6155,6183,6210,6238,
  6266,6295,6323,6351,6380,6408,6437,6465,6494,6523,6552,6581,6610,6639,6669,6698,
  6727,6757,6787,6816,6846,6876,6906,6936,6966,6997,7027,7058,7088,7119,7150,7180,
  7211,7242,7274,7305,7336,7368,7399,7431,7462,7494,7526,7558,7590,7622,7654,7687,
  7719,7752,7784,7817,7850,7883,7916,7949,7982,8015,8049,8082,8116,8150,8183,8217,
  8251,8285,8320,8354,8388,8423,8457,8492,8527,8562,8597,8632,8667,8702,8738,8773,
  8809,8844,8880,8916,8952,8988,9024,9060,9097,9133,9170,9207,9243,9280,9317,9354,
  9391,9429,9466,9504,9541,9579,9617,9655,9693,9731,9769,9808,9846,9885,9923,9962,
  10001,10040,10079,10118,10157,10197,10236,10276,10316,10355,10395,10435,10475,10516,10556,10597,
  10637,10678,10719,10759,10800,10842,10883,10924,10966,11007,11049,11091,11132,11174,11217,11259,
  11301,11344,11386,11429,11472,11514,11557,11600,11644,11687,11730,11774,11818,11861,11905,11949,
  11993,12038,12082,12126,12171,12216,12261,12305,12350,12396,12441,12486,12532,12577,12623,12669,
  12715,12761,12807,12853,12900,12946,12993,13040,13087,13134,13181,13228,13275,13323,13370,13418,
  13466,13514,13562,13610,13658,13707,13755,13804,13853,13902,13951,14000,14049,14099,14148,14198,
  14247,14297,14347,14397,14448,14498,14548,14599,14650,14701,14752,14803,14854,14905,14957,15008,
  15060,15112,15164,15216,15268,15320,15373,15425,15478,15531,15584,15637,15690,15743,15797,15851,
  15904,15958,16012,16066,16120,16175,16229,16284,16338,16393,16448,16503,16559,16614,16670,16725,
  16781,16837,16893,16949,17005,17062,17118,17175,17232,17289,17346,17403,17460,17518,17575,17633,
  17691,17749,17807,17865,17924,17982,18041,18099,18158,18217,18277,18336,18395,18455,18515,18575,
  18635,18695,18755,18815,18876,18936,18997,19058,19119,19180,19242,19303,19365,19427,19489,19551,
  19613,19675,19737,19800,19863,19926,19989,20052,20115,20178,20242,20306,20369,20433,20498,20562,
  20626,20691,20755,20820,20885,20950,21015,21081,21146,21212,21278,21344,21410,21476,21542,21609,
  21676,21742,21809,21876,21944,22011,22079,22146,22214,22282,22350,22418,22487,22555,22624,22693,
  22762,22831,22900,22969,23039,23109,23178,23248,23319,23389,23459,23530,23601,23672,23743,23814,
  23885,23957,24028,24100,24172,24244,24316,24388,24461,24534,24606,24679,24753,24826,24899,24973,
  25047,25120,25194,25269,25343,25417,25492,25567,25642,25717,25792,25868,25943,26019,26095,26171,
  26247,26323,26400,26476,26553,26630,26707,26784,26862,26939,27017,27095,27173,27251,27329,27408,
  27487,27565,27644,27724,27803,27882,27962,28042,28122,28202,28282,28362,28443,28524,28604,28685,
  28767,28848,28930,29011,29093,29175,29257,29340,29422,29505,29587,29670,29754,29837,29920,30004,
  30088,30172,30256,30340,30424,30509,30594,30679,30764,30849,30934,31020,31106,31192,31278,31364,
  31450,31537,31624,31711,31798,31885,31972,32060,32148,32236,32324,32412,32500,32589,32678,32766,
  32856,32945,33034,33124,33214,33303,33394,33484,33574,33665,33756,33847,33938,34029,34120,34212,
  34304,34396,34488,34580,34673,34765,34858,34951,35044,35138,35231,35325,35419,35513,35607,35702,
  35796,35891,35986,36081,36176,36272,36367,36463,36559,36655,36751,36848,36945,37041,37138,37236,
  37333,37431,37528,37626,37724,37823,37921,38020,38118,38217,38317,38416,38515,38615,38715,38815,
  38915,39016,39116,39217,39318,39419,39521,39622,39724,39826,39928,40030,40132,40235,40338,40441,
  40544,40647,40751,40854,40958,41062,41167,41271,41376,41480,41585,41691,41796,41901,42007,42113,
  42219,42325,42432,42539,42645,42752,42860,42967,43075,43182,43290,43399,43507,43615,43724,43833,
  43942,44051,44161,44271,44380,44491,44601,44711,44822,44933,45044,45155,45266,45378,45490,45601,
  45714,45826,45938,46051,46164,46277,46391,46504,46618,46732,46846,46960,47074,47189,47304,47419,
  47534,47650,47765,47881,47997,48113,48230,48346,48463,48580,48697,48815,48932,49050,49168,49286,
  49405,49523,49642,49761,49880,50000,50119,50239,50359,50479,50600,50720,50841,50962,51083,51204,
  51326,51448,51570,51692,51814,51937,52060,52183,52306,52429,52553,52677,52801,52925,53049,53174,
  53299,53424,53549,53674,53800,53926,54052,54178,54305,54431,54558,54685,54812,54940,55068,55196,
  55324,55452,55580,55709,55838,55967,56097,56226,56356,56486,56616,56747,56877,57008,57139,57270,
  57402,57533,57665,57797,57929,58062,58195,58328,58461,58594,58728,58861,58995,59130,59264,59399,
  59533,59668,59804,59939,60075,60211,60347,60483,60620,60756,60893,61031,61168,61306,61443,61581,
  61720,61858,61997,62136,62275,62414,62554,62694,62834,62974,63114,63255,63396,63537,63678,63820,
  63961,64103,64245,64388,64531,64673,64816,64960,65103,65247,65391,65535,
};
// clang-format on

void sun_init(void)
{
  LL_TIM_SetCounter(SUN_TIM, 0);
  sun_set_intensity(cfg_data.sun_manual_intensity);
  LL_TIM_EnableAllOutputs(SUN_TIM);
}

void sun_pwr_on(void)
{
  LL_TIM_CC_EnableChannel(SUN_TIM, SUN_TIM_CH);
  LL_TIM_EnableCounter(SUN_TIM);
}

void sun_pwr_off(void)
{
  LL_TIM_DisableCounter(SUN_TIM);
  LL_TIM_CC_DisableChannel(SUN_TIM, SUN_TIM_CH);
}

void sun_pwr_on_manual(void)
{
  sun_set_intensity(cfg_data.sun_manual_intensity);
  sun_pwr_on();
}

void sun_set_intensity(uint8_t intensity)
{
  // percent to value conversion (up to SUN_INTENSITY_MAX)
  _last_user_sun_intensity = intensity;
  uint32_t compare_val = get_sun_intensity_value(intensity);

  LL_TIM_OC_SetCompareCH4(SUN_TIM, _scale_intensity_to_lut(compare_val));
}

uint8_t sun_get_intensity(void)
{
  // return last intensity that was set via `set_sun_intensity()`
  return _last_user_sun_intensity;
}

uint16_t _scale_intensity_to_lut(uint32_t intensity)
{
  // scale intensity range to length of LUT
  uint32_t idx_in_lut = (intensity * LUT_SIZE) / cfg_data.intensity_resolution;

  return gamma_lut[idx_in_lut - 1];
}

void sun_set_intensity_precise(uint32_t intensity)
{
  LL_TIM_OC_SetCompareCH4(SUN_TIM, _scale_intensity_to_lut(intensity));
}

uint32_t get_sun_intensity_value(uint8_t user_intensity)
{
  return (cfg_data.intensity_resolution * (uint32_t)user_intensity) / SUN_INTENSITY_MAX;
}
