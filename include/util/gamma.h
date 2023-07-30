#ifndef _BBM_GAMMA_H_
#define _BBM_GAMMA_H_

#include <numeric>
#include "util/poly.h"
#include "util/named.h"


/************************************************************************/
/*! \file gamma.h

  \brief Packet-type friendly implementation of:
    + tgamma_lower : lower unnormalized incomplete gamma
    + tgamma       : upper unnormalized incomplete gamma
    + gamma_p      : lower nornalized incomplete gamma
    + gamma_q      : upper normalized incomplete gamma
    + gamma_pq     : upper and lower normalized incomplete gamma

    Follows for (a < 20) "Numerical Recipes in C++, The Art of Scientific
    Computing, 2nd ed.", Press et al., p 221, Sec. 6.2.

    and

    Follow for (a > 20): "The asymptotic expansion of the incomplete gamma
    functions", Temme [1979]: https://doi.org/10.1137/0510071
    
    The accuracy for x < 1.1 is not optimal, and higher accuracy can be
    achieved following the additional steps described at
    https://www.boost.org/doc/libs/1_64_0/libs/math/doc/html/math_toolkit/sf_gamma/igamma.html
    
************************************************************************/

namespace bbm {

  namespace detail {

    /********************************************************************/
    /*! \brief Compute the series approximation of the lower incomplete gamma
        function
      
      \f$ \gamma(a,x) = e^{-x} x^a \sum_m (x^m * \Gamma(a) / \Gamma(a+1+m)) \f$

      Use: \f$ \Gamma(z+1) = z \Gamma(z) \f$
    *********************************************************************/
    template<size_t MaxTerms, bool Normalize, typename TA, typename TX>
      inline auto gamma(const TA& a, const TX& x)
    {
      using result_t = decltype(a + x);
      
      // check if anythings needs to be computed
      auto mask = (x >= 0) && (a > 0);

      // bail out
      if(bbm::none(mask)) return result_t(0);

      result_t ap = a+1;
      result_t sum_series = bbm::select(mask, bbm::rcp(result_t(a)), 0);
      result_t term(sum_series);
      const auto epsilon = std::numeric_limits<scalar_t<result_t>>::epsilon();

      // sum_series = sum_m(term_m)
      //
      // m=0: term_0 = Gamma(a) / Gamma(a+1) * x^0 => inv_a
      // m=1: term_1 = Gamma(a) / Gamma((a+1)+1) * x^1 => term_0 / (a+1) * x
      // m=2: term_2 = Gamma(a) / Gamma((a+2)+1) * x^2 => term_1 / (a+2) * x
      auto converged = !mask;
      for(size_t m=1; m <= MaxTerms && bbm::any(!converged); ++m, ++ap)
      {
        // update term
        term *= bbm::select(converged, 0, x * bbm::rcp(ap));
        
        // update sum
        sum_series += term;

        // update converged
        converged |= bbm::abs(term) < bbm::abs(sum_series)*epsilon;
      }
      
      // normalization requested?
      result_t norm = 0;
      if constexpr (Normalize) norm = bbm::lgamma(result_t(a));
      
      // gamma = (sum_series * e^-x * a^x);
      //       = sum_series * e^(-x + a*ln(x));
      sum_series *= bbm::exp(-x + a*bbm::log(x) - norm);
      
      // Done.
      return bbm::select(mask, sum_series, 0);
    }


