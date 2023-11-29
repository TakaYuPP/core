#define NO_UEFI

#include "gtest/gtest.h"

// current optimized implementation
#include "../src/score.h"

// reference implementation
#include "score_reference.h"


#include "uc32.h"


template<
    unsigned int dataLength,
    unsigned int infoLength,
    unsigned int numberOfInputNeutrons,
    unsigned int numberOfOutputNeutrons,
    unsigned int maxInputDuration,
    unsigned int maxOutputDuration,
    unsigned int maxNumberOfProcessors,
    unsigned int solutionBufferCount = 8
>
struct ScoreTester
{
    typedef ScoreFunction<
        dataLength, infoLength,
        numberOfInputNeutrons, numberOfOutputNeutrons,
        maxInputDuration, maxOutputDuration,
        maxNumberOfProcessors, solutionBufferCount
    > ScoreFuncOpt;
    typedef ScoreReferenceImplementation<
        dataLength, infoLength,
        numberOfInputNeutrons, numberOfOutputNeutrons,
        maxInputDuration, maxOutputDuration,
        maxNumberOfProcessors
    > ScoreFuncRef;

    ScoreFuncOpt* score;
    ScoreFuncRef* score_ref_impl;

    ScoreTester()
    {
        score = new ScoreFuncOpt;
        score_ref_impl = new ScoreFuncRef;
        score->initMiningData();
        score_ref_impl->initMiningData();
#if USE_SCORE_CACHE
        score->initEmptyScoreCache();
#endif
    }

    ~ScoreTester()
    {
        delete score;
        delete score_ref_impl;
    }

    bool operator()(const unsigned long long processorNumber, unsigned char* publicKey, unsigned char* nonce)
    {
        unsigned int current = (*score)(processorNumber, publicKey, nonce);
        unsigned int reference = (*score_ref_impl)(processorNumber, publicKey, nonce);
        std::cout << "current score() returns " << current << ", reference score() returns " << reference << std::endl;
        return current == reference;
    }
};


