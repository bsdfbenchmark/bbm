#ifndef _BBM_PRECOMPUTE_HOLZSCHUCHPACANOWSKI_DISTRIBUTION_NORMALIZATION_H_
#define _BBM_PRECOMPUTE_HOLZSCHUCHPACANOWSKI_DISTRIBUTION_NORMALIZATION_H_

/************************************************************************/
/* Precomputed EPD normalization (without beta) from 'A two-scale       */
/* microfacet reflectance model combining reflection and diffraction',  */
/* Holzschuch and Pacanowski [2017]:  doi.org/10.1145/3072959.3073621   */
/************************************************************************/

namespace bbm {                                                           
  namespace precomputed {                                                 
    namespace holzschuchpacanowski {                                      

      static const tab<float, std::array{256},                                   
           decltype( [](const auto& p) { return p * 256.0 / 5.0; } )      
         > distributionNormalization = {                                  
        0, 9.32554e-68, 2.91892e-27, 7.39356e-16, 8.59034e-11, 4.97486e-08, 2.47968e-06, 3.3303e-05, 
        0.000206516, 0.000785737, 0.00215869, 0.00472973, 0.00880765, 0.0145448, 0.0219333, 0.030837, 
        0.0410368, 0.0522724, 0.0642754, 0.0767913, 0.0895925, 0.102484, 0.115304, 0.127924, 
        0.140245, 0.15219, 0.163708, 0.174761, 0.185329, 0.1954, 0.204974, 0.214055, 
        0.222653, 0.230782, 0.238459, 0.245702, 0.25253, 0.258963, 0.265021, 0.270724, 
        0.27609, 0.28114, 0.28589, 0.290358, 0.294562, 0.298516, 0.302235, 0.305734, 
        0.309026, 0.312124, 0.315038, 0.317781, 0.320363, 0.322792, 0.32508, 0.327233, 
        0.329261, 0.331171, 0.332969, 0.334662, 0.336257, 0.337759, 0.339173, 0.340505, 
        0.34176, 0.342942, 0.344055, 0.345102, 0.346089, 0.347017, 0.347891, 0.348714, 
        0.349487, 0.350215, 0.350899, 0.351542, 0.352146, 0.352713, 0.353245, 0.353744, 
        0.354212, 0.35465, 0.355061, 0.355445, 0.355803, 0.356138, 0.35645, 0.356741, 
        0.357011, 0.357262, 0.357494, 0.35771, 0.357908, 0.358091, 0.358259, 0.358413, 
        0.358553, 0.358681, 0.358796, 0.3589, 0.358993, 0.359075, 0.359148, 0.359211, 
        0.359265, 0.359311, 0.359348, 0.359378, 0.3594, 0.359416, 0.359425, 0.359427, 
        0.359424, 0.359415, 0.3594, 0.35938, 0.359355, 0.359326, 0.359292, 0.359254, 
        0.359212, 0.359167, 0.359117, 0.359064, 0.359008, 0.358949, 0.358887, 0.358822, 
        0.358754, 0.358684, 0.358612, 0.358537, 0.358461, 0.358382, 0.358301, 0.358218, 
        0.358134, 0.358048, 0.357961, 0.357872, 0.357782, 0.357691, 0.357598, 0.357504, 
        0.35741, 0.357314, 0.357217, 0.35712, 0.357021, 0.356922, 0.356822, 0.356722, 
        0.356621, 0.356519, 0.356417, 0.356315, 0.356212, 0.356108, 0.356005, 0.355901, 
        0.355796, 0.355692, 0.355587, 0.355482, 0.355377, 0.355272, 0.355167, 0.355061, 
        0.354956, 0.35485, 0.354745, 0.354639, 0.354534, 0.354428, 0.354323, 0.354218, 
        0.354113, 0.354008, 0.353903, 0.353798, 0.353693, 0.353589, 0.353485, 0.353381, 
        0.353277, 0.353173, 0.35307, 0.352967, 0.352864, 0.352761, 0.352659, 0.352557, 
        0.352455, 0.352353, 0.352252, 0.352151, 0.352051, 0.35195, 0.35185, 0.351751, 
        0.351651, 0.351552, 0.351454, 0.351355, 0.351258, 0.35116, 0.351063, 0.350966, 
        0.350869, 0.350773, 0.350677, 0.350582, 0.350487, 0.350392, 0.350298, 0.350204, 
        0.35011, 0.350017, 0.349924, 0.349832, 0.34974, 0.349648, 0.349557, 0.349466, 
        0.349375, 0.349285, 0.349195, 0.349106, 0.349017, 0.348928, 0.34884, 0.348752, 
        0.348665, 0.348578, 0.348491, 0.348405, 0.348319, 0.348233, 0.348148, 0.348063, 
        0.347979, 0.347895, 0.347811, 0.347728, 0.347645, 0.347562, 0.34748, 0.347398, 
        0.347317, 0.347236, 0.347155, 0.347075, 0.346995, 0.346916, 0.346836, 0.346758
      };                                                                  
    } // end holzschuchpacanowski namespace                               
  } // end precomputed namespace                                          
} // end bbm namespace                                                    

#endif /* _BBM_PRECOMPUTE_HOLZSCHUCHPACANOWSKI_DISTRIBUTION_NORMALIZATION_H_ */