    /********************************************************************/
    /*! \brief Computes the Legendre's continued faction development of the
        upper incomplete gamma function:

        \f$ \Gamma(a,x) = e^{-x} x^a ( b_0 + a_1 / (b_1 + a_2 / (b_2 + ... )^-1 \f$

        with \f$ a_k = k(a-k) \f$ and \f$ b_k = x-a+2k+1 \f$
        
    *********************************************************************/
    template<size_t MaxTerms, bool Normalize, typename TA, typename TX>
      inline auto Gamma(const TA& a, const TX& x)
    {
      using result_t = decltype(a + x);
      
      // check if anythings needs to be computed
      auto mask = (x >= 0) && (a > 0);
      
      // bail out
      if(bbm::none(mask)) return result_t(0);

      // set constants
      const auto epsilon = std::numeric_limits<scalar_t<result_t>>::epsilon();
      const auto tiny = std::numeric_limits<scalar_t<result_t>>::min() / epsilon;

      // init 
      result_t b_k = x + 1 - a;
      result_t c = bbm::rcp(tiny);
      result_t d = bbm::rcp(b_k);
      result_t Gamma=d;
      
      auto converged = !mask;
      for(size_t k=1; k < MaxTerms && bbm::any(!converged); ++k)
      {
        // update a_k and b_k
        result_t a_k = k*(a-k);
        b_k += 2;

        // update d and c
        d = b_k + a_k*d;
        d = bbm::select(bbm::abs(d) < tiny, tiny, d);

        c = b_k + a_k/c;
        c = bbm::select(bbm::abs(c) < tiny, tiny, c);

        /// compute delta and update result
        d = bbm::rcp(d);
        result_t delta = c*d;
        Gamma = bbm::select(converged, Gamma, Gamma * delta);
        
        // update converged
        converged |= bbm::abs(delta-1) <= epsilon;
      }
      
      // normalization requested?
      result_t norm = 0;
      if constexpr (Normalize) norm = bbm::lgamma(result_t(a));
      
      // Done.
      return bbm::select(mask, bbm::exp(-x + a*bbm::log(x) - norm) * Gamma, 0);
    }


    /********************************************************************/
    /*! \brief Determines if using gamma_large is appropriate
     ********************************************************************/
    template<typename TA, typename TX>
      inline auto gamma_is_large(const TA& a, const TX& x)
    {
      using result_t = decltype(a+x);
      result_t sigma = bbm::abs((x-a)/a);
      auto result = ((a > 200) && (20/a > sigma*sigma)) || ((a > 20) && (sigma < 0.4) && (std::numeric_limits<scalar_t<result_t>>::digits <= 64));
      return result;
    }
    