template <typename ScoreTester>
void runCommonTests(ScoreTester& test_score)
{
    EXPECT_TRUE(test_score(250, UC32x(8320711378477050309ULL, 248251795722472794ULL, 7094584288671124888ULL, 14227443369010736271ULL).uc32x, UC32x(5716962451283696375ULL, 15438913665440563544ULL, 5660271417447366021ULL, 1449305955311789203ULL).uc32x));
    EXPECT_TRUE(test_score(76, UC32x(6015686698731382584ULL, 17693382922901793301ULL, 5303467829488852063ULL, 5536581860782508177ULL).uc32x, UC32x(14651129848458773837ULL, 16606782960251081014ULL, 4824316377441174482ULL, 3112397658988141833ULL).uc32x));
    /*
    EXPECT_TRUE(test_score(836, UC32x(8819055211791392223ULL, 3612344957800964343ULL, 1616111278302647729ULL, 12291118741461627018ULL).uc32x, UC32x(5867722639434795907ULL, 17375317486170178940ULL, 18387574397302113102ULL, 9961112317218108906ULL).uc32x));
    EXPECT_TRUE(test_score(949, UC32x(3740010853302435321ULL, 11266677711290499638ULL, 6513295658310774369ULL, 15399782464434570122ULL).uc32x, UC32x(6941240860102303002ULL, 227594860492777111ULL, 18372348483042259765ULL, 12780778386658597109ULL).uc32x));
    EXPECT_TRUE(test_score(530, UC32x(7810734274729939137ULL, 15112447306033184324ULL, 10032622121553863387ULL, 13373831249435042196ULL).uc32x, UC32x(700835417149071305ULL, 14041796352912169829ULL, 17241713312634323814ULL, 6501703188642931345ULL).uc32x));
    EXPECT_TRUE(test_score(171, UC32x(6881693068054561446ULL, 8198193001912881382ULL, 8163393271466223361ULL, 8034056475698829591ULL).uc32x, UC32x(650169868309319616ULL, 1823305317569880788ULL, 7570330312056244878ULL, 8447310366263252888ULL).uc32x));
    EXPECT_TRUE(test_score(460, UC32x(9935290575929513068ULL, 3361334582520659635ULL, 18155488379260412692ULL, 4478717486148509604ULL).uc32x, UC32x(3750433195679230041ULL, 2516972679359509920ULL, 16735967429906322932ULL, 14551154992642235887ULL).uc32x));
    EXPECT_TRUE(test_score(882, UC32x(4313385617050085526ULL, 4738891533386970273ULL, 3513109154786599585ULL, 7945128467931244511ULL).uc32x, UC32x(3453524507424301765ULL, 10932906502554513623ULL, 41719851520346144ULL, 14030742550589866331ULL).uc32x));
    EXPECT_TRUE(test_score(350, UC32x(5063643796228753393ULL, 331912174799411468ULL, 14574381799323593931ULL, 17083611716540179373ULL).uc32x, UC32x(15316155741777348904ULL, 7825717298620594378ULL, 7951094136069142064ULL, 4770425742020355364ULL).uc32x));
    EXPECT_TRUE(test_score(755, UC32x(16389646495862008616ULL, 13893830711919112707ULL, 1131260305373423775ULL, 9121041763512903564ULL).uc32x, UC32x(15299538529594252043ULL, 13602960729522503533ULL, 3673523746285887157ULL, 5364900307275415015ULL).uc32x));
    EXPECT_TRUE(test_score(314, UC32x(15913335766282960142ULL, 12384689933754864034ULL, 18095014241744928810ULL, 7526449114250040880ULL).uc32x, UC32x(5933023302009888929ULL, 17142321738063909494ULL, 6002289459746442539ULL, 5080097501472233013ULL).uc32x));
    EXPECT_TRUE(test_score(975, UC32x(10781904601693064024ULL, 5213147055142167032ULL, 14622816232426232556ULL, 1695451401714056683ULL).uc32x, UC32x(7320907976003247452ULL, 8824006621344223264ULL, 4763823577360622280ULL, 17710302295718056691ULL).uc32x));
    EXPECT_TRUE(test_score(829, UC32x(3822418878361472798ULL, 13574879389023813648ULL, 3553521771729970075ULL, 2954536520207062075ULL).uc32x, UC32x(10121827377094623986ULL, 2370110259271873238ULL, 15873426783715339763ULL, 17627039380405293802ULL).uc32x));
    EXPECT_TRUE(test_score(208, UC32x(5998717381609534770ULL, 11582611746788655377ULL, 17852550046063201878ULL, 8141564077695068879ULL).uc32x, UC32x(13215225797323315039ULL, 2330135378216956038ULL, 16951204830553393121ULL, 7494452792305868867ULL).uc32x));
    EXPECT_TRUE(test_score(104, UC32x(1490369402565320157ULL, 16179995763892143016ULL, 3242025777714338691ULL, 6855179062767378829ULL).uc32x, UC32x(17760210398447341531ULL, 4087936017898107026ULL, 2531763042366085708ULL, 1249216924950339922ULL).uc32x));
    EXPECT_TRUE(test_score(677, UC32x(14842179162542010533ULL, 1722882821968462551ULL, 8873697282441890544ULL, 1534652074700099280ULL).uc32x, UC32x(12938069764787092054ULL, 11040711420066622454ULL, 4406090710094690164ULL, 11005237789112295671ULL).uc32x));
    EXPECT_TRUE(test_score(838, UC32x(14879370025966105641ULL, 3831520276244726142ULL, 9852923425012327689ULL, 8541450690758149347ULL).uc32x, UC32x(3296306860323646117ULL, 11460607386698570900ULL, 18031055325860946868ULL, 1718089108317175497ULL).uc32x));
    EXPECT_TRUE(test_score(875, UC32x(4663642578309289095ULL, 14178731860827742329ULL, 18067351146719198739ULL, 12855312971371829163ULL).uc32x, UC32x(14104654580730513187ULL, 11934296273236936322ULL, 2275592252320284858ULL, 12217943001688772122ULL).uc32x));
    EXPECT_TRUE(test_score(674, UC32x(3600415757942109996ULL, 7891135569092760953ULL, 17469442016727328648ULL, 1906945334230040132ULL).uc32x, UC32x(3874110291435975482ULL, 1771808040140268179ULL, 8035658789611213633ULL, 6020143893974890169ULL).uc32x));
    EXPECT_TRUE(test_score(978, UC32x(5932980716309300042ULL, 7767492666787187881ULL, 5489751836121542328ULL, 1295013782452449839ULL).uc32x, UC32x(9808879403453018497ULL, 4008269019949293027ULL, 4110235051706821008ULL, 16199080033181601120ULL).uc32x));
    EXPECT_TRUE(test_score(27, UC32x(15827688664481280218ULL, 18404888195370335629ULL, 12629130740756897733ULL, 2381103608567068937ULL).uc32x, UC32x(5511008621205997146ULL, 12225182969783615253ULL, 15755920040396827300ULL, 2619852573919917303ULL).uc32x));
    EXPECT_TRUE(test_score(320, UC32x(4159333232469054113ULL, 570593575142079247ULL, 17472502345077165642ULL, 306384074843942021ULL).uc32x, UC32x(1145751198889409615ULL, 5100486357148182841ULL, 12897409571053744241ULL, 14669147490745406362ULL).uc32x));
    EXPECT_TRUE(test_score(44, UC32x(7547718341191572987ULL, 12536572665680640637ULL, 15179639360895563288ULL, 4073412258708607161ULL).uc32x, UC32x(10461675706840658064ULL, 6605869240808615474ULL, 1447208672744224892ULL, 10977503805818872372ULL).uc32x));
    EXPECT_TRUE(test_score(377, UC32x(14817660939575992657ULL, 6283555411091924519ULL, 2348851900113265349ULL, 3420585337712279937ULL).uc32x, UC32x(15793474073212649509ULL, 5731320875556199112ULL, 6597683710304969317ULL, 12590884175876093255ULL).uc32x));
    EXPECT_TRUE(test_score(644, UC32x(7737691158429672973ULL, 12397073719753320820ULL, 18441501052151932058ULL, 12432735581990512922ULL).uc32x, UC32x(9950020182604334816ULL, 2016162000822870855ULL, 2428401978451345003ULL, 7577649916460267914ULL).uc32x));
    EXPECT_TRUE(test_score(467, UC32x(956043332791994204ULL, 13284555381750111619ULL, 7898048307762414920ULL, 12575244084923366711ULL).uc32x, UC32x(11896233178349272887ULL, 14896625352073288983ULL, 3474411118881977056ULL, 4396423745467790181ULL).uc32x));
    EXPECT_TRUE(test_score(498, UC32x(4212535111137880198ULL, 16921611503325485491ULL, 4381910305554235058ULL, 7428497372078031187ULL).uc32x, UC32x(17259285338536844523ULL, 15873786749988580982ULL, 1894505817393630197ULL, 12821596659844842890ULL).uc32x));
    EXPECT_TRUE(test_score(593, UC32x(10735840204260344566ULL, 3956772891382250627ULL, 10583132493219320843ULL, 11085194559008762902ULL).uc32x, UC32x(11694366614157025337ULL, 6011352688735248565ULL, 3412258571822280656ULL, 11321083584818015741ULL).uc32x));
    EXPECT_TRUE(test_score(493, UC32x(10392478494505791544ULL, 3324407202343794836ULL, 1094179073088230779ULL, 17099464405855028739ULL).uc32x, UC32x(7938302250517529089ULL, 1532220003657154156ULL, 10723118439392081575ULL, 16449121918861310950ULL).uc32x));
    EXPECT_TRUE(test_score(361, UC32x(26095677719007985ULL, 10244152299641502782ULL, 17072009567021538075ULL, 16091502303016440237ULL).uc32x, UC32x(15813794340593965403ULL, 11489789827450116701ULL, 12534701338229299540ULL, 2104807437322884472ULL).uc32x));
    EXPECT_TRUE(test_score(46, UC32x(4354436232808510712ULL, 12570146846624677225ULL, 16246181885903399077ULL, 16547968421612383158ULL).uc32x, UC32x(250786383125860740ULL, 5541042747586308701ULL, 16268962736654303713ULL, 13241282279461919381ULL).uc32x));
    EXPECT_TRUE(test_score(398, UC32x(5023103874329294260ULL, 471638172854653410ULL, 14037890222290910650ULL, 12789424372677828596ULL).uc32x, UC32x(5098635702923420502ULL, 16951786842391864286ULL, 492311859622593039ULL, 14444710847856904978ULL).uc32x));
    EXPECT_TRUE(test_score(850, UC32x(9115944895839046515ULL, 9384023253682518157ULL, 15607466781469566937ULL, 15247937523457164830ULL).uc32x, UC32x(6006047271887805850ULL, 548443856048427202ULL, 4534064240235861884ULL, 4364366688588631885ULL).uc32x));
    EXPECT_TRUE(test_score(683, UC32x(276423892738786578ULL, 1722888286520621052ULL, 18022545900055107398ULL, 13188090218230933820ULL).uc32x, UC32x(11340395345311001099ULL, 16783597359868621026ULL, 16076312614649487834ULL, 5231564538968841463ULL).uc32x));
    EXPECT_TRUE(test_score(219, UC32x(7764529989863631289ULL, 12118812160635781670ULL, 149732193013299402ULL, 10067550106051478551ULL).uc32x, UC32x(971477966161792209ULL, 11959058061184446831ULL, 4094052783796128570ULL, 5535726317383716262ULL).uc32x));
    EXPECT_TRUE(test_score(37, UC32x(14719188896737439007ULL, 4541244637349059951ULL, 9197920453502154751ULL, 2947288576325972214ULL).uc32x, UC32x(8878119007260882270ULL, 11142208524669387557ULL, 9717548670009979761ULL, 4490779411282196792ULL).uc32x));
    EXPECT_TRUE(test_score(990, UC32x(4804079394802484541ULL, 6080980650220622788ULL, 3452651687024477774ULL, 11384503479206958146ULL).uc32x, UC32x(7286672408672248637ULL, 6068988545934057694ULL, 9421087539810003169ULL, 17563678417510449378ULL).uc32x));
    EXPECT_TRUE(test_score(128, UC32x(1248150536833538901ULL, 8860626881943623919ULL, 7855182086443200346ULL, 16201807590841254943ULL).uc32x, UC32x(8968468205075497809ULL, 17241855459633465534ULL, 15632491947292814844ULL, 16348708045275200540ULL).uc32x));
    EXPECT_TRUE(test_score(838, UC32x(13461140141988248356ULL, 8770058495264401534ULL, 6410737695373323630ULL, 4306191902688270566ULL).uc32x, UC32x(9689632182778853529ULL, 5808156710129207446ULL, 6258014013904477871ULL, 2957096442857094054ULL).uc32x));
    EXPECT_TRUE(test_score(300, UC32x(4113304660815210099ULL, 3033269907446706598ULL, 1844551531642138618ULL, 13100756204391741534ULL).uc32x, UC32x(15923380334268822861ULL, 1931851065069812214ULL, 14721893413597329277ULL, 11728975159342625192ULL).uc32x));
    EXPECT_TRUE(test_score(88, UC32x(879219737532582416ULL, 11879235255229192410ULL, 1707676354312616551ULL, 14784114619624720487ULL).uc32x, UC32x(287142185453099170ULL, 9334456307204934524ULL, 17355003283286805449ULL, 5070332635039800451ULL).uc32x));
    EXPECT_TRUE(test_score(376, UC32x(13724590976430816837ULL, 10851270777722246047ULL, 17697172949588442351ULL, 509566469415944740ULL).uc32x, UC32x(1761440979986425079ULL, 4261985364689393311ULL, 10825097146228397900ULL, 3324567434354875716ULL).uc32x));
    */
}