    /********************************************************************/
    /*! \brief evaluate the normalized upper incomplete gamma function for
        large values of a (> 20).

      Follows: "The asymptotic expansion of the incomplete gamma functions",
      Temme [1979]: https://doi.org/10.1137/0510071

      with precomputed coefficients from boost/math::special_functions/detail/igamma_large.hpp
      
    ********************************************************************/
    template<bool normalize, typename TA, typename TX>
      inline auto gamma_large(const TA& a , const TX& x)
    {
      using result_t = decltype(a + x);

      // setup
      result_t la = bbm::log(a);
      result_t lx = bbm::log(x);
      result_t phi = x/a - 1 - (lx - la);
      result_t y = x - a*(1 + (lx-la));
      result_t z = bbm::sign(x-a) * bbm::safe_sqrt(2*phi);  // x-a < 0 => x < a => x/a < 1

      auto eval_poly = []<typename T, size_t... IDX>(const T& a, const T& z, std::index_sequence<IDX...>)
      {
        constexpr std::array idx{IDX...};
        
        // compute polynomials
        auto c0 = bbm::poly<idx[0]>(z,
                                    T(-0.333333333333333333333333333333333333),
                                    T(0.0833333333333333333333333333333333333),
                                    T(-0.0148148148148148148148148148148148148),
                                    T(0.00115740740740740740740740740740740741),
                                    T(0.0003527336860670194003527336860670194),
                                    T(-0.000178755144032921810699588477366255144),
                                    T(0.391926317852243778169704095630021556e-4),
                                    T(-0.218544851067999216147364295512443661e-5),
                                    T(-0.185406221071515996070179883622956325e-5),
                                    T(0.829671134095308600501624213166443227e-6),
                                    T(-0.17665952736826079304360054245742403e-6),
                                    T(0.670785354340149858036939710029613572e-8),
                                    T(0.102618097842403080425739573227252951e-7),
                                    T(-0.438203601845335318655297462244719123e-8),
                                    T(0.914769958223679023418248817633113681e-9),
                                    T(-0.255141939949462497668779537993887013e-10),
                                    T(-0.583077213255042506746408945040035798e-10),
                                    T(0.243619480206674162436940696707789943e-10),
                                    T(-0.502766928011417558909054985925744366e-11),
                                    T(0.110043920319561347708374174497293411e-12),
                                    T(0.337176326240098537882769884169200185e-12),
                                    T(-0.13923887224181620659193661848957998e-12),
                                    T(0.285348938070474432039669099052828299e-13),
                                    T(-0.513911183424257261899064580300494205e-15),
                                    T(-0.197522882943494428353962401580710912e-14),
                                    T(0.809952115670456133407115668702575255e-15),
                                    T(-0.165225312163981618191514820265351162e-15),
                                    T(0.253054300974788842327061090060267385e-17),
                                    T(0.116869397385595765888230876507793475e-16),
                                    T(-0.477003704982048475822167804084816597e-17),
                                    T(0.969912605905623712420709685898585354e-18));
        
        auto c1 = bbm::poly<idx[1]>(z,
                                    T(-0.00185185185185185185185185185185185185),
                                    T(-0.00347222222222222222222222222222222222),
                                    T(0.0026455026455026455026455026455026455),
                                    T(-0.000990226337448559670781893004115226337),
                                    T(0.000205761316872427983539094650205761317),
                                    T(-0.401877572016460905349794238683127572e-6),
                                    T(-0.180985503344899778370285914867533523e-4),
                                    T(0.76491609160811100846374214980916921e-5),
                                    T(-0.16120900894563446003775221882217767e-5),
                                    T(0.464712780280743434226135033938722401e-8),
                                    T(0.137863344691572095931187533077488877e-6),
                                    T(-0.575254560351770496402194531835048307e-7),
                                    T(0.119516285997781473243076536699698169e-7),
                                    T(-0.175432417197476476237547551202312502e-10),
                                    T(-0.100915437106004126274577504686681675e-8),
                                    T(0.416279299184258263623372347219858628e-9),
                                    T(-0.856390702649298063807431562579670208e-10),
                                    T(0.606721510160475861512701762169919581e-13),
                                    T(0.716249896481148539007961017165545733e-11),
                                    T(-0.293318664377143711740636683615595403e-11),
                                    T(0.599669636568368872330374527568788909e-12),
                                    T(-0.216717865273233141017100472779701734e-15),
                                    T(-0.497833997236926164052815522048108548e-13),
                                    T(0.202916288237134247736694804325894226e-13),
                                    T(-0.413125571381061004935108332558187111e-14),
                                    T(0.828651623988309644380188591057589316e-18),
                                    T(0.341003088693333279336339355910600992e-15),
                                    T(-0.138541953028939715357034547426313703e-15),
                                    T(0.281234665322887466568860332727259483e-16));
        
        auto c2 = bbm::poly<idx[2]>(z,
                                    T(0.0041335978835978835978835978835978836),
                                    T(-0.00268132716049382716049382716049382716),
                                    T(0.000771604938271604938271604938271604938),
                                    T(0.200938786008230452674897119341563786e-5),
                                    T(-0.000107366532263651605215391223621676297),
                                    T(0.529234488291201254164217127180090143e-4),
                                    T(-0.127606351886187277133779191392360117e-4),
                                    T(0.34235787340961380741902003904747389e-7),
                                    T(0.137219573090629332055943852926020279e-5),
                                    T(-0.629899213838005502290672234278391876e-6),
                                    T(0.142806142060642417915846008822771748e-6),
                                    T(-0.204770984219908660149195854409200226e-9),
                                    T(-0.140925299108675210532930244154315272e-7),
                                    T(0.622897408492202203356394293530327112e-8),
                                    T(-0.136704883966171134992724380284402402e-8),
                                    T(0.942835615901467819547711211663208075e-12),
                                    T(0.128722524000893180595479368872770442e-9),
                                    T(-0.556459561343633211465414765894951439e-10),
                                    T(0.119759355463669810035898150310311343e-10),
                                    T(-0.416897822518386350403836626692480096e-14),
                                    T(-0.109406404278845944099299008640802908e-11),
                                    T(0.4662239946390135746326204922464679e-12),
                                    T(-0.990510576390690597844122258212382301e-13),
                                    T(0.189318767683735145056885183170630169e-16),
                                    T(0.885922187259112726176031067028740667e-14),
                                    T(-0.373782039804640545306560251777191937e-14),
                                    T(0.786883363903515525774088394065960751e-15));
        
        auto c3 = bbm::poly<idx[3]>(z, 
                                    T(0.000649434156378600823045267489711934156),
                                    T(0.000229472093621399176954732510288065844),
                                    T(-0.000469189494395255712128140111679206329),
                                    T(0.000267720632062838852962309752433209223),
                                    T(-0.756180167188397641072538191879755666e-4),
                                    T(-0.239650511386729665193314027333231723e-6),
                                    T(0.110826541153473023614770299726861227e-4),
                                    T(-0.567495282699159656749963105701560205e-5),
                                    T(0.14230900732435883914551894470580433e-5),
                                    T(-0.278610802915281422405802158211174452e-10),
                                    T(-0.16958404091930277289864168795820267e-6),
                                    T(0.809946490538808236335278504852724081e-7),
                                    T(-0.191111684859736540606728140872727635e-7),
                                    T(0.239286204398081179686413514022282056e-11),
                                    T(0.206201318154887984369925818486654549e-8),
                                    T(-0.946049666185513217375417988510192814e-9),
                                    T(0.215410497757749078380130268468744512e-9),
                                    T(-0.138882333681390304603424682490735291e-13),
                                    T(-0.218947616819639394064123400466489455e-10),
                                    T(0.979099895117168512568262802255883368e-11),
                                    T(-0.217821918801809621153859472011393244e-11),
                                    T(0.62088195734079014258166361684972205e-16),
                                    T(0.212697836327973697696702537114614471e-12),
                                    T(-0.934468879151743333127396765626749473e-13),
                                    T(0.204536712267828493249215913063207436e-13));
        
        auto c4 = bbm::poly<idx[4]>(z, 
                                    T(-0.000861888290916711698604702719929057378),
                                    T(0.00078403922172006662747403488144228885),
                                    T(-0.000299072480303190179733389609932819809),
                                    T(-0.146384525788434181781232535690697556e-5),
                                    T(0.664149821546512218665853782451862013e-4),
                                    T(-0.396836504717943466443123507595386882e-4),
                                    T(0.113757269706784190980552042885831759e-4),
                                    T(0.250749722623753280165221942390057007e-9),
                                    T(-0.169541495365583060147164356781525752e-5),
                                    T(0.890750753220530968882898422505515924e-6),
                                    T(-0.229293483400080487057216364891158518e-6),
                                    T(0.295679413754404904696572852500004588e-10),
                                    T(0.288658297427087836297341274604184504e-7),
                                    T(-0.141897394378032193894774303903982717e-7),
                                    T(0.344635804994648970659527720474194356e-8),
                                    T(-0.230245171745280671320192735850147087e-12),
                                    T(-0.394092330280464052750697640085291799e-9),
                                    T(0.186023389685045019134258533045185639e-9),
                                    T(-0.435632300505661804380678327446262424e-10),
                                    T(0.127860010162962312660550463349930726e-14),
                                    T(0.467927502665791946200382739991760062e-11),
                                    T(-0.214924647061348285410535341910721086e-11),
                                    T(0.490881561480965216323649688463984082e-12));
        
        auto c5 = bbm::poly<idx[5]>(z, 
                                    T(-0.000336798553366358150308767592718210002),
                                    T(-0.697281375836585777429398828575783308e-4),
                                    T(0.00027727532449593920787336425196507501),
                                    T(-0.000199325705161888477003360405280844238),
                                    T(0.679778047793720783881640176604435742e-4),
                                    T(0.141906292064396701483392727105575757e-6),
                                    T(-0.135940481897686932784583938837504469e-4),
                                    T(0.80184702563342015397192571980419684e-5),
                                    T(-0.229148117650809517038048790128781806e-5),
                                    T(-0.325247355129845395166230137750005047e-9),
                                    T(0.346528464910852649559195496827579815e-6),
                                    T(-0.184471871911713432765322367374920978e-6),
                                    T(0.482409670378941807563762631738989002e-7),
                                    T(-0.179894667217435153025754291716644314e-13),
                                    T(-0.630619450001352343517516981425944698e-8),
                                    T(0.316241762877456793773762181540969623e-8),
                                    T(-0.784092425369742929000839303523267545e-9));
        
        auto c6 = bbm::poly<idx[6]>(z, 
                                    T(0.00053130793646399222316574854297762391),
                                    T(-0.000592166437353693882864836225604401187),
                                    T(0.000270878209671804482771279183488328692),
                                    T(0.790235323266032787212032944390816666e-6),
                                    T(-0.815396936756196875092890088464682624e-4),
                                    T(0.561168275310624965003775619041471695e-4),
                                    T(-0.183291165828433755673259749374098313e-4),
                                    T(-0.307961345060330478256414192546677006e-8),
                                    T(0.346515536880360908673728529745376913e-5),
                                    T(-0.202913273960586037269527254582695285e-5),
                                    T(0.578879286314900370889997586203187687e-6),
                                    T(0.233863067382665698933480579231637609e-12),
                                    T(-0.88286007463304835250508524317926246e-7),
                                    T(0.474359588804081278032150770595852426e-7),
                                    T(-0.125454150207103824457130611214783073e-7));
        
        auto c7 = bbm::poly<idx[7]>(z, 
                                    T(0.000344367606892377671254279625108523655),
                                    T(0.517179090826059219337057843002058823e-4),
                                    T(-0.000334931610811422363116635090580012327),
                                    T(0.000281269515476323702273722110707777978),
                                    T(-0.000109765822446847310235396824500789005),
                                    T(-0.127410090954844853794579954588107623e-6),
                                    T(0.277444515115636441570715073933712622e-4),
                                    T(-0.182634888057113326614324442681892723e-4),
                                    T(0.578769494973505239894178121070843383e-5),
                                    T(0.493875893393627039981813418398565502e-9),
                                    T(-0.105953670140260427338098566209633945e-5),
                                    T(0.616671437611040747858836254004890765e-6),
                                    T(-0.175629733590604619378669693914265388e-6));
        
        auto c8 = bbm::poly<idx[8]>(z, 
                                    T(-0.000652623918595309418922034919726622692),
                                    T(0.000839498720672087279993357516764983445),
                                    T(-0.000438297098541721005061087953050560377),
                                    T(-0.696909145842055197136911097362072702e-6),
                                    T(0.00016644846642067547837384572662326101),
                                    T(-0.000127835176797692185853344001461664247),
                                    T(0.462995326369130429061361032704489636e-4),
                                    T(0.455790986792270771162749294232219616e-8),
                                    T(-0.105952711258051954718238500312872328e-4),
                                    T(0.678334290486516662273073740749269432e-5),
                                    T(-0.210754766662588042469972680229376445e-5));
        
        auto c9 = bbm::poly<idx[9]>(z, 
                                    T(-0.000596761290192746250124390067179459605),
                                    T(-0.720489541602001055908571930225015052e-4),
                                    T(0.000678230883766732836161951166000673426),
                                    T(-0.000640147526026275845100045652582354779),
                                    T(0.000277501076343287044992374518205845463),
                                    T(0.181970083804651510461686554030325202e-6),
                                    T(-0.847950711706850318239732559632810086e-4),
                                    T(0.610519208250153101764709122740859458e-4),
                                    T(-0.210739201834048624082975255893773306e-4));
        
        auto c10 = bbm::poly<idx[10]>(z, 
                                      T(0.00133244544948006563712694993432717968),
                                      T(-0.00191443849856547752650089885832852254),
                                      T(0.0011089369134596637339607446329267522),
                                      T(0.993240412264229896742295262075817566e-6),
                                      T(-0.000508745012930931989848393025305956774),
                                      T(0.00042735056665392884328432271160040444),
                                      T(-0.000168588537679107988033552814662382059));
        
        auto c11 = bbm::poly<idx[11]>(z, 
                                      T(0.00157972766073083495908785631307733022),
                                      T(0.000162516262783915816898635123980270998),
                                      T(-0.00206334210355432762645284467690276817),
                                      T(0.00213896861856890981541061922797693947),
                                      T(-0.00101085593912630031708085801712479376));
        
        auto c12 = bbm::poly<idx[12]>(z, 
                                      T(-0.00407251211951401664727281097914544601),
                                      T(0.00640336283380806979482363809026579583),
                                      T(-0.00404101610816766177473974858518094879));
        
        auto c13 = bbm::poly<idx[13]>(z, T(-0.0059475779383993002845382844736066323L));

        return bbm::poly<idx[14]>(bbm::rcp(a), c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13);
      };

      // Eval poly (based on number of digits in result_t)
      result_t result;
      if constexpr (std::numeric_limits<scalar_t<result_t>>::digits <= 24) result = eval_poly(result_t(a), z, std::index_sequence<7,5,3,0,0,0,0,0,0,0,0,0,0,0,0,3>{});
      else if constexpr (std::numeric_limits<scalar_t<result_t>>::digits <= 53) result = eval_poly(result_t(a), z, std::index_sequence<15,13,11,9,7,9,7,5,3,1,0,0,0,0,10>{});
      else if constexpr (std::numeric_limits<scalar_t<result_t>>::digits <= 64) result = eval_poly(result_t(a), z, std::index_sequence<19,17,15,13,11,9,11,9,7,5,3,5,3,0,13>{});
      else result = eval_poly(result_t(a), z, std::index_sequence<31,29,27,25,23,17,15,13,11,9,7,5,3,1,14>{}); 

      // Complete eval.
      result *= bbm::sign(x-a) * bbm::exp(-y) / bbm::sqrt(std::numbers::pi * 2 * a);
      result += bbm::erfc( bbm::sqrt(y)) * 0.5;

      // Denormalize if required
      if constexpr (!normalize) result *= bbm::tgamma(result_t(a));
      
      // Done.
      return bbm::select(x < a, result, 1 - result);
    }
    
  } // end detail namespace


  /**********************************************************************/
  /*! \brief Unnormalized incomplete lower gamma function
    
    \f$ \gamma(a,x) = \int^x_0 e^{-t} t^{a-1} dt \f$

    *********************************************************************/
  template<typename TA, typename TX, size_t MaxTerm=100>
    inline auto tgamma_lower(const TA& a, const TX& x)
  {
    using result_t = decltype(a+x);
    result_t result=0;
    
    // Large approx?
    auto mask = bbm::detail::gamma_is_large(a,x);
    auto todo = !mask;
    if(bbm::any(mask)) result = bbm::select(mask, bbm::detail::gamma_large<false>(a, x), result);
    if(bbm::none(todo)) return result;
    
    // Small prox (x <= a+1) && (a<20)
    mask = todo && (x <= a+1);
    todo &= !mask;
    if(bbm::any(mask)) result = bbm::select(mask, bbm::detail::gamma<MaxTerm, false>(a, x), result);
    if(bbm::none(todo)) return result;
    
    // x>a+1 && a < 20
    mask = todo && (x > a+1);
    if(bbm::any(mask)) result = bbm::select(mask, (bbm::tgamma(a) - bbm::detail::Gamma<MaxTerm, false>(a, x)), result);
    
    // Done.
    return result;
  }
  