TEST(TestQubicScoreFunction, CurrentLengthNeronsDurationSettings) {
    ScoreTester<
        DATA_LENGTH, INFO_LENGTH,
        NUMBER_OF_INPUT_NEURONS, NUMBER_OF_OUTPUT_NEURONS,
        MAX_INPUT_DURATION, MAX_OUTPUT_DURATION,
        MAX_NUMBER_OF_PROCESSORS
    > test_score;
    runCommonTests(test_score);
}

TEST(TestQubicScoreFunction, LengthNeurons1000Duration200) {
    ScoreTester<
        1000, // DATA_LENGTH
        1000, // INFO_LENGTH
        1000, // NUMBER_OF_INPUT_NEURONS
        1000, // NUMBER_OF_OUTPUT_NEURONS
        200,  // MAX_INPUT_DURATION
        200,  // MAX_OUTPUT_DURATION
        MAX_NUMBER_OF_PROCESSORS
    > test_score;
    runCommonTests(test_score);
}

TEST(TestQubicScoreFunction, LengthNeurons1100Duration200) {
    ScoreTester<
        1100, // DATA_LENGTH
        1100, // INFO_LENGTH
        1100, // NUMBER_OF_INPUT_NEURONS
        1100, // NUMBER_OF_OUTPUT_NEURONS
        200,  // MAX_INPUT_DURATION
        200,  // MAX_OUTPUT_DURATION
        MAX_NUMBER_OF_PROCESSORS
    > test_score;
    runCommonTests(test_score);
}

TEST(TestQubicScoreFunction, DataLength1200InfoLength1000Neurons1000Duration200) {
    ScoreTester<
        1200, // DATA_LENGTH
        1000, // INFO_LENGTH
        1000, // NUMBER_OF_INPUT_NEURONS
        1000, // NUMBER_OF_OUTPUT_NEURONS
        200,  // MAX_INPUT_DURATION
        200,  // MAX_OUTPUT_DURATION
        MAX_NUMBER_OF_PROCESSORS
    > test_score;
    runCommonTests(test_score);
}

TEST(TestQubicScoreFunction, Length1200InputNeurons1000OutputNeurons1200Duration200) {
    ScoreTester<
        1200, // DATA_LENGTH
        1200, // INFO_LENGTH
        1000, // NUMBER_OF_INPUT_NEURONS
        1200, // NUMBER_OF_OUTPUT_NEURONS
        200,  // MAX_INPUT_DURATION
        200,  // MAX_OUTPUT_DURATION
        MAX_NUMBER_OF_PROCESSORS
    > test_score;
    runCommonTests(test_score);
}