  /**********************************************************************/
  /*! \brief Normalized incomplete lower gamma function
    
    \f$ P(a,x) = \frac{\gamma(a,x)}{\Gamma(a)} = \frac{1}{\Gamma(a)} \int^x_0 e^{-t} t^{a-1} dt \f$

    *********************************************************************/
  template<typename TA, typename TX, size_t MaxTerm=100>
    inline auto gamma_p(const TA& a, const TX& x)
  {
    using result_t = decltype(a+x);
    result_t result=0;
    
    // Large approx?
    auto mask = bbm::detail::gamma_is_large(a,x);
    auto todo = !mask;
    if(bbm::any(mask)) result = bbm::select(mask, bbm::detail::gamma_large<true>(a, x), result);
    if(bbm::none(todo)) return result;
    
    // Small prox (x <= a+1) && (a<20)
    mask = todo && (x <= a+1);
    todo &= !mask;
    if(bbm::any(mask)) result = bbm::select(mask, bbm::detail::gamma<MaxTerm, true>(a, x), result);
    if(bbm::none(todo)) return result;
    
    // x>a+1 && a < 20
    mask = todo && (x > a+1);
    if(bbm::any(mask)) result = bbm::select(mask, (1 - bbm::detail::Gamma<MaxTerm, true>(a, x)), result);
    
    // Done.
    return result;
  }


  /**********************************************************************/
  /*! \brief Unnormalized incomplete upper gamma function
    
    \f$ \gamma(a,x) = \int^\infty_x e^{-t} t^{a-1} dt \f$

    *********************************************************************/
  template<typename TA, typename TX, size_t MaxTerm=100>
    inline auto tgamma(const TA& a, const TX& x)
  {
    using result_t = decltype(a+x);
    result_t result=0;
    
    // Large approx?
    auto mask = bbm::detail::gamma_is_large(a,x);
    auto todo = !mask;
    if(bbm::any(mask)) result = bbm::select(mask, (1-bbm::detail::gamma_large<false>(a, x)), result);
    if(bbm::none(todo)) return result;
    
    // Small prox (x <= a+1) && (a<20)
    mask = todo && (x <= a+1);
    todo &= !mask;
    if(bbm::any(mask)) result = bbm::select(mask, (bbm::tgamma(a) - bbm::detail::gamma<MaxTerm, false>(a, x)), result);
    if(bbm::none(todo)) return result;
    
    // x>a+1 && a < 20
    mask = todo && (x > a+1);
    if(bbm::any(mask)) result = bbm::select(mask, bbm::detail::Gamma<MaxTerm, false>(a, x), result);
    
    // Done.
    return result;
  }

 /**********************************************************************/
  /*! \brief Normalized incomplete upper gamma function

    \f$ Q(a,x) = 1 - P(a,x) = \frac{\Gamma(a,x)}{\Gamma(a)} = \frac{1}{\Gamma(a)} \int^\infty_x e^{-t} t^{a-1} dt \f$
        
    *********************************************************************/
  template<typename TA, typename TX, size_t MaxTerm=100>
    inline auto gamma_q(const TA& a, const TX& x)
  {
    using result_t = decltype(a+x);
    result_t result=0;
    
    // Large approx?
    auto mask = bbm::detail::gamma_is_large(a,x);
    auto todo = !mask;
    if(bbm::any(mask)) result = bbm::select(mask, (1-bbm::detail::gamma_large<true>(a, x)), result);
    if(bbm::none(todo)) return result;
    
    // Small prox (x <= a+1) && (a<20)
    mask = todo && (x <= a+1);
    todo &= !mask;
    if(bbm::any(mask)) result = bbm::select(mask, (1 - bbm::detail::gamma<MaxTerm, true>(a, x)), result);
    if(bbm::none(todo)) return result;
    
    // x>a+1 && a < 20
    mask = todo && (x > a+1);
    if(bbm::any(mask)) result = bbm::select(mask, bbm::detail::Gamma<MaxTerm, true>(a, x), result);
    
    // Done.
    return result;
  }

  /**********************************************************************/
  /*! \brief Normalized incomplete upper and lower gamma function
    *********************************************************************/
  template<typename TA, typename TX, size_t MaxTerm=100>
    inline auto gamma_pq(const TA& a, const TX& x)
  {
    using result_t = decltype(a+x);
    auto result = make_named<"p","q">(result_t(0), result_t(0));
    
    // Large approx?
    auto mask = bbm::detail::gamma_is_large(a,x);
    auto todo = !mask;
    if(bbm::any(mask))
    {
      auto p = bbm::detail::gamma_large<true>(a, x);
      result = bbm::select(mask, {p,1-p}, result);
    }
    if(bbm::none(todo)) return result;
    
    // Small prox (x <= a+1) && (a<20)
    mask = todo && (x <= a+1);
    todo &= !mask;
    if(bbm::any(mask))
    {
      auto p = bbm::detail::gamma<MaxTerm, true>(a, x);
      result = bbm::select(mask, {p, 1-p}, result);
    }
    if(bbm::none(todo)) return result;
    
    // x>a+1 && a < 20
    mask = todo && (x > a+1);
    if(bbm::any(mask))
    {
      auto q = bbm::detail::Gamma<MaxTerm, true>(a, x);
      result = bbm::select(mask, {1-q,q}, result);
    }
    
    // Done.
    return result;
  }

} // end bbm namespace

#endif /* _BBM_GAMMA_H_ */
